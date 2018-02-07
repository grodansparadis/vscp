/*
 * CAN driver for IXXAT USB-to-CAN V2
 *
 * Copyright (C) 2014 Michael Hengler <mhengler@ixxat.de>
 *
 * Based on code originally by pcan_usb_core
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */
#include <linux/netdevice.h>
#include <linux/usb.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/can.h>
#include <linux/can/dev.h>
#include <linux/can/error.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/types.h>
#include <linux/gfp.h>
#include <asm-generic/errno.h>
#include <stdarg.h>

#include "ixx_usb_core.h"

MODULE_SUPPORTED_DEVICE("IXXAT Automation GmbH USB-to-CAN V2");

/* use sja 1000 clock due to internal bittiming calculations */
#define SJA1000_CRYSTAL_HZ	      8000000

/* usb-to-can v2 Endpoints */
#define IXXAT_USBV2_EP_CMDOUT   0
#define IXXAT_USBV2_EP_CMDIN    (IXXAT_USBV2_EP_CMDOUT | USB_DIR_IN)
#define IXXAT_USBV2_EP_MSGOUT_0 1
#define IXXAT_USBV2_EP_MSGIN_0  (IXXAT_USBV2_EP_MSGOUT_0 | USB_DIR_IN)
#define IXXAT_USBV2_EP_MSGOUT_1 2
#define IXXAT_USBV2_EP_MSGIN_1  (IXXAT_USBV2_EP_MSGOUT_1 | USB_DIR_IN)
#define IXXAT_USBV2_EP_MSGOUT_2 3
#define IXXAT_USBV2_EP_MSGIN_2  (IXXAT_USBV2_EP_MSGOUT_2 | USB_DIR_IN)
#define IXXAT_USBV2_EP_MSGOUT_3 4
#define IXXAT_USBV2_EP_MSGIN_3  (IXXAT_USBV2_EP_MSGOUT_3 | USB_DIR_IN)
#define IXXAT_USBV2_EP_MSGOUT_4 5
#define IXXAT_USBV2_EP_MSGIN_4  (IXXAT_USBV2_EP_MSGOUT_4 | USB_DIR_IN)

/* usb-to-can v2 rx/tx buffers size */
#define IXXAT_USBV2_RX_BUFFER_SIZE      512
#define IXXAT_USBV2_TX_BUFFER_SIZE      256

#define IXXAT_USBV2_CMD_BUFFER_SIZE     256

#define IXXAT_USBV2_OPMODE_STANDARD  0x01 /* reception of 11-bit id messages */
#define IXXAT_USBV2_OPMODE_EXTENDED  0x02 /* reception of 29-bit id messages */
#define IXXAT_USBV2_OPMODE_ERRFRAME  0x04 /* enable reception of error frames */
#define IXXAT_USBV2_OPMODE_LISTONLY  0x08 /* listen only mode (TX passive) */

/* Stuff error */
#define IXXAT_USBV2_CAN_ERROR_STUFF      1
/* Form error */
#define IXXAT_USBV2_CAN_ERROR_FORM       2
/* Acknowledgment error */
#define IXXAT_USBV2_CAN_ERROR_ACK        3
/* Bit error */
#define IXXAT_USBV2_CAN_ERROR_BIT        4
/* CRC error */
#define IXXAT_USBV2_CAN_ERROR_CRC        6
/* Other (unspecified) error */
#define IXXAT_USBV2_CAN_ERROR_OTHER      7

/* Data overrun occurred */
#define IXXAT_USBV2_CAN_STATUS_OVRRUN    0x02
/* Error warning limit exceeded */
#define IXXAT_USBV2_CAN_STATUS_ERRLIM    0x04
/* Bus off status */
#define IXXAT_USBV2_CAN_STATUS_BUSOFF    0x08

#define IXXAT_USBV2_CAN_DATA             0x00
#define IXXAT_USBV2_CAN_INFO             0x01
#define IXXAT_USBV2_CAN_ERROR            0x02
#define IXXAT_USBV2_CAN_STATUS           0x03
#define IXXAT_USBV2_CAN_WAKEUP           0x04
#define IXXAT_USBV2_CAN_TIMEOVR          0x05
#define IXXAT_USBV2_CAN_TIMERST          0x06

#define IXXAT_USBV2_MSG_FLAGS_TYPE       0x000000FF
#define IXXAT_USBV2_MSG_FLAGS_SSM        0x00000100
#define IXXAT_USBV2_MSG_FLAGS_HPM        0x00000600
#define IXXAT_USBV2_MSG_FLAGS_RES        0x0000F800
#define IXXAT_USBV2_MSG_FLAGS_DLC        0x000F0000
#define IXXAT_USBV2_MSG_FLAGS_OVR        0x00100000
#define IXXAT_USBV2_MSG_FLAGS_SRR        0x00200000
#define IXXAT_USBV2_MSG_FLAGS_RTR        0x00400000
#define IXXAT_USBV2_MSG_FLAGS_EXT        0x00800000
#define IXXAT_USBV2_MSG_FLAGS_AFC        0xFF000000

#define IXXAT_USBV2_BAL_CMD_CLASS        3
#define IXXAT_USBV2_BRD_CMD_CLASS        4
#define IXXAT_USBV2_BMG_CMD_CLASS        5

#define IXXAT_USBV2_BRD_CMD_CAT          0
#define IXXAT_USBV2_CAN_CMD_CAT          1

#define IXXAT_USBV2_VCI_CMD_CODE(Class, Function) \
	((u32) (((Class) << 8) | (Function)))

#define IXXAT_USBV2_BRD_CMD_CODE(Category, Function) \
	IXXAT_USBV2_VCI_CMD_CODE(IXXAT_USBV2_BRD_CMD_CLASS, \
		((Category) << 5) | (Function))

#define IXXAT_USBV2_BAL_CMD_CODE(Category, Function) \
	IXXAT_USBV2_VCI_CMD_CODE(IXXAT_USBV2_BAL_CMD_CLASS, \
		((Category) << 5) | (Function))

#define IXXAT_USBV2_CAN_GET_CAPS_CMD \
	IXXAT_USBV2_BAL_CMD_CODE(IXXAT_USBV2_CAN_CMD_CAT, 0)
#define IXXAT_USBV2_CAN_INIT_CMD \
	IXXAT_USBV2_BAL_CMD_CODE(IXXAT_USBV2_CAN_CMD_CAT, 5)
#define IXXAT_USBV2_CAN_START_CMD \
	IXXAT_USBV2_BAL_CMD_CODE(IXXAT_USBV2_CAN_CMD_CAT, 6)
#define IXXAT_USBV2_CAN_STOP_CMD \
	IXXAT_USBV2_BAL_CMD_CODE(IXXAT_USBV2_CAN_CMD_CAT, 7)
#define IXXAT_USBV2_CAN_RESET_CMD \
	IXXAT_USBV2_BAL_CMD_CODE(IXXAT_USBV2_CAN_CMD_CAT, 8)

#define IXXAT_USBV2_BRD_GET_FWINFO_CMD \
	IXXAT_USBV2_BRD_CMD_CODE(IXXAT_USBV2_BRD_CMD_CAT, 0)
#define IXXAT_USBV2_BRD_GET_DEVCAPS_CMD \
	IXXAT_USBV2_BRD_CMD_CODE(IXXAT_USBV2_BRD_CMD_CAT, 1)
#define IXXAT_USBV2_BRD_GET_DEVINFO_CMD \
	IXXAT_USBV2_BRD_CMD_CODE(IXXAT_USBV2_BRD_CMD_CAT, 2)

struct ixx_usbv2_dal_req {
	u32 req_size;
	u16 req_port;
	u16 req_socket;
	u32 req_code;
} __packed;

struct ixx_usbv2_dal_res {
	u32 res_size;
	u32 ret_size;
	u32 ret_code;
} __packed;

struct ixx_usbv2_dev_caps_req {
	struct ixx_usbv2_dal_req dal_req;
} __packed;

struct ixx_usbv2_dev_caps_res {
	struct ixx_usbv2_dal_res dal_res;
	struct ixx_dev_caps dev_caps;
} __packed;

struct ixx_usbv2_ctrl_caps_req {
	struct ixx_usbv2_dal_req dal_req;
} __packed;

struct ixx_usbv2_ctrl_caps_res {
	struct ixx_usbv2_dal_res dal_res;
	struct ixx_ctrl_caps ctrl_caps;
} __packed;

struct ixx_usbv2_ctrl_init_req {
	struct ixx_usbv2_dal_req dal_req;
	u8 mode;
	u8 btr0;
	u8 btr1;
	u8 padding;
} __packed;

struct ixx_usbv2_ctrl_init_res {
	struct ixx_usbv2_dal_res dal_res;
} __packed;

struct ixx_usbv2_ctrl_start_req {
	struct ixx_usbv2_dal_req dal_req;
} __packed;

struct ixx_usbv2_ctrl_start_res {
	struct ixx_usbv2_dal_res dal_res;
	u32 start_time;
} __packed;

struct ixx_usbv2_ctrl_stop_req {
	struct ixx_usbv2_dal_req dal_req;
	u32 action;
} __packed;

struct ixx_usbv2_ctrl_stop_res {
	struct ixx_usbv2_dal_res dal_res;
} __packed;

struct ixx_usbv2_brd_get_fwinfo_req {
	struct ixx_usbv2_dal_req dal_req;
} __packed;

struct ixx_usbv2_brd_get_fwinfo_res {
	struct ixx_usbv2_dal_res dal_res;
	struct ixx_intf_fw_info fwinfo;
} __packed;

struct ixx_usbv2_brd_get_intf_info_req {
	struct ixx_usbv2_dal_req dal_req;
} __packed;

struct ixx_usbv2_brd_get_intf_info_res {
	struct ixx_usbv2_dal_res dal_res;
	struct ixx_intf_info info;
} __packed;

/*
 * send usb-to-can v2 command synchronously
 */
static int ixx_usbv2_send_cmd(struct usb_device *dev,
		struct ixx_usbv2_dal_req *dal_req)
{
	int err, i;
	u16 size, value;
	u8  request, requesttype;
	u8 *buf;

	request     = 0xff;
	requesttype = USB_TYPE_VENDOR | USB_DIR_OUT;
	value       = le16_to_cpu(dal_req->req_port);
	size        = le32_to_cpu(dal_req->req_size) +
		sizeof(const struct ixx_usbv2_dal_res);

	buf = kmalloc(size, GFP_KERNEL);
	if(!buf)
		return -ENOMEM;
	memcpy(buf, (u8 *)dal_req, size);


	for (i = 0; i < 10; ++i) {
		err = usb_control_msg(dev, usb_sndctrlpipe(dev, 0), request,
			requesttype,
			value,
			0,
			buf,
			size,
			msecs_to_jiffies(50));

		if (err < 0)
			msleep(20);
		else
			break;
	}

	kfree(buf);

	if (err < 0) {
		dev_err(&dev->dev, "sending command failure: %d\n", err);
		return err;
	}

	return 0;
}

/*
 * receive usb-to-can v2 command synchronously
 */
static int ixx_usbv2_rcv_cmd(struct usb_device *dev,
		struct ixx_usbv2_dal_res *dal_res, int value)
{
	int err, res_size, i, size_to_read;
	u8  request, requesttype;
	u8 *buf;

	request      = 0xff;
	requesttype  = USB_TYPE_VENDOR | USB_DIR_IN;
	res_size     = 0;
	size_to_read = le32_to_cpu(dal_res->res_size);

	buf = kmalloc(size_to_read, GFP_KERNEL);
	if(!buf)
		return -ENOMEM;


	for (i = 0; i < 10; ++i) {
		err = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0), request,
				requesttype, value,
				0, buf + (u8) res_size,
				size_to_read - res_size, msecs_to_jiffies(50));

		if (err < 0) {
			msleep(20);
			continue;
		}

		res_size += err;
		if (res_size < size_to_read)
			msleep(20);
		else
			break;
	}

	if (res_size != size_to_read)
		err = -EBADMSG;

	if (err < 0) {
		dev_err(&dev->dev, "receiving command failure: %d\n", err);
		kfree(buf);
		return err;
	}

	memcpy((u8 *)dal_res, buf, size_to_read);
	kfree(buf);

	return err;
}

static int ixx_usbv2_init_ctrl(struct ixx_usb_device *dev, u8 mode, u8 btr0,
		u8 btr1)
{
	int err = -ENODEV;
	u8 data[IXXAT_USBV2_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbv2_ctrl_init_req *ctrl_init_req;
	struct ixx_usbv2_ctrl_init_res *ctrl_init_res;
	u32 req_size = sizeof(*ctrl_init_req);

	ctrl_init_req = (struct ixx_usbv2_ctrl_init_req *) data;
	ctrl_init_res = (struct ixx_usbv2_ctrl_init_res *)(data + req_size);

	ctrl_init_req->dal_req.req_size   = cpu_to_le32(req_size);
	ctrl_init_req->dal_req.req_code   =
		cpu_to_le32(IXXAT_USBV2_CAN_INIT_CMD);
	ctrl_init_req->dal_req.req_port   = cpu_to_le16(dev->ctrl_idx);
	ctrl_init_req->dal_req.req_socket = 0xffff;
	ctrl_init_req->mode               = mode;
	ctrl_init_req->btr0               = btr0;
	ctrl_init_req->btr1               = btr1;

	ctrl_init_res->dal_res.res_size = cpu_to_le32(
		sizeof(*ctrl_init_res));
	ctrl_init_res->dal_res.ret_size = 0;
	ctrl_init_res->dal_res.ret_code = 0xffffffff;

	err = ixx_usbv2_send_cmd(dev->udev, &ctrl_init_req->dal_req);
	if (err < 0)
		return err;

	err = ixx_usbv2_rcv_cmd(dev->udev,
		&ctrl_init_res->dal_res,
		dev->ctrl_idx);
	if (err < 0)
		return err;

	return le32_to_cpu(ctrl_init_res->dal_res.ret_code);
}

static int ixx_usbv2_start_ctrl(struct ixx_usb_device *dev, u32 *time_ref)
{
	int err = -ENODEV;
	u8 data[IXXAT_USBV2_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbv2_ctrl_start_req *ctrl_start_req;
	struct ixx_usbv2_ctrl_start_res *ctrl_start_res;
	u32 req_size = sizeof(*ctrl_start_req);

	ctrl_start_req = (struct ixx_usbv2_ctrl_start_req *) data;
	ctrl_start_res = (struct ixx_usbv2_ctrl_start_res *)(data + req_size);

	ctrl_start_req->dal_req.req_size   = cpu_to_le32(req_size);
	ctrl_start_req->dal_req.req_code   =
		cpu_to_le32(IXXAT_USBV2_CAN_START_CMD);
	ctrl_start_req->dal_req.req_port   = cpu_to_le16(dev->ctrl_idx);
	ctrl_start_req->dal_req.req_socket = 0xffff;

	ctrl_start_res->dal_res.res_size = cpu_to_le32(
		sizeof(*ctrl_start_res));
	ctrl_start_res->dal_res.ret_size = 0;
	ctrl_start_res->dal_res.ret_code = 0xffffffff;
	ctrl_start_res->start_time       = 0;

	err = ixx_usbv2_send_cmd(dev->udev, &ctrl_start_req->dal_req);
	if (err < 0)
		return err;

	err = ixx_usbv2_rcv_cmd(dev->udev,
		&ctrl_start_res->dal_res,
		dev->ctrl_idx);
	if (err < 0)
		return err;

	if (time_ref)
		*time_ref = le32_to_cpu(ctrl_start_res->start_time);

	return le32_to_cpu(ctrl_start_res->dal_res.ret_code);
}

static int ixx_usbv2_stop_ctrl(struct ixx_usb_device *dev)
{
	int err = -ENODEV;
	u8 data[IXXAT_USBV2_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbv2_ctrl_stop_req *ctrl_stop_req;
	struct ixx_usbv2_ctrl_stop_res *ctrl_stop_res;
	u32 req_size = sizeof(struct ixx_usbv2_ctrl_stop_req);

	ctrl_stop_req = (struct ixx_usbv2_ctrl_stop_req *) data;
	ctrl_stop_res = (struct ixx_usbv2_ctrl_stop_res *)(data + req_size);

	ctrl_stop_req->dal_req.req_size   = cpu_to_le32(req_size);
	ctrl_stop_req->dal_req.req_code   =
		cpu_to_le32(IXXAT_USBV2_CAN_STOP_CMD);
	ctrl_stop_req->dal_req.req_port   = cpu_to_le16(dev->ctrl_idx);
	ctrl_stop_req->dal_req.req_socket = 0xffff;
	ctrl_stop_req->action             = cpu_to_le32(0x3);

	ctrl_stop_res->dal_res.res_size =
		cpu_to_le32(sizeof(*ctrl_stop_res));
	ctrl_stop_res->dal_res.ret_size = 0;
	ctrl_stop_res->dal_res.ret_code = 0xffffffff;

	err = ixx_usbv2_send_cmd(dev->udev, &ctrl_stop_req->dal_req);
	if (err < 0)
		return err;

	err = ixx_usbv2_rcv_cmd(dev->udev,
		&ctrl_stop_res->dal_res,
		dev->ctrl_idx);
	if (err < 0)
		return err;

	if (!le32_to_cpu(ctrl_stop_res->dal_res.ret_code))
		dev->can.state = CAN_STATE_STOPPED;

	return le32_to_cpu(ctrl_stop_res->dal_res.ret_code);
}

static int ixx_usbv2_reset_ctrl(struct ixx_usb_device *dev)
{
	int err = -ENODEV;
	u8 data[IXXAT_USBV2_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbv2_dal_req *dal_req;
	struct ixx_usbv2_dal_res *dal_res;
	u32 req_size = sizeof(*dal_req);

	dal_req = (struct ixx_usbv2_dal_req *) data;
	dal_res = (struct ixx_usbv2_dal_res *)(data + req_size);

	dal_req->req_size   = cpu_to_le32(req_size);
	dal_req->req_code   = cpu_to_le32(IXXAT_USBV2_CAN_RESET_CMD);
	dal_req->req_port   = cpu_to_le16(dev->ctrl_idx);
	dal_req->req_socket = 0xffff;

	dal_res->res_size = cpu_to_le32(sizeof(*dal_res));
	dal_res->ret_size = 0;
	dal_res->ret_code = 0xffffffff;

	err = ixx_usbv2_send_cmd(dev->udev, dal_req);
	if (err < 0)
		return err;

	err = ixx_usbv2_rcv_cmd(dev->udev, dal_res,
			dev->ctrl_idx);
	if (err < 0)
		return err;

	return le32_to_cpu(dal_res->ret_code);
}

static int ixx_usbv2_set_bittiming(struct ixx_usb_device *dev,
		struct can_bittiming *bt)
{
	u8 btr0 = 0, btr1 = 0, can_opmode = 0;

	btr0 = ((bt->brp - 1) & 0x3f) | (((bt->sjw - 1) & 0x3) << 6);
	btr1 = ((bt->prop_seg + bt->phase_seg1 - 1) & 0xf)
			| (((bt->phase_seg2 - 1) & 0x7) << 4);
	if (dev->can.ctrlmode & CAN_CTRLMODE_3_SAMPLES)
		btr1 |= 0x80;

	can_opmode = IXXAT_USBV2_OPMODE_EXTENDED | IXXAT_USBV2_OPMODE_STANDARD;

	if (dev->can.ctrlmode & CAN_CTRLMODE_BERR_REPORTING)
		can_opmode |= IXXAT_USBV2_OPMODE_ERRFRAME;
	if (dev->can.ctrlmode & CAN_CTRLMODE_LISTENONLY)
		can_opmode |= IXXAT_USBV2_OPMODE_LISTONLY;

	dev->btr0 = btr0;
	dev->btr1 = btr1;

	netdev_dbg(dev->netdev, "setting btr0=0x%08x btr1=0x%08x mode=0x%08x\n",
			btr0, btr1, can_opmode);

	return ixx_usbv2_init_ctrl(dev, can_opmode, btr0, btr1);
}

/*
 * handle restart but in asynchronously way
 */
static int ixx_usbv2_restart_task(void *user_data)
{
	u32 time_ref;
	struct ixx_usb_device *dev = user_data;

	while (!kthread_should_stop()) {
		if (!dev->must_quit) {
			wait_event_interruptible(dev->wait_queue,
					dev->restart_flag);
			if (!dev->must_quit) {
				ixx_usbv2_stop_ctrl(dev);
				ixx_usbv2_start_ctrl(dev, &time_ref);
				dev->restart_flag = 0;
				dev->can.state = CAN_STATE_ERROR_ACTIVE;
			}
		} else
			msleep(20);
	}
	return 0;
}

static int ixx_usbv2_handle_canmsg(struct ixx_usb_device *dev,
		struct ixx_can_msg *rx)
{
	struct net_device *netdev = dev->netdev;
	struct can_frame *can_frame;
	struct sk_buff *skb;

	skb = alloc_can_skb(netdev, &can_frame);
	if (!skb)
		return -ENOMEM;

	if (le32_to_cpu(rx->flags) & IXXAT_USBV2_MSG_FLAGS_OVR) {
		netdev->stats.rx_over_errors++;
		netdev->stats.rx_errors++;
	}

	can_frame->can_id = le32_to_cpu(rx->msg_id);
	can_frame->can_dlc =
		(le32_to_cpu(rx->flags) &
		IXXAT_USBV2_MSG_FLAGS_DLC) >> 16;

	if (le32_to_cpu(rx->flags) & IXXAT_USBV2_MSG_FLAGS_EXT)
		can_frame->can_id |= CAN_EFF_FLAG;

	if (le32_to_cpu(rx->flags) & IXXAT_USBV2_MSG_FLAGS_RTR)
		can_frame->can_id |= CAN_RTR_FLAG;
	else
		memcpy(can_frame->data, rx->data, can_frame->can_dlc);

	ixxat_usb_get_ts_tv(dev, le32_to_cpu(rx->time), &skb->tstamp);

	netif_rx(skb);
	netdev->stats.rx_packets++;
	netdev->stats.rx_bytes += can_frame->can_dlc;

	return 0;
}

static int ixx_usbv2_handle_error(struct ixx_usb_device *dev,
		struct ixx_can_msg *rx)
{
	struct net_device *netdev = dev->netdev;
	struct can_frame *can_frame;
	struct sk_buff *skb;
	u8 raw_status = 0;

	/* nothing should be sent while in BUS_OFF state */
	if (dev->can.state == CAN_STATE_BUS_OFF)
		return 0;

	raw_status = rx->data[0];

	/* allocate an skb to store the error frame */
	skb = alloc_can_err_skb(netdev, &can_frame);
	if (!skb)
		return -ENOMEM;

	switch (raw_status) {
	case IXXAT_USBV2_CAN_ERROR_ACK:
		can_frame->can_id |= CAN_ERR_ACK;
		netdev->stats.tx_errors++;
		break;
	case IXXAT_USBV2_CAN_ERROR_BIT:
		can_frame->can_id |= CAN_ERR_PROT;
		can_frame->data[2] |= CAN_ERR_PROT_BIT;
		netdev->stats.rx_errors++;
		break;
	case IXXAT_USBV2_CAN_ERROR_CRC:
		can_frame->can_id |= CAN_ERR_PROT;
		can_frame->data[3] |= CAN_ERR_PROT_LOC_CRC_SEQ;
		netdev->stats.rx_errors++;
		break;
	case IXXAT_USBV2_CAN_ERROR_FORM:
		can_frame->can_id |= CAN_ERR_PROT;
		can_frame->data[2] |= CAN_ERR_PROT_FORM;
		netdev->stats.rx_errors++;
		break;
	case IXXAT_USBV2_CAN_ERROR_STUFF:
		can_frame->can_id |= CAN_ERR_PROT;
		can_frame->data[2] |= CAN_ERR_PROT_STUFF;
		netdev->stats.rx_errors++;
		break;
	case IXXAT_USBV2_CAN_ERROR_OTHER:
		can_frame->can_id |= CAN_ERR_PROT;
		can_frame->data[2] |= CAN_ERR_PROT_UNSPEC;
		netdev->stats.rx_errors++;
		break;
	default:
		can_frame->can_id |= CAN_ERR_PROT;
		netdev->stats.rx_errors++;
	}

	netif_rx(skb);
	netdev->stats.rx_packets++;
	netdev->stats.rx_bytes += can_frame->can_dlc;

	dev->bec.txerr = le16_to_cpu(rx->data[1]);
	dev->bec.rxerr = le16_to_cpu(rx->data[3]);

	return 0;
}

static int ixx_usbv2_handle_status(struct ixx_usb_device *dev,
		struct ixx_can_msg *rx)
{
	struct net_device *netdev = dev->netdev;
	struct can_frame *can_frame;
	struct sk_buff *skb;
	u8 raw_status = 0;
	u32 new_state = 0;

	raw_status = rx->data[0];

	/* nothing should be sent while in BUS_OFF state */
	if (dev->can.state == CAN_STATE_BUS_OFF)
		return 0;

	if (!raw_status) {
		/* no error bit (back to active state) */
		dev->can.state = CAN_STATE_ERROR_ACTIVE;

		dev->bec.txerr = 0;
		dev->bec.rxerr = 0;
		return 0;
	}

	/* allocate an skb to store the error frame */
	skb = alloc_can_err_skb(netdev, &can_frame);
	if (!skb)
		return -ENOMEM;

	if (raw_status & IXXAT_USBV2_CAN_STATUS_BUSOFF) {
		can_frame->can_id |= CAN_ERR_BUSOFF;
		new_state = CAN_STATE_BUS_OFF;
		dev->can.can_stats.bus_off++;
		can_bus_off(netdev);
	} else {
		if (raw_status & IXXAT_USBV2_CAN_STATUS_ERRLIM) {
			can_frame->can_id |= CAN_ERR_CRTL;
			can_frame->data[1] |= CAN_ERR_CRTL_TX_WARNING;
			can_frame->data[1] |= CAN_ERR_CRTL_RX_WARNING;
			dev->can.can_stats.error_warning++;
			new_state = CAN_STATE_ERROR_WARNING;
		}

		if (raw_status & IXXAT_USBV2_CAN_STATUS_OVRRUN) {
			can_frame->can_id |= CAN_ERR_PROT;
			can_frame->data[2] |= CAN_ERR_PROT_OVERLOAD;
			netdev->stats.rx_over_errors++;
			netdev->stats.rx_errors++;
		}

		if (!new_state) {
			new_state = CAN_STATE_ERROR_ACTIVE;

			dev->bec.txerr = 0;
			dev->bec.rxerr = 0;
		}
	}

	dev->can.state = new_state;

	netif_rx(skb);
	netdev->stats.rx_packets++;
	netdev->stats.rx_bytes += can_frame->can_dlc;

	return 0;
}

/*
 * callback for bulk IN urb
 */
static int ixx_usbv2_decode_buf(struct ixx_usb_device *dev, struct urb *urb)
{
	struct net_device *netdev = dev->netdev;
	struct ixx_can_msg *can_msg;
	u32 msg_end;
	int err = 0;
	u32 read_size = 0;
	u8 msg_type;
	u8 *data;

	data = urb->transfer_buffer;

	/* loop reading all the records from the incoming message */
	msg_end = urb->actual_length;
	for (; msg_end > 0;) {
		can_msg = (struct ixx_can_msg *) &data[read_size];

		if (!can_msg || !can_msg->size) {
			netdev_err(netdev, "got unsupported rec in usb msg:\n");
			err = -ENOTSUPP;
			break;
		}

		/* check if the record goes out of current packet */
		if ((read_size + can_msg->size + 1) > urb->actual_length) {
			netdev_err(netdev,
			"got frag rec: should inc usb rx buf size\n");
			err = -EBADMSG;
			break;
		}

		msg_type =
			(le32_to_cpu(can_msg->flags) &
			IXXAT_USBV2_MSG_FLAGS_TYPE);

		switch (msg_type) {

		case IXXAT_USBV2_CAN_DATA:
			err = ixx_usbv2_handle_canmsg(dev, can_msg);
			if (err < 0)
				goto fail;
			break;

		case IXXAT_USBV2_CAN_STATUS:
			err = ixx_usbv2_handle_status(dev, can_msg);
			if (err < 0)
				goto fail;
			break;

		case IXXAT_USBV2_CAN_ERROR:
			err = ixx_usbv2_handle_error(dev, can_msg);
			if (err < 0)
				goto fail;
			break;

		case IXXAT_USBV2_CAN_TIMEOVR:
			ixxat_usb_get_ts_tv(dev, can_msg->time, NULL);
			break;

		case IXXAT_USBV2_CAN_INFO:
		case IXXAT_USBV2_CAN_WAKEUP:
		case IXXAT_USBV2_CAN_TIMERST:
			break;

		default:
			netdev_err(netdev,
					"unhandled rec type 0x%02x (%d): ignored\n",
					msg_type, msg_type);
			break;
		}

		read_size += can_msg->size + 1;
		msg_end -= (can_msg->size + 1);
	}

fail:
	if (err)
		ixxat_dump_mem("received msg", urb->transfer_buffer,
				urb->actual_length);

	return err;
}

static int ixx_usbv2_encode_msg(struct ixx_usb_device *dev, struct sk_buff *skb,
		u8 *obuf, size_t *size)
{
	struct can_frame *cf = (struct can_frame *) skb->data;
	struct ixx_can_msg can_msg = { 0 };

	if (cf->can_id & CAN_RTR_FLAG)
		can_msg.flags |= IXXAT_USBV2_MSG_FLAGS_RTR;

	if (cf->can_id & CAN_EFF_FLAG) {
		can_msg.flags |= IXXAT_USBV2_MSG_FLAGS_EXT;
		can_msg.msg_id = cf->can_id & CAN_EFF_MASK;
	} else {
		can_msg.msg_id = cf->can_id & CAN_SFF_MASK;
	}

	if (dev->can.ctrlmode & CAN_CTRLMODE_ONE_SHOT)
		can_msg.flags |= IXXAT_USBV2_MSG_FLAGS_SSM;

	can_msg.flags |= (cf->can_dlc << 16) & IXXAT_USBV2_MSG_FLAGS_DLC;

	can_msg.flags = cpu_to_le32(can_msg.flags);
	can_msg.msg_id = cpu_to_le32(can_msg.msg_id);

	memcpy(can_msg.data, cf->data, cf->can_dlc);
	can_msg.size = (u8)(sizeof(can_msg) - 1 - CAN_MAX_DLEN + cf->can_dlc);

	memcpy(obuf, &can_msg, can_msg.size + 1);

	*size = can_msg.size + 1;

	skb->data_len = *size;

	return 0;
}

static int ixx_usbv2_start(struct ixx_usb_device *dev)
{
	int err;
	u32 time_ref = 0;
	u8 can_opmode = IXXAT_USBV2_OPMODE_EXTENDED
			| IXXAT_USBV2_OPMODE_STANDARD;

	if (dev->can.ctrlmode & CAN_CTRLMODE_BERR_REPORTING)
		can_opmode |= IXXAT_USBV2_OPMODE_ERRFRAME;
	if (dev->can.ctrlmode & CAN_CTRLMODE_LISTENONLY)
		can_opmode |= IXXAT_USBV2_OPMODE_LISTONLY;

	err = ixx_usbv2_init_ctrl(dev, can_opmode, dev->btr0, dev->btr1);
	if (err)
		return err;

	/* opening first device: */
	if (dev->ctrl_opened_count == 0) {
		err = ixx_usbv2_start_ctrl(dev, &time_ref);
		if (err)
			return err;

		ixxat_usb_set_ts_now(dev, time_ref);
	}

	dev->ctrl_opened_count++;

	dev->bec.txerr = 0;
	dev->bec.rxerr = 0;

	return err;
}

/*
 * stop interface
 * (last chance before set bus off)
 */
static int ixx_usbv2_stop(struct ixx_usb_device *dev)
{
	int err;

	if (dev->ctrl_opened_count == 1) {
		err = ixx_usbv2_stop_ctrl(dev);
		if (err)
			return err;
	}

	/* turn off ts msgs for that interface if no other dev opened */
//	if (pdev->usb_if->dev_opened_count == 1)
//		ixx_usbv2_set_ts(dev, 0);
	dev->ctrl_opened_count--;

	return 0;
}

/*
 * called when probing to initialize a device object.
 */
static int ixx_usbv2_init(struct ixx_usb_device *dev)
{
	dev->restart_task = kthread_run(&ixx_usbv2_restart_task, dev,
		"restart_thread");
	if (!dev->restart_task)
		return -ENOBUFS;

	return 0;
}

static void ixx_usbv2_exit(struct ixx_usb_device *dev)
{
	ixx_usbv2_reset_ctrl(dev);

	dev->must_quit = 1;
	dev->restart_flag = 1;
	wake_up_interruptible(&dev->wait_queue);
	if (dev->restart_task)
		kthread_stop(dev->restart_task);
}

/*
 * probe function for new IXXAT USB-to-CAN V2 interface
 */
static int ixx_usbv2_probe(struct usb_interface *intf)
{
	struct usb_host_interface *if_desc;
	int i;

	if_desc = intf->altsetting;

	/* check interface endpoint addresses */
	for (i = 0; i < if_desc->desc.bNumEndpoints; i++) {
		struct usb_endpoint_descriptor *ep = &if_desc->endpoint[i].desc;

		/*
		 * below is the list of valid ep addreses. Any other ep address
		 * is considered as not-CAN interface address => no dev created
		 */
		switch (ep->bEndpointAddress) {
		case IXXAT_USBV2_EP_MSGOUT_0:
		case IXXAT_USBV2_EP_MSGOUT_1:
		case IXXAT_USBV2_EP_MSGOUT_2:
		case IXXAT_USBV2_EP_MSGOUT_3:
		case IXXAT_USBV2_EP_MSGOUT_4:
		case IXXAT_USBV2_EP_MSGIN_0:
		case IXXAT_USBV2_EP_MSGIN_1:
		case IXXAT_USBV2_EP_MSGIN_2:
		case IXXAT_USBV2_EP_MSGIN_3:
		case IXXAT_USBV2_EP_MSGIN_4:

			break;
		default:
			return -ENODEV;
		}
	}

	return 0;
}

static int ixx_usbv2_get_dev_caps(struct usb_device *dev,
		struct ixx_dev_caps *dev_caps)
{
	int err = -ENODEV, i;
	u8 data[IXXAT_USBV2_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbv2_dev_caps_req *dev_caps_req;
	struct ixx_usbv2_dev_caps_res *dev_caps_res;
	u32 req_size = sizeof(*dev_caps_req);

	dev_caps_req = (struct ixx_usbv2_dev_caps_req *) data;
	dev_caps_res = (struct ixx_usbv2_dev_caps_res *)(data + req_size);

	dev_caps_req->dal_req.req_size   = cpu_to_le32(req_size);
	dev_caps_req->dal_req.req_code   =
		cpu_to_le32(IXXAT_USBV2_BRD_GET_DEVCAPS_CMD);
	dev_caps_req->dal_req.req_port   = 0xffff;
	dev_caps_req->dal_req.req_socket = 0xffff;

	dev_caps_res->dal_res.res_size =
		cpu_to_le32(sizeof(*dev_caps_res));
	dev_caps_res->dal_res.ret_size = 0;
	dev_caps_res->dal_res.ret_code = 0xffffffff;

	err = ixx_usbv2_send_cmd(dev, &dev_caps_req->dal_req);
	if (err < 0)
		return err;

	err = ixx_usbv2_rcv_cmd(dev, &dev_caps_res->dal_res,
			0xffff);
	if (err < 0)
		return err;

	dev_caps->bus_ctrl_count =
		le16_to_cpu(dev_caps_res->dev_caps.bus_ctrl_count);
	for (i = 0; i < dev_caps->bus_ctrl_count; ++i)
		dev_caps->bus_ctrl_types[i] =
			le16_to_cpu(dev_caps_res->dev_caps.bus_ctrl_types[i]);

	return 0;
}

static int ixx_usbv2_get_ctrl_caps(struct usb_device *dev,
		struct ixx_ctrl_caps *ctrl_caps, int index)
{
	int err = -ENODEV;
	u8 data[IXXAT_USBV2_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbv2_ctrl_caps_req *ctrl_caps_req;
	struct ixx_usbv2_ctrl_caps_res *ctrl_caps_res;
	u32 req_size = sizeof(*ctrl_caps_req);

	ctrl_caps_req = (struct ixx_usbv2_ctrl_caps_req *) data;
	ctrl_caps_res = (struct ixx_usbv2_ctrl_caps_res *)(data + req_size);

	ctrl_caps_req->dal_req.req_size   = cpu_to_le32(req_size);
	ctrl_caps_req->dal_req.req_code   =
		cpu_to_le32(IXXAT_USBV2_CAN_GET_CAPS_CMD);
	ctrl_caps_req->dal_req.req_port   = cpu_to_le16(index);
	ctrl_caps_req->dal_req.req_socket = 0xffff;

	ctrl_caps_res->dal_res.res_size =
		cpu_to_le32(sizeof(*ctrl_caps_res));
	ctrl_caps_res->dal_res.ret_size = 0;
	ctrl_caps_res->dal_res.ret_code = 0xffffffff;

	err = ixx_usbv2_send_cmd(dev, &ctrl_caps_req->dal_req);
	if (err < 0)
		return err;

	err = ixx_usbv2_rcv_cmd(dev, &ctrl_caps_res->dal_res,
			index);
	if (err < 0)
		return err;

	ctrl_caps->bus_coupling = le16_to_cpu(
			ctrl_caps_res->ctrl_caps.bus_coupling);
	ctrl_caps->clock_freq =
		le32_to_cpu(ctrl_caps_res->ctrl_caps.clock_freq);
	ctrl_caps->cms_divisor = le32_to_cpu(
		ctrl_caps_res->ctrl_caps.cms_divisor);
	ctrl_caps->cms_max_ticks = le32_to_cpu(
		ctrl_caps_res->ctrl_caps.cms_max_ticks);
	ctrl_caps->ctrl_type = le16_to_cpu(ctrl_caps_res->ctrl_caps.ctrl_type);
	ctrl_caps->dtx_divisor = le32_to_cpu(
		ctrl_caps_res->ctrl_caps.dtx_divisor);
	ctrl_caps->dtx_max_ticks = le32_to_cpu(
		ctrl_caps_res->ctrl_caps.dtx_max_ticks);
	ctrl_caps->features = le32_to_cpu(ctrl_caps_res->ctrl_caps.features);
	ctrl_caps->tsc_divisor = le32_to_cpu(
		ctrl_caps_res->ctrl_caps.tsc_divisor);

	return 0;
}

static int ixx_usbv2_get_fwinfo(struct ixx_usb_device *dev,
		struct ixx_intf_fw_info *fwinfo)
{
	int err = -ENODEV;
	u8 data[IXXAT_USBV2_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbv2_brd_get_fwinfo_req *fw_info_req;
	struct ixx_usbv2_brd_get_fwinfo_res *fw_info_res;
	u32 req_size = sizeof(*fw_info_req);

	fw_info_req = (struct ixx_usbv2_brd_get_fwinfo_req *) data;
	fw_info_res = (struct ixx_usbv2_brd_get_fwinfo_res *)(data + req_size);

	fw_info_req->dal_req.req_size   = cpu_to_le32(req_size);
	fw_info_req->dal_req.req_code   =
		cpu_to_le32(IXXAT_USBV2_BRD_GET_FWINFO_CMD);
	fw_info_req->dal_req.req_port   = 0xffff;
	fw_info_req->dal_req.req_socket = 0xffff;

	fw_info_res->dal_res.res_size =
		cpu_to_le32(sizeof(*fw_info_res));
	fw_info_res->dal_res.ret_size = 0;
	fw_info_res->dal_res.ret_code = 0xffffffff;

	err = ixx_usbv2_send_cmd(dev->udev, &fw_info_req->dal_req);
	if (err < 0)
		return err;

	err = ixx_usbv2_rcv_cmd(dev->udev,
		&fw_info_res->dal_res, 0xffff);
	if (err < 0)
		return err;

	if (fwinfo) {
		fwinfo->build_version =
			le16_to_cpu(fw_info_res->fwinfo.build_version);
		fwinfo->firmware_type =
			le32_to_cpu(fw_info_res->fwinfo.firmware_type);
		fwinfo->major_version =
			le16_to_cpu(fw_info_res->fwinfo.major_version);
		fwinfo->minor_version =
			le16_to_cpu(fw_info_res->fwinfo.minor_version);
		fwinfo->reserved =
			le16_to_cpu(fw_info_res->fwinfo.reserved);
	}

	return le32_to_cpu(fw_info_res->dal_res.ret_code);
}

static int ixx_usbv2_get_dev_info(struct ixx_usb_device *dev,
		struct ixx_intf_info *dev_info)
{
	int err = -ENODEV;
	u8 data[IXXAT_USBV2_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbv2_brd_get_intf_info_req *dev_info_req;
	struct ixx_usbv2_brd_get_intf_info_res *dev_info_res;
	u32 req_size = sizeof(*dev_info_req);

	dev_info_req = (struct ixx_usbv2_brd_get_intf_info_req *) data;
	dev_info_res =
		(struct ixx_usbv2_brd_get_intf_info_res *)(data + req_size);

	dev_info_req->dal_req.req_size   = cpu_to_le32(req_size);
	dev_info_req->dal_req.req_code   =
		cpu_to_le32(IXXAT_USBV2_BRD_GET_DEVINFO_CMD);
	dev_info_req->dal_req.req_port   = 0xffff;
	dev_info_req->dal_req.req_socket = 0xffff;

	dev_info_res->dal_res.res_size =
		cpu_to_le32(sizeof(*dev_info_res));
	dev_info_res->dal_res.ret_size = 0;
	dev_info_res->dal_res.ret_code = 0xffffffff;

	err = ixx_usbv2_send_cmd(dev->udev, &dev_info_req->dal_req);
	if (err < 0)
		return err;

	err = ixx_usbv2_rcv_cmd(dev->udev,
		&dev_info_res->dal_res, 0xffff);
	if (err < 0)
		return err;

	if (dev_info) {
		memcpy(dev_info->device_id, &dev_info_res->info.device_id,
			sizeof(dev_info_res->info.device_id));
		memcpy(dev_info->device_name, &dev_info_res->info.device_name,
			sizeof(dev_info_res->info.device_name));
		dev_info->device_fpga_version =
			le16_to_cpu(dev_info_res->info.device_fpga_version);
		dev_info->device_version =
			le32_to_cpu(dev_info_res->info.device_version);
	}

	return le32_to_cpu(dev_info_res->dal_res.ret_code);
}

/*
 * describe the describes the USB-to-CAN V2 compact adapter
 */
struct ixx_usb_adapter usb_to_can_v2_compact = {
	.name = "USB-to-CAN V2 compact",
	.device_id = USB_TO_CAN_V2_COMPACT_PRODUCT_ID,
	.clock = {
		.freq = SJA1000_CRYSTAL_HZ,
	},
	.bittiming_const = {
		.name = "ixxat_usb",
		.tseg1_min = 1,
		.tseg1_max = 16,
		.tseg2_min = 1,
		.tseg2_max = 8,
		.sjw_max = 4,
		.brp_min = 1,
		.brp_max = 64,
		.brp_inc = 1,
	},

	.ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
		CAN_CTRLMODE_BERR_REPORTING |
		CAN_CTRLMODE_LOOPBACK |
		CAN_CTRLMODE_LISTENONLY,

	/* size of device private data */
	.sizeof_dev_private = sizeof(struct ixx_usb_device),

	/* give here messages in/out endpoints */
	.ep_msg_in = {  IXXAT_USBV2_EP_MSGIN_0, IXXAT_USBV2_EP_MSGIN_1,
		IXXAT_USBV2_EP_MSGIN_2, IXXAT_USBV2_EP_MSGIN_3,
		IXXAT_USBV2_EP_MSGIN_4 },
	.ep_msg_out = { IXXAT_USBV2_EP_MSGOUT_0, IXXAT_USBV2_EP_MSGOUT_1,
		IXXAT_USBV2_EP_MSGOUT_2, IXXAT_USBV2_EP_MSGOUT_3,
		IXXAT_USBV2_EP_MSGOUT_4 },

	/* size of rx/tx usb buffers */
	.rx_buffer_size = IXXAT_USBV2_RX_BUFFER_SIZE,
	.tx_buffer_size = IXXAT_USBV2_TX_BUFFER_SIZE,

	/* device callbacks */
	.intf_probe = ixx_usbv2_probe,
	.dev_get_dev_caps = ixx_usbv2_get_dev_caps,
	.dev_get_ctrl_caps = ixx_usbv2_get_ctrl_caps,
	.dev_init = ixx_usbv2_init,
	.dev_exit = ixx_usbv2_exit,
	.dev_set_bittiming = ixx_usbv2_set_bittiming,
	.intf_get_info = ixx_usbv2_get_dev_info,
	.intf_get_fw_info = ixx_usbv2_get_fwinfo,
	.dev_decode_buf = ixx_usbv2_decode_buf,
	.dev_encode_msg = ixx_usbv2_encode_msg,
	.dev_start = ixx_usbv2_start,
	.dev_stop = ixx_usbv2_stop,
};

/*
 * describes the USB-to-CAN V2 automotive adapter
 */
struct ixx_usb_adapter usb_to_can_v2_automotive = {
	.name = "USB-to-CAN V2 automotive",
	.device_id = USB_TO_CAN_V2_AUTOMOTIVE_PRODUCT_ID,
	.clock = {
		.freq = SJA1000_CRYSTAL_HZ,
	},
	.bittiming_const = {
		.name = "ixxat_usb",
		.tseg1_min = 1,
		.tseg1_max = 16,
		.tseg2_min = 1,
		.tseg2_max = 8,
		.sjw_max = 4,
		.brp_min = 1,
		.brp_max = 64,
		.brp_inc = 1,
	},

	.ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
		CAN_CTRLMODE_BERR_REPORTING |
		CAN_CTRLMODE_LOOPBACK |
		CAN_CTRLMODE_LISTENONLY,

	/* size of device private data */
	.sizeof_dev_private = sizeof(struct ixx_usb_device),

	/* give here messages in/out endpoints */
	.ep_msg_in = {  IXXAT_USBV2_EP_MSGIN_0, IXXAT_USBV2_EP_MSGIN_1,
		IXXAT_USBV2_EP_MSGIN_2, IXXAT_USBV2_EP_MSGIN_3,
		IXXAT_USBV2_EP_MSGIN_4 },
	.ep_msg_out = { IXXAT_USBV2_EP_MSGOUT_0, IXXAT_USBV2_EP_MSGOUT_1,
		IXXAT_USBV2_EP_MSGOUT_2, IXXAT_USBV2_EP_MSGOUT_3,
		IXXAT_USBV2_EP_MSGOUT_4 },

	/* size of rx/tx usb buffers */
	.rx_buffer_size = IXXAT_USBV2_RX_BUFFER_SIZE,
	.tx_buffer_size = IXXAT_USBV2_TX_BUFFER_SIZE,

	/* device callbacks */
	.intf_probe = ixx_usbv2_probe,
	.dev_get_dev_caps = ixx_usbv2_get_dev_caps,
	.dev_get_ctrl_caps = ixx_usbv2_get_ctrl_caps,
	.dev_init = ixx_usbv2_init,
	.dev_exit = ixx_usbv2_exit,
	.dev_set_bittiming = ixx_usbv2_set_bittiming,
	.intf_get_info = ixx_usbv2_get_dev_info,
	.intf_get_fw_info = ixx_usbv2_get_fwinfo,
	.dev_decode_buf = ixx_usbv2_decode_buf,
	.dev_encode_msg = ixx_usbv2_encode_msg,
	.dev_start = ixx_usbv2_start,
	.dev_stop = ixx_usbv2_stop,
};

/*
 * describes the USB-to-CAN V2 embedded adapter
 */
struct ixx_usb_adapter usb_to_can_v2_embedded = {
	.name = "USB-to-CAN V2 embedded",
	.device_id = USB_TO_CAN_V2_EMBEDDED_PRODUCT_ID,
	.clock = {
		.freq = SJA1000_CRYSTAL_HZ,
	},
	.bittiming_const = {
		.name = "ixxat_usb",
		.tseg1_min = 1,
		.tseg1_max = 16,
		.tseg2_min = 1,
		.tseg2_max = 8,
		.sjw_max = 4,
		.brp_min = 1,
		.brp_max = 64,
		.brp_inc = 1,
	},

	.ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
		CAN_CTRLMODE_BERR_REPORTING |
		CAN_CTRLMODE_LOOPBACK |
		CAN_CTRLMODE_LISTENONLY,

	/* size of device private data */
	.sizeof_dev_private = sizeof(struct ixx_usb_device),

	/* give here messages in/out endpoints */
	.ep_msg_in = {  IXXAT_USBV2_EP_MSGIN_0, IXXAT_USBV2_EP_MSGIN_1,
		IXXAT_USBV2_EP_MSGIN_2, IXXAT_USBV2_EP_MSGIN_3,
		IXXAT_USBV2_EP_MSGIN_4 },
	.ep_msg_out = { IXXAT_USBV2_EP_MSGOUT_0, IXXAT_USBV2_EP_MSGOUT_1,
		IXXAT_USBV2_EP_MSGOUT_2, IXXAT_USBV2_EP_MSGOUT_3,
		IXXAT_USBV2_EP_MSGOUT_4 },

	/* size of rx/tx usb buffers */
	.rx_buffer_size = IXXAT_USBV2_RX_BUFFER_SIZE,
	.tx_buffer_size = IXXAT_USBV2_TX_BUFFER_SIZE,

	/* device callbacks */
	.intf_probe = ixx_usbv2_probe,
	.dev_get_dev_caps = ixx_usbv2_get_dev_caps,
	.dev_get_ctrl_caps = ixx_usbv2_get_ctrl_caps,
	.dev_init = ixx_usbv2_init,
	.dev_exit = ixx_usbv2_exit,
	.dev_set_bittiming = ixx_usbv2_set_bittiming,
	.intf_get_info = ixx_usbv2_get_dev_info,
	.intf_get_fw_info = ixx_usbv2_get_fwinfo,
	.dev_decode_buf = ixx_usbv2_decode_buf,
	.dev_encode_msg = ixx_usbv2_encode_msg,
	.dev_start = ixx_usbv2_start,
	.dev_stop = ixx_usbv2_stop,
};

/*
 * describes the USB-to-CAN V2 professional adapter
 */
struct ixx_usb_adapter usb_to_can_v2_professional = {
	.name = "USB-to-CAN V2 professional",
	.device_id = USB_TO_CAN_V2_PROFESSIONAL_PRODUCT_ID,
	.clock = {
		.freq = SJA1000_CRYSTAL_HZ,
	},
	.bittiming_const = {
		.name = "ixxat_usb",
		.tseg1_min = 1,
		.tseg1_max = 16,
		.tseg2_min = 1,
		.tseg2_max = 8,
		.sjw_max = 4,
		.brp_min = 1,
		.brp_max = 64,
		.brp_inc = 1,
	},

	.ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
		CAN_CTRLMODE_BERR_REPORTING |
		CAN_CTRLMODE_LOOPBACK |
		CAN_CTRLMODE_LISTENONLY,

	/* size of device private data */
	.sizeof_dev_private = sizeof(struct ixx_usb_device),

	/* give here messages in/out endpoints */
	.ep_msg_in = {  IXXAT_USBV2_EP_MSGIN_0, IXXAT_USBV2_EP_MSGIN_1,
		IXXAT_USBV2_EP_MSGIN_2, IXXAT_USBV2_EP_MSGIN_3,
		IXXAT_USBV2_EP_MSGIN_4 },
	.ep_msg_out = { IXXAT_USBV2_EP_MSGOUT_0, IXXAT_USBV2_EP_MSGOUT_1,
		IXXAT_USBV2_EP_MSGOUT_2, IXXAT_USBV2_EP_MSGOUT_3,
		IXXAT_USBV2_EP_MSGOUT_4 },

	/* size of rx/tx usb buffers */
	.rx_buffer_size = IXXAT_USBV2_RX_BUFFER_SIZE,
	.tx_buffer_size = IXXAT_USBV2_TX_BUFFER_SIZE,

	/* device callbacks */
	.intf_probe = ixx_usbv2_probe,
	.dev_get_dev_caps = ixx_usbv2_get_dev_caps,
	.dev_get_ctrl_caps = ixx_usbv2_get_ctrl_caps,
	.dev_init = ixx_usbv2_init,
	.dev_exit = ixx_usbv2_exit,
	.dev_set_bittiming = ixx_usbv2_set_bittiming,
	.intf_get_info = ixx_usbv2_get_dev_info,
	.intf_get_fw_info = ixx_usbv2_get_fwinfo,
	.dev_decode_buf = ixx_usbv2_decode_buf,
	.dev_encode_msg = ixx_usbv2_encode_msg,
	.dev_start = ixx_usbv2_start,
	.dev_stop = ixx_usbv2_stop,
};

/*
 * describes the USB-to-CAN V2 low speed adapter
 */
struct ixx_usb_adapter usb_to_can_v2_low_speed = {
	.name = "USB-to-CAN V2 low speed",
	.device_id = USB_TO_CAN_V2_LOW_SPEED_PRODUCT_ID,
	.clock = {
		.freq = SJA1000_CRYSTAL_HZ,
	},
	.bittiming_const = {
		.name = "ixxat_usb",
		.tseg1_min = 1,
		.tseg1_max = 16,
		.tseg2_min = 1,
		.tseg2_max = 8,
		.sjw_max = 4,
		.brp_min = 1,
		.brp_max = 64,
		.brp_inc = 1,
	},

	.ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
		CAN_CTRLMODE_BERR_REPORTING |
		CAN_CTRLMODE_LOOPBACK |
		CAN_CTRLMODE_LISTENONLY,

	/* size of device private data */
	.sizeof_dev_private = sizeof(struct ixx_usb_device),

	/* give here messages in/out endpoints */
	.ep_msg_in = {  IXXAT_USBV2_EP_MSGIN_0, IXXAT_USBV2_EP_MSGIN_1,
		IXXAT_USBV2_EP_MSGIN_2, IXXAT_USBV2_EP_MSGIN_3,
		IXXAT_USBV2_EP_MSGIN_4 },
	.ep_msg_out = { IXXAT_USBV2_EP_MSGOUT_0, IXXAT_USBV2_EP_MSGOUT_1,
		IXXAT_USBV2_EP_MSGOUT_2, IXXAT_USBV2_EP_MSGOUT_3,
		IXXAT_USBV2_EP_MSGOUT_4 },

	/* size of rx/tx usb buffers */
	.rx_buffer_size = IXXAT_USBV2_RX_BUFFER_SIZE,
	.tx_buffer_size = IXXAT_USBV2_TX_BUFFER_SIZE,

	/* device callbacks */
	.intf_probe = ixx_usbv2_probe,
	.dev_get_dev_caps = ixx_usbv2_get_dev_caps,
	.dev_get_ctrl_caps = ixx_usbv2_get_ctrl_caps,
	.dev_init = ixx_usbv2_init,
	.dev_exit = ixx_usbv2_exit,
	.dev_set_bittiming = ixx_usbv2_set_bittiming,
	.intf_get_info = ixx_usbv2_get_dev_info,
	.intf_get_fw_info = ixx_usbv2_get_fwinfo,
	.dev_decode_buf = ixx_usbv2_decode_buf,
	.dev_encode_msg = ixx_usbv2_encode_msg,
	.dev_start = ixx_usbv2_start,
	.dev_stop = ixx_usbv2_stop,
};

/*
 * describes the USB-to-CAN V2 extended adapter
 */
struct ixx_usb_adapter usb_to_can_v2_extended = {
	.name = "USB-to-CAN V2 extended",
	.device_id = USB_TO_CAN_V2_EXTENDED_PRODUCT_ID,
	.clock = {
		.freq = SJA1000_CRYSTAL_HZ,
	},
	.bittiming_const = {
		.name = "ixxat_usb",
		.tseg1_min = 1,
		.tseg1_max = 16,
		.tseg2_min = 1,
		.tseg2_max = 8,
		.sjw_max = 4,
		.brp_min = 1,
		.brp_max = 64,
		.brp_inc = 1,
	},

	.ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
		CAN_CTRLMODE_BERR_REPORTING |
		CAN_CTRLMODE_LOOPBACK |
		CAN_CTRLMODE_LISTENONLY,

	/* size of device private data */
	.sizeof_dev_private = sizeof(struct ixx_usb_device),

	/* give here messages in/out endpoints */
	.ep_msg_in = {  IXXAT_USBV2_EP_MSGIN_0, IXXAT_USBV2_EP_MSGIN_1,
		IXXAT_USBV2_EP_MSGIN_2, IXXAT_USBV2_EP_MSGIN_3,
		IXXAT_USBV2_EP_MSGIN_4 },
	.ep_msg_out = { IXXAT_USBV2_EP_MSGOUT_0, IXXAT_USBV2_EP_MSGOUT_1,
		IXXAT_USBV2_EP_MSGOUT_2, IXXAT_USBV2_EP_MSGOUT_3,
		IXXAT_USBV2_EP_MSGOUT_4 },

	/* size of rx/tx usb buffers */
	.rx_buffer_size = IXXAT_USBV2_RX_BUFFER_SIZE,
	.tx_buffer_size = IXXAT_USBV2_TX_BUFFER_SIZE,

	/* device callbacks */
	.intf_probe = ixx_usbv2_probe,
	.dev_get_dev_caps = ixx_usbv2_get_dev_caps,
	.dev_get_ctrl_caps = ixx_usbv2_get_ctrl_caps,
	.dev_init = ixx_usbv2_init,
	.dev_exit = ixx_usbv2_exit,
	.dev_set_bittiming = ixx_usbv2_set_bittiming,
	.intf_get_info = ixx_usbv2_get_dev_info,
	.intf_get_fw_info = ixx_usbv2_get_fwinfo,
	.dev_decode_buf = ixx_usbv2_decode_buf,
	.dev_encode_msg = ixx_usbv2_encode_msg,
	.dev_start = ixx_usbv2_start,
	.dev_stop = ixx_usbv2_stop,
};
