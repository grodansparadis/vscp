/*
 * CAN driver for IXXAT USB-to-CAN FD
 *
 * Copyright (C) 2017 Michael Hengler <mhengler@ixxat.de>
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

#ifdef CANFD_CAPABLE

MODULE_SUPPORTED_DEVICE("IXXAT Automation GmbH USB-to-CAN FD");

/* use ifi can fd clock due to internal bittiming calculations */
#define IFIFD_CRYSTAL_HZ	      80000000

/* usb-to-can fd Endpoints */
#define IXXAT_USBFD_EP_CMDOUT   0
#define IXXAT_USBFD_EP_CMDIN    (IXXAT_USBFD_EP_CMDOUT | USB_DIR_IN)
#define IXXAT_USBFD_EP_MSGOUT_0 1
#define IXXAT_USBFD_EP_MSGIN_0  (IXXAT_USBFD_EP_MSGOUT_0 | USB_DIR_IN)
#define IXXAT_USBFD_EP_MSGOUT_1 2
#define IXXAT_USBFD_EP_MSGIN_1  (IXXAT_USBFD_EP_MSGOUT_1 | USB_DIR_IN)
#define IXXAT_USBFD_EP_MSGOUT_2 3
#define IXXAT_USBFD_EP_MSGIN_2  (IXXAT_USBFD_EP_MSGOUT_2 | USB_DIR_IN)
#define IXXAT_USBFD_EP_MSGOUT_3 4
#define IXXAT_USBFD_EP_MSGIN_3  (IXXAT_USBFD_EP_MSGOUT_3 | USB_DIR_IN)
#define IXXAT_USBFD_EP_MSGOUT_4 5
#define IXXAT_USBFD_EP_MSGIN_4  (IXXAT_USBFD_EP_MSGOUT_4 | USB_DIR_IN)

/* usb-to-can fd rx/tx buffers size */
#define IXXAT_USBFD_RX_BUFFER_SIZE          512
#define IXXAT_USBFD_TX_BUFFER_SIZE          512

#define IXXAT_USBFD_CMD_BUFFER_SIZE         256

/* reception of 11-bit id messages */
#define IXXAT_USBFD_OPMODE_STANDARD         0x01
/* reception of 29-bit id messages */
#define IXXAT_USBFD_OPMODE_EXTENDED         0x02
/* enable reception of error frames */
#define IXXAT_USBFD_OPMODE_ERRFRAME         0x04
/* listen only mode (TX passive) */
#define IXXAT_USBFD_OPMODE_LISTONLY         0x08

/* no extended operation */
#define IXXAT_USBFD_EXMODE_DISABLED         0x00
/* extended data length */
#define IXXAT_USBFD_EXMODE_EXTDATA          0x01
/* fast data bit rate */
#define IXXAT_USBFD_EXMODE_FASTDATA         0x02
/* ISO conform CAN-FD frame */
#define IXXAT_USBFD_EXMODE_ISOFD            0x04

/* Stuff error */
#define IXXAT_USBFD_CAN_ERROR_STUFF         1
/* Form error */
#define IXXAT_USBFD_CAN_ERROR_FORM          2
/* Acknowledgment error */
#define IXXAT_USBFD_CAN_ERROR_ACK           3
/* Bit error */
#define IXXAT_USBFD_CAN_ERROR_BIT           4
/* Fast data bit rate error */
#define IXXAT_USBFD_CAN_ERROR_FAST_DATA     5
/* CRC error */
#define IXXAT_USBFD_CAN_ERROR_CRC           6
/* Other (unspecified) error */
#define IXXAT_USBFD_CAN_ERROR_OTHER         7

/* Data overrun occurred */
#define IXXAT_USBFD_CAN_STATUS_OVRRUN    0x02
/* Error warning limit exceeded */
#define IXXAT_USBFD_CAN_STATUS_ERRLIM    0x04
/* Bus off status */
#define IXXAT_USBFD_CAN_STATUS_BUSOFF    0x08

#define IXXAT_USBFD_CAN_DATA             0x00
#define IXXAT_USBFD_CAN_INFO             0x01
#define IXXAT_USBFD_CAN_ERROR            0x02
#define IXXAT_USBFD_CAN_STATUS           0x03
#define IXXAT_USBFD_CAN_WAKEUP           0x04
#define IXXAT_USBFD_CAN_TIMEOVR          0x05
#define IXXAT_USBFD_CAN_TIMERST          0x06


#define IXXAT_USBFD_MSG_FLAGS_TYPE   0x000000FF
#define IXXAT_USBFD_MSG_FLAGS_SSM    0x00000100
#define IXXAT_USBFD_MSG_FLAGS_HPM    0x00000200
#define IXXAT_USBFD_MSG_FLAGS_EDL    0x00000400
#define IXXAT_USBFD_MSG_FLAGS_FDR    0x00000800
#define IXXAT_USBFD_MSG_FLAGS_ESI    0x00001000
#define IXXAT_USBFD_MSG_FLAGS_RES    0x0000E000
#define IXXAT_USBFD_MSG_FLAGS_DLC    0x000F0000
#define IXXAT_USBFD_MSG_FLAGS_OVR    0x00100000
#define IXXAT_USBFD_MSG_FLAGS_SRR    0x00200000
#define IXXAT_USBFD_MSG_FLAGS_RTR    0x00400000
#define IXXAT_USBFD_MSG_FLAGS_EXT    0x00800000
#define IXXAT_USBFD_MSG_FLAGS_AFC    0xFF000000

#define IXXAT_USBFD_BAL_CMD_CLASS    3
#define IXXAT_USBFD_BRD_CMD_CLASS    4

#define IXXAT_USBFD_BRD_CMD_CAT      0
#define IXXAT_USBFD_CAN_CMD_CAT      1

#define IXXAT_USBFD_VCI_CMD_CODE(Class, Function) \
	((u32) (((Class) << 8) | (Function)))

#define IXXAT_USBFD_BRD_CMD_CODE(Category, Function) \
	IXXAT_USBFD_VCI_CMD_CODE(IXXAT_USBFD_BRD_CMD_CLASS, \
		((Category) << 5) | (Function))

#define IXXAT_USBFD_BAL_CMD_CODE(Category, Function) \
	IXXAT_USBFD_VCI_CMD_CODE(IXXAT_USBFD_BAL_CMD_CLASS, \
		((Category) << 5) | (Function))

#define IXXAT_USBFD_CAN_GET_CAPS_CMD \
	IXXAT_USBFD_BAL_CMD_CODE(IXXAT_USBFD_CAN_CMD_CAT, 0)
#define IXXAT_USBFD_POWER_CMD \
	IXXAT_USBFD_BRD_CMD_CODE(IXXAT_USBFD_CAN_CMD_CAT, 1)
#define IXXAT_USBFD_CAN_INIT_CMD \
	IXXAT_USBFD_BAL_CMD_CODE(IXXAT_USBFD_CAN_CMD_CAT, 5)
#define IXXAT_USBFD_CAN_START_CMD \
	IXXAT_USBFD_BAL_CMD_CODE(IXXAT_USBFD_CAN_CMD_CAT, 6)
#define IXXAT_USBFD_CAN_STOP_CMD \
	IXXAT_USBFD_BAL_CMD_CODE(IXXAT_USBFD_CAN_CMD_CAT, 7)
#define IXXAT_USBFD_CAN_RESET_CMD \
	IXXAT_USBFD_BAL_CMD_CODE(IXXAT_USBFD_CAN_CMD_CAT, 8)
/* Additional commands for USB-to-CAN FD */
#define IXXAT_USBFD_INIT_V2_CMD \
	IXXAT_USBFD_BAL_CMD_CODE(IXXAT_USBFD_CAN_CMD_CAT, 23)

#define IXXAT_USBFD_BRD_GET_FWINFO_CMD \
	IXXAT_USBFD_BRD_CMD_CODE(IXXAT_USBFD_BRD_CMD_CAT, 0)
#define IXXAT_USBFD_BRD_GET_DEVCAPS_CMD \
	IXXAT_USBFD_BRD_CMD_CODE(IXXAT_USBFD_BRD_CMD_CAT, 1)
#define IXXAT_USBFD_BRD_GET_DEVINFO_CMD \
	IXXAT_USBFD_BRD_CMD_CODE(IXXAT_USBFD_BRD_CMD_CAT, 2)

struct ixx_usbfd_dal_req {
	u32 req_size;
	u16 req_port;
	u16 req_socket;
	u32 req_code;
} __packed;

struct ixx_usbfd_dal_res {
	u32 res_size;
	u32 ret_size;
	u32 ret_code;
} __packed;

// Additional structures for the for USB-to-CAN FD

struct ixx_usbfd_dev_power_req {
	struct ixx_usbfd_dal_req dal_req;
	u8  mode;
	u8  _padding1;
	u16 _padding2;
} __packed;

struct ixx_usbfd_dev_power_res {
	struct ixx_usbfd_dal_res dal_res;
} __packed;

struct ixx_usbfd_ctrl_init_v2_req {
	struct ixx_usbfd_dal_req dal_req;
	u8                opmode;
	u8                exmode;
	struct canbtp	    sdr;
	struct canbtp	    fdr;
	u16		      _padding;
} __packed;

struct ixx_usbfd_ctrl_init_v2_res {
	struct ixx_usbfd_dal_res dal_res;
} __packed;

struct ixx_usbfd_dev_caps_req {
	struct ixx_usbfd_dal_req dal_req;
} __packed;

struct ixx_usbfd_dev_caps_res {
	struct ixx_usbfd_dal_res dal_res;
	struct ixx_dev_caps dev_caps;
} __packed;

struct ixx_usbfd_ctrl_caps_req {
	struct ixx_usbfd_dal_req dal_req;
} __packed;

struct ixx_usbfd_ctrl_caps_res {
	struct ixx_usbfd_dal_res dal_res;
	struct ixx_ctrl_caps ctrl_caps;
} __packed;

struct ixx_usbfd_ctrl_init_req {
	struct ixx_usbfd_dal_req dal_req;
	u8 mode;
	u8 btr0;
	u8 btr1;
	u8 padding;
} __packed;

struct ixx_usbfd_ctrl_init_res {
	struct ixx_usbfd_dal_res dal_res;
} __packed;

struct ixx_usbfd_ctrl_start_req {
	struct ixx_usbfd_dal_req dal_req;
} __packed;

struct ixx_usbfd_ctrl_start_res {
	struct ixx_usbfd_dal_res dal_res;
	u32 start_time;
} __packed;

struct ixx_usbfd_ctrl_stop_req {
	struct ixx_usbfd_dal_req dal_req;
	u32 action;
} __packed;

struct ixx_usbfd_ctrl_stop_res {
	struct ixx_usbfd_dal_res dal_res;
} __packed;

struct ixx_usbfd_brd_get_fwinfo_req {
	struct ixx_usbfd_dal_req dal_req;
} __packed;

struct ixx_usbfd_brd_get_fwinfo_res {
	struct ixx_usbfd_dal_res dal_res;
	struct ixx_intf_fw_info fwinfo;
} __packed;

struct ixx_usbfd_brd_get_intf_info_req {
	struct ixx_usbfd_dal_req dal_req;
} __packed;

struct ixx_usbfd_brd_get_intf_info_res {
	struct ixx_usbfd_dal_res dal_res;
	struct ixx_intf_info info;
} __packed;

/*
 * send usb-to-can fd command synchronously
 */
static int ixx_usbfd_send_cmd(struct usb_device *dev,
		struct ixx_usbfd_dal_req *dal_req)
{
	int err, i;
	u16 size, value;
	u8  request, requesttype;
	u8 *buf;

	request     = 0xff;
	requesttype = USB_TYPE_VENDOR | USB_DIR_OUT;
	value       = le16_to_cpu(dal_req->req_port);
	size        = le32_to_cpu(dal_req->req_size) +
		sizeof(const struct ixx_usbfd_dal_res);

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
 * receive usb-to-can fd command synchronously
 */
static int ixx_usbfd_rcv_cmd(struct usb_device *dev,
		struct ixx_usbfd_dal_res *dal_res, int value)
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

static int ixx_usbfd_init_ctrl(struct ixx_usb_device *dev, u8 mode,
			u8 exmode,
			struct can_bittiming *arbitration_phase,
			struct can_bittiming *data_phase)
{
	int err = -ENODEV;
	u8 data[IXXAT_USBFD_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbfd_ctrl_init_v2_req *ctrl_init_req;
	struct ixx_usbfd_ctrl_init_v2_res *ctrl_init_res;
	u32 req_size = sizeof(*ctrl_init_req);

	ctrl_init_req = (struct ixx_usbfd_ctrl_init_v2_req *) data;
	ctrl_init_res = (struct ixx_usbfd_ctrl_init_v2_res *)(data + req_size);

	ctrl_init_req->dal_req.req_size   = cpu_to_le32(req_size);
	ctrl_init_req->dal_req.req_code   =
		cpu_to_le32(IXXAT_USBFD_INIT_V2_CMD);
	ctrl_init_req->dal_req.req_port   = cpu_to_le16(dev->ctrl_idx);
	ctrl_init_req->dal_req.req_socket = 0xffff;
	ctrl_init_req->opmode             = mode;
	ctrl_init_req->exmode             = exmode;

	ctrl_init_req->sdr.mode = cpu_to_le32(IXX_BTMODE_NAT);
	ctrl_init_req->sdr.bps  = cpu_to_le32(arbitration_phase->brp);
	ctrl_init_req->sdr.ts1  =
		cpu_to_le16(arbitration_phase->prop_seg +
		arbitration_phase->phase_seg1);
	ctrl_init_req->sdr.ts2  = cpu_to_le16(arbitration_phase->phase_seg2);
	ctrl_init_req->sdr.sjw  = cpu_to_le16(arbitration_phase->sjw);
	ctrl_init_req->sdr.tdo  = 0;

	if (exmode) {
		ctrl_init_req->fdr.mode = cpu_to_le32(IXX_BTMODE_NAT);
		ctrl_init_req->fdr.bps  = cpu_to_le32(data_phase->brp);
		ctrl_init_req->fdr.ts1  =
			cpu_to_le16(data_phase->prop_seg +
			data_phase->phase_seg1);
		ctrl_init_req->fdr.ts2  = cpu_to_le16(data_phase->phase_seg2);
		ctrl_init_req->fdr.sjw  = cpu_to_le16(data_phase->sjw);
		ctrl_init_req->fdr.tdo  =
			cpu_to_le16((1 + data_phase->phase_seg1 +
				data_phase->prop_seg) *
			data_phase->brp);
	}

	ctrl_init_res->dal_res.res_size =
		cpu_to_le32(sizeof(*ctrl_init_res));
	ctrl_init_res->dal_res.ret_size = 0;
	ctrl_init_res->dal_res.ret_code = 0xffffffff;

	err = ixx_usbfd_send_cmd(dev->udev, &ctrl_init_req->dal_req);
	if (err < 0)
		return err;

	err = ixx_usbfd_rcv_cmd(dev->udev,
		&ctrl_init_res->dal_res,
		dev->ctrl_idx);
	if (err < 0)
		return err;

	return le32_to_cpu(ctrl_init_res->dal_res.ret_code);
}

static int ixx_usbfd_start_ctrl(struct ixx_usb_device *dev, u32 *time_ref)
{
	int err = -ENODEV;
	u8 data[IXXAT_USBFD_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbfd_ctrl_start_req *ctrl_start_req;
	struct ixx_usbfd_ctrl_start_res *ctrl_start_res;
	u32 req_size = sizeof(*ctrl_start_req);

	ctrl_start_req = (struct ixx_usbfd_ctrl_start_req *) data;
	ctrl_start_res = (struct ixx_usbfd_ctrl_start_res *)(data + req_size);

	ctrl_start_req->dal_req.req_size   = cpu_to_le32(req_size);
	ctrl_start_req->dal_req.req_code   =
		cpu_to_le32(IXXAT_USBFD_CAN_START_CMD);
	ctrl_start_req->dal_req.req_port   = cpu_to_le16(dev->ctrl_idx);
	ctrl_start_req->dal_req.req_socket = 0xffff;

	ctrl_start_res->dal_res.res_size =
		cpu_to_le32(sizeof(*ctrl_start_res));
	ctrl_start_res->dal_res.ret_size = 0;
	ctrl_start_res->dal_res.ret_code = 0xffffffff;
	ctrl_start_res->start_time       = 0;

	err = ixx_usbfd_send_cmd(dev->udev, &ctrl_start_req->dal_req);
	if (err < 0)
		return err;

	err = ixx_usbfd_rcv_cmd(dev->udev,
		&ctrl_start_res->dal_res,
		dev->ctrl_idx);
	if (err < 0)
		return err;

	if (time_ref)
		*time_ref = le32_to_cpu(ctrl_start_res->start_time);

	return le32_to_cpu(ctrl_start_res->dal_res.ret_code);
}

static int ixx_usbfd_stop_ctrl(struct ixx_usb_device *dev)
{
	int err = -ENODEV;
	u8 data[IXXAT_USBFD_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbfd_ctrl_stop_req *ctrl_stop_req;
	struct ixx_usbfd_ctrl_stop_res *ctrl_stop_res;
	u32 req_size = sizeof(*ctrl_stop_req);

	ctrl_stop_req = (struct ixx_usbfd_ctrl_stop_req *) data;
	ctrl_stop_res = (struct ixx_usbfd_ctrl_stop_res *)(data + req_size);

	ctrl_stop_req->dal_req.req_size   = cpu_to_le32(req_size);
	ctrl_stop_req->dal_req.req_code   =
		cpu_to_le32(IXXAT_USBFD_CAN_STOP_CMD);
	ctrl_stop_req->dal_req.req_port   = cpu_to_le16(dev->ctrl_idx);
	ctrl_stop_req->dal_req.req_socket = 0xffff;
	ctrl_stop_req->action             = cpu_to_le32(0x3);

	ctrl_stop_res->dal_res.res_size =
		cpu_to_le32(sizeof(*ctrl_stop_res));
	ctrl_stop_res->dal_res.ret_size = 0;
	ctrl_stop_res->dal_res.ret_code = 0xffffffff;

	err = ixx_usbfd_send_cmd(dev->udev, &ctrl_stop_req->dal_req);
	if (err < 0)
		return err;

	err = ixx_usbfd_rcv_cmd(dev->udev,
		&ctrl_stop_res->dal_res,
		dev->ctrl_idx);
	if (err < 0)
		return err;

	if (!le32_to_cpu(ctrl_stop_res->dal_res.ret_code))
		dev->can.state = CAN_STATE_STOPPED;

	return le32_to_cpu(ctrl_stop_res->dal_res.ret_code);
}

static int ixx_usbfd_reset_ctrl(struct ixx_usb_device *dev)
{
	int err = -ENODEV;
	u8 data[IXXAT_USBFD_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbfd_dal_req *dal_req;
	struct ixx_usbfd_dal_res *dal_res;
	u32 req_size = sizeof(*dal_req);

	dal_req = (struct ixx_usbfd_dal_req *) data;
	dal_res = (struct ixx_usbfd_dal_res *)(data + req_size);

	dal_req->req_size   = cpu_to_le32(req_size);
	dal_req->req_code   = cpu_to_le32(IXXAT_USBFD_CAN_RESET_CMD);
	dal_req->req_port   = cpu_to_le16(dev->ctrl_idx);
	dal_req->req_socket = 0xffff;

	dal_res->res_size = cpu_to_le32(sizeof(*dal_res));
	dal_res->ret_size = 0;
	dal_res->ret_code = 0xffffffff;

	err = ixx_usbfd_send_cmd(dev->udev, dal_req);
	if (err < 0)
		return err;

	err = ixx_usbfd_rcv_cmd(dev->udev, dal_res,
			dev->ctrl_idx);
	if (err < 0)
		return err;

	return le32_to_cpu(dal_res->ret_code);
}

static int ixx_usbfd_power_ctrl(struct usb_device *dev, u8 mode)
{
	int err = -ENODEV;
	u8 data[IXXAT_USBFD_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbfd_dev_power_req *ctrl_power_req;
	struct ixx_usbfd_dev_power_res *ctrl_power_res;
	u32 req_size = sizeof(*ctrl_power_req);

	ctrl_power_req = (struct ixx_usbfd_dev_power_req *) data;
	ctrl_power_res = (struct ixx_usbfd_dev_power_res *)(data + req_size);

	ctrl_power_req->dal_req.req_size   = cpu_to_le32(req_size);
	ctrl_power_req->dal_req.req_code   =
		cpu_to_le32(IXXAT_USBFD_POWER_CMD);
	ctrl_power_req->dal_req.req_port   = cpu_to_le16(0xffff);
	ctrl_power_req->dal_req.req_socket = 0xffff;
	ctrl_power_req->mode               = mode;

	ctrl_power_res->dal_res.res_size =
		cpu_to_le32(sizeof(*ctrl_power_res));
	ctrl_power_res->dal_res.ret_size = 0;
	ctrl_power_res->dal_res.ret_code = 0xffffffff;

	err = ixx_usbfd_send_cmd(dev, &ctrl_power_req->dal_req);
	if (err < 0)
		return err;

	err = ixx_usbfd_rcv_cmd(dev,
		&ctrl_power_res->dal_res,
		0xffff);
	if (err < 0)
		return err;

	return le32_to_cpu(ctrl_power_res->dal_res.ret_code);
}

/*
 * handle restart but in asynchronously way
 */
static int ixx_usbfd_restart_task(void *user_data)
{
	u32 time_ref;
	struct ixx_usb_device *dev = user_data;

	while (!kthread_should_stop()) {
		if (!dev->must_quit) {
			wait_event_interruptible(dev->wait_queue,
					dev->restart_flag);
			if (!dev->must_quit) {
				ixx_usbfd_stop_ctrl(dev);
				ixx_usbfd_start_ctrl(dev, &time_ref);
				dev->restart_flag = 0;
				dev->can.state = CAN_STATE_ERROR_ACTIVE;
			}
		} else
			msleep(20);
	}
	return 0;
}

static int ixx_usbfd_handle_canmsg(struct ixx_usb_device *dev,
		struct ixx_can_msg_v2 *rx)
{
	struct net_device *netdev = dev->netdev;
	struct canfd_frame *can_frame;
	struct sk_buff *skb;
	const u32 flags = le32_to_cpu(rx->flags);

	if (flags & IXXAT_USBFD_MSG_FLAGS_EDL)
		skb = alloc_canfd_skb(netdev, &can_frame);
	else
		skb = alloc_can_skb(netdev, (struct can_frame **)&can_frame);

	if (!skb)
		return -ENOMEM;

	if (flags & IXXAT_USBFD_MSG_FLAGS_EDL) {
		if (flags & IXXAT_USBFD_MSG_FLAGS_FDR)
			can_frame->flags |= CANFD_BRS;

		if (flags & IXXAT_USBFD_MSG_FLAGS_ESI)
			can_frame->flags |= CANFD_ESI;

		can_frame->len =
			can_dlc2len(
			get_canfd_dlc((flags & IXXAT_USBFD_MSG_FLAGS_DLC)
			>> 16));
	}	else {
		can_frame->len =
			get_canfd_dlc((flags & IXXAT_USBFD_MSG_FLAGS_DLC)
			>> 16);
	}

	if (flags & IXXAT_USBFD_MSG_FLAGS_OVR) {
		netdev->stats.rx_over_errors++;
		netdev->stats.rx_errors++;
	}

	can_frame->can_id = le32_to_cpu(rx->msg_id);

	if (flags & IXXAT_USBFD_MSG_FLAGS_EXT)
		can_frame->can_id |= CAN_EFF_FLAG;

	if (flags & IXXAT_USBFD_MSG_FLAGS_RTR)
		can_frame->can_id |= CAN_RTR_FLAG;
	else
		memcpy(can_frame->data, rx->data, can_frame->len);

	ixxat_usb_get_ts_tv(dev, le32_to_cpu(rx->time), &skb->tstamp);

	netif_rx(skb);
	netdev->stats.rx_packets++;
	netdev->stats.rx_bytes += can_frame->len;

	return 0;
}

static int ixx_usbfd_handle_error(struct ixx_usb_device *dev,
		struct ixx_can_msg_v2 *rx)
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
	case IXXAT_USBFD_CAN_ERROR_ACK:
		can_frame->can_id |= CAN_ERR_ACK;
		netdev->stats.tx_errors++;
		break;
	case IXXAT_USBFD_CAN_ERROR_BIT:
		can_frame->can_id |= CAN_ERR_PROT;
		can_frame->data[2] |= CAN_ERR_PROT_BIT;
		netdev->stats.rx_errors++;
		break;
	case IXXAT_USBFD_CAN_ERROR_CRC:
		can_frame->can_id |= CAN_ERR_PROT;
		can_frame->data[3] |= CAN_ERR_PROT_LOC_CRC_SEQ;
		netdev->stats.rx_errors++;
		break;
	case IXXAT_USBFD_CAN_ERROR_FORM:
		can_frame->can_id |= CAN_ERR_PROT;
		can_frame->data[2] |= CAN_ERR_PROT_FORM;
		netdev->stats.rx_errors++;
		break;
	case IXXAT_USBFD_CAN_ERROR_STUFF:
		can_frame->can_id |= CAN_ERR_PROT;
		can_frame->data[2] |= CAN_ERR_PROT_STUFF;
		netdev->stats.rx_errors++;
		break;
	case IXXAT_USBFD_CAN_ERROR_OTHER:
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

static int ixx_usbfd_handle_status(struct ixx_usb_device *dev,
		struct ixx_can_msg_v2 *rx)
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

	if (raw_status & IXXAT_USBFD_CAN_STATUS_BUSOFF) {
		can_frame->can_id |= CAN_ERR_BUSOFF;
		new_state = CAN_STATE_BUS_OFF;
		dev->can.can_stats.bus_off++;
		can_bus_off(netdev);
	} else {
		if (raw_status & IXXAT_USBFD_CAN_STATUS_ERRLIM) {
			can_frame->can_id |= CAN_ERR_CRTL;
			can_frame->data[1] |= CAN_ERR_CRTL_TX_WARNING;
			can_frame->data[1] |= CAN_ERR_CRTL_RX_WARNING;
			dev->can.can_stats.error_warning++;
			new_state = CAN_STATE_ERROR_WARNING;
		}

		if (raw_status & IXXAT_USBFD_CAN_STATUS_OVRRUN) {
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
static int ixx_usbfd_decode_buf(struct ixx_usb_device *dev, struct urb *urb)
{
	struct net_device *netdev = dev->netdev;
	struct ixx_can_msg_v2 *can_msg;
	u32 msg_end;
	int err = 0;
	u32 read_size = 0;
	u8 msg_type;

	u8 *data = urb->transfer_buffer;

	/* loop reading all the records from the incoming message */
	msg_end = urb->actual_length;
	for (; msg_end > 0;) {
		can_msg = (struct ixx_can_msg_v2 *) &data[read_size];

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

		msg_type = (le32_to_cpu(can_msg->flags) &
			IXXAT_USBFD_MSG_FLAGS_TYPE);

		switch (msg_type) {

		case IXXAT_USBFD_CAN_DATA:
			err = ixx_usbfd_handle_canmsg(dev, can_msg);
			if (err < 0)
				goto fail;
			break;

		case IXXAT_USBFD_CAN_STATUS:
			err = ixx_usbfd_handle_status(dev, can_msg);
			if (err < 0)
				goto fail;
			break;

		case IXXAT_USBFD_CAN_ERROR:
			err = ixx_usbfd_handle_error(dev, can_msg);
			if (err < 0)
				goto fail;
			break;

		case IXXAT_USBFD_CAN_TIMEOVR:
			ixxat_usb_get_ts_tv(dev, can_msg->time, NULL);
			break;

		case IXXAT_USBFD_CAN_INFO:
		case IXXAT_USBFD_CAN_WAKEUP:
		case IXXAT_USBFD_CAN_TIMERST:
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

static int ixx_usbfd_encode_msg(struct ixx_usb_device *dev, struct sk_buff *skb,
		u8 *obuf, size_t *size)
{
	struct canfd_frame *cf = (struct canfd_frame *) skb->data;
	struct ixx_can_msg_v2 can_msg = { 0 };

	if (cf->can_id & CAN_RTR_FLAG)
		can_msg.flags |= IXXAT_USBFD_MSG_FLAGS_RTR;

	if (cf->can_id & CAN_EFF_FLAG) {
		can_msg.flags |= IXXAT_USBFD_MSG_FLAGS_EXT;
		can_msg.msg_id = cf->can_id & CAN_EFF_MASK;
	} else {
		can_msg.msg_id = cf->can_id & CAN_SFF_MASK;
	}

	if (dev->can.ctrlmode & CAN_CTRLMODE_ONE_SHOT)
		can_msg.flags |= IXXAT_USBFD_MSG_FLAGS_SSM;

	if (skb->len == CANFD_MTU) {
		can_msg.flags |= IXXAT_USBFD_MSG_FLAGS_EDL;

		if (!(cf->can_id & CAN_RTR_FLAG) && (cf->flags & CANFD_BRS))
			can_msg.flags |= IXXAT_USBFD_MSG_FLAGS_FDR;
	}

	can_msg.flags |= (can_len2dlc(cf->len) << 16) &
		IXXAT_USBFD_MSG_FLAGS_DLC;

	can_msg.flags = cpu_to_le32(can_msg.flags);
	can_msg.msg_id = cpu_to_le32(can_msg.msg_id);

	memcpy(can_msg.data, cf->data, cf->len);
	can_msg.size = (u8)(sizeof(can_msg) - 1 - CANFD_MAX_DLEN + cf->len);

	memcpy(obuf, &can_msg, can_msg.size + 1);

	*size = can_msg.size + 1;

	skb->data_len = *size;

	return 0;
}

static int ixx_usbfd_start(struct ixx_usb_device *dev)
{
	int err;
	u32 time_ref = 0;
	u8 can_opmode = IXXAT_USBFD_OPMODE_EXTENDED
			| IXXAT_USBFD_OPMODE_STANDARD;
	u8 can_exmode = 0;

	if (dev->can.ctrlmode & CAN_CTRLMODE_BERR_REPORTING)
		can_opmode |= IXXAT_USBFD_OPMODE_ERRFRAME;
	if (dev->can.ctrlmode & CAN_CTRLMODE_LISTENONLY)
		can_opmode |= IXXAT_USBFD_OPMODE_LISTONLY;

	if ((CAN_CTRLMODE_FD | CAN_CTRLMODE_FD_NON_ISO) & dev->can.ctrlmode)
		can_exmode |= IXXAT_USBFD_EXMODE_EXTDATA |
			IXXAT_USBFD_EXMODE_FASTDATA;

	if (!(CAN_CTRLMODE_FD_NON_ISO & dev->can.ctrlmode) && can_exmode)
		can_exmode |= IXXAT_USBFD_EXMODE_ISOFD;

	err = ixx_usbfd_init_ctrl(dev, can_opmode,
			can_exmode,
			&dev->can.bittiming,
			&dev->can.data_bittiming);
	if (err)
		return err;

	/* opening first device: */
	if (dev->ctrl_opened_count == 0) {
		err = ixx_usbfd_start_ctrl(dev, &time_ref);
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
static int ixx_usbfd_stop(struct ixx_usb_device *dev)
{
	int err;

	if (dev->ctrl_opened_count == 1) {
		err = ixx_usbfd_stop_ctrl(dev);
		if (err)
			return err;
	}

	dev->ctrl_opened_count--;

	return 0;
}

/*
 * called when probing to initialize a device object.
 */
static int ixx_usbfd_init(struct ixx_usb_device *dev)
{
	dev->restart_task = kthread_run(&ixx_usbfd_restart_task, dev,
			"restart_thread");
	if (!dev->restart_task)
		return -ENOBUFS;

	return 0;
}

static void ixx_usbfd_exit(struct ixx_usb_device *dev)
{
	ixx_usbfd_reset_ctrl(dev);

	dev->must_quit = 1;
	dev->restart_flag = 1;
	wake_up_interruptible(&dev->wait_queue);
	if (dev->restart_task)
		kthread_stop(dev->restart_task);
}

/*
 * probe function for new IXXAT USB-to-CAN FD interface
 */
static int ixx_usbfd_probe(struct usb_interface *intf)
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
		case IXXAT_USBFD_EP_MSGOUT_0:
		case IXXAT_USBFD_EP_MSGOUT_1:
		case IXXAT_USBFD_EP_MSGOUT_2:
		case IXXAT_USBFD_EP_MSGOUT_3:
		case IXXAT_USBFD_EP_MSGOUT_4:
		case IXXAT_USBFD_EP_MSGIN_0:
		case IXXAT_USBFD_EP_MSGIN_1:
		case IXXAT_USBFD_EP_MSGIN_2:
		case IXXAT_USBFD_EP_MSGIN_3:
		case IXXAT_USBFD_EP_MSGIN_4:

			break;
		default:
			return -ENODEV;
		}
	}

	return 0;
}

static int ixx_usbfd_get_dev_caps(struct usb_device *dev,
		struct ixx_dev_caps *dev_caps)
{
	int err = -ENODEV, i;
	u8 data[IXXAT_USBFD_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbfd_dev_caps_req *dev_caps_req;
	struct ixx_usbfd_dev_caps_res *dev_caps_res;
	u32 req_size = sizeof(*dev_caps_req);

	dev_caps_req = (struct ixx_usbfd_dev_caps_req *) data;
	dev_caps_res = (struct ixx_usbfd_dev_caps_res *)(data + req_size);

	dev_caps_req->dal_req.req_size   = cpu_to_le32(req_size);
	dev_caps_req->dal_req.req_code   =
		cpu_to_le32(IXXAT_USBFD_BRD_GET_DEVCAPS_CMD);
	dev_caps_req->dal_req.req_port   = 0xffff;
	dev_caps_req->dal_req.req_socket = 0xffff;

	dev_caps_res->dal_res.res_size = cpu_to_le32(
		sizeof(*dev_caps_res));
	dev_caps_res->dal_res.ret_size = 0;
	dev_caps_res->dal_res.ret_code = 0xffffffff;

	err = ixx_usbfd_send_cmd(dev, &dev_caps_req->dal_req);
	if (err < 0)
		return err;

	err = ixx_usbfd_rcv_cmd(dev, &dev_caps_res->dal_res,
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

static int ixx_usbfd_get_ctrl_caps(struct usb_device *dev,
		struct ixx_ctrl_caps *ctrl_caps, int index)
{
	int err = -ENODEV;
	u8 data[IXXAT_USBFD_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbfd_ctrl_caps_req *ctrl_caps_req;
	struct ixx_usbfd_ctrl_caps_res *ctrl_caps_res;
	u32 req_size = sizeof(*ctrl_caps_req);

	ctrl_caps_req = (struct ixx_usbfd_ctrl_caps_req *) data;
	ctrl_caps_res = (struct ixx_usbfd_ctrl_caps_res *)(data + req_size);

	ctrl_caps_req->dal_req.req_size   = cpu_to_le32(req_size);
	ctrl_caps_req->dal_req.req_code   =
		cpu_to_le32(IXXAT_USBFD_CAN_GET_CAPS_CMD);
	ctrl_caps_req->dal_req.req_port   = cpu_to_le16(index);
	ctrl_caps_req->dal_req.req_socket = 0xffff;

	ctrl_caps_res->dal_res.res_size =
		cpu_to_le32(sizeof(*ctrl_caps_res));
	ctrl_caps_res->dal_res.ret_size = 0;
	ctrl_caps_res->dal_res.ret_code = 0xffffffff;

	err = ixx_usbfd_send_cmd(dev, &ctrl_caps_req->dal_req);
	if (err < 0)
		return err;

	err = ixx_usbfd_rcv_cmd(dev,
			&ctrl_caps_res->dal_res,
			index);
	if (err < 0)
		return err;

	ctrl_caps->bus_coupling =
		le16_to_cpu(ctrl_caps_res->ctrl_caps.bus_coupling);
	ctrl_caps->clock_freq =
		le32_to_cpu(ctrl_caps_res->ctrl_caps.clock_freq);
	ctrl_caps->cms_divisor =
		le32_to_cpu(ctrl_caps_res->ctrl_caps.cms_divisor);
	ctrl_caps->cms_max_ticks =
		le32_to_cpu(ctrl_caps_res->ctrl_caps.cms_max_ticks);
	ctrl_caps->ctrl_type = le16_to_cpu(ctrl_caps_res->ctrl_caps.ctrl_type);
	ctrl_caps->dtx_divisor =
		le32_to_cpu(ctrl_caps_res->ctrl_caps.dtx_divisor);
	ctrl_caps->dtx_max_ticks =
		le32_to_cpu(ctrl_caps_res->ctrl_caps.dtx_max_ticks);
	ctrl_caps->features = le32_to_cpu(ctrl_caps_res->ctrl_caps.features);
	ctrl_caps->tsc_divisor =
		le32_to_cpu(ctrl_caps_res->ctrl_caps.tsc_divisor);

	return 0;
}

static int ixx_usbfd_get_fwinfo(struct ixx_usb_device *dev,
		struct ixx_intf_fw_info *fwinfo)
{
	int err = -ENODEV;
	u8 data[IXXAT_USBFD_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbfd_brd_get_fwinfo_req *fw_info_req;
	struct ixx_usbfd_brd_get_fwinfo_res *fw_info_res;
	u32 req_size = sizeof(*fw_info_req);

	fw_info_req = (struct ixx_usbfd_brd_get_fwinfo_req *) data;
	fw_info_res = (struct ixx_usbfd_brd_get_fwinfo_res *)(data + req_size);

	fw_info_req->dal_req.req_size   = cpu_to_le32(req_size);
	fw_info_req->dal_req.req_code   =
		cpu_to_le32(IXXAT_USBFD_BRD_GET_FWINFO_CMD);
	fw_info_req->dal_req.req_port   = 0xffff;
	fw_info_req->dal_req.req_socket = 0xffff;

	fw_info_res->dal_res.res_size =
		cpu_to_le32(sizeof(*fw_info_res));
	fw_info_res->dal_res.ret_size = 0;
	fw_info_res->dal_res.ret_code = 0xffffffff;

	err = ixx_usbfd_send_cmd(dev->udev, &fw_info_req->dal_req);
	if (err < 0)
		return err;

	err = ixx_usbfd_rcv_cmd(dev->udev,
		&fw_info_res->dal_res, 0xffff);
	if (err < 0)
		return err;

	if (fwinfo) {
		fwinfo->build_version = le16_to_cpu(
			fw_info_res->fwinfo.build_version);
		fwinfo->firmware_type = le32_to_cpu(
			fw_info_res->fwinfo.firmware_type);
		fwinfo->major_version = le16_to_cpu(
			fw_info_res->fwinfo.major_version);
		fwinfo->minor_version = le16_to_cpu(
			fw_info_res->fwinfo.minor_version);
		fwinfo->reserved = le16_to_cpu(fw_info_res->fwinfo.reserved);
	}

	return le32_to_cpu(fw_info_res->dal_res.ret_code);
}

static int ixx_usbfd_get_dev_info(struct ixx_usb_device *dev,
		struct ixx_intf_info *dev_info)
{
	int err = -ENODEV;
	u8 data[IXXAT_USBFD_CMD_BUFFER_SIZE] = { 0 };
	struct ixx_usbfd_brd_get_intf_info_req *dev_info_req;
	struct ixx_usbfd_brd_get_intf_info_res *dev_info_res;
	u32 req_size = sizeof(*dev_info_req);

	dev_info_req = (struct ixx_usbfd_brd_get_intf_info_req *) data;
	dev_info_res =
		(struct ixx_usbfd_brd_get_intf_info_res *)(data + req_size);

	dev_info_req->dal_req.req_size = cpu_to_le32(req_size);
	dev_info_req->dal_req.req_code =
		cpu_to_le32(IXXAT_USBFD_BRD_GET_DEVINFO_CMD);
	dev_info_req->dal_req.req_port = 0xffff;
	dev_info_req->dal_req.req_socket = 0xffff;

	dev_info_res->dal_res.res_size =
		cpu_to_le32(sizeof(*dev_info_res));
	dev_info_res->dal_res.ret_size = 0;
	dev_info_res->dal_res.ret_code = 0xffffffff;

	err = ixx_usbfd_send_cmd(dev->udev,
			&dev_info_req->dal_req);
	if (err < 0)
		return err;

	err = ixx_usbfd_rcv_cmd(dev->udev,
			&dev_info_res->dal_res,
			0xffff);
	if (err < 0)
		return err;

	if (dev_info) {
		memcpy(dev_info->device_id, &dev_info_res->info.device_id,
			sizeof(dev_info_res->info.device_id));
		memcpy(dev_info->device_name, &dev_info_res->info.device_name,
			sizeof(dev_info_res->info.device_name));
		dev_info->device_fpga_version = le16_to_cpu(
			dev_info_res->info.device_fpga_version);
		dev_info->device_version = le32_to_cpu(
			dev_info_res->info.device_version);
	}

	return le32_to_cpu(dev_info_res->dal_res.ret_code);
}

/*
 * describes the USB-to-CAN FD automotive adapter
 */
struct ixx_usb_adapter usb_to_can_fd_automotive = {
	.name = "USB-to-CAN FD automotive",
	.device_id = USB_TO_CAN_FD_AUTOMOTIVE_PRODUCT_ID,
	.clock = {
		.freq = IFIFD_CRYSTAL_HZ,
	},

	.bittiming_const = {
		.name = "ifi_can",
		.tseg1_min = 1,
		.tseg1_max = 256,
		.tseg2_min = 1,
		.tseg2_max = 256,
		.sjw_max = 128,
		.brp_min = 2,
		.brp_max = 513,
		.brp_inc = 1, },

	.data_bittiming_const = {
		.name = "ifi_can",
		.tseg1_min = 1,
		.tseg1_max = 256,
		.tseg2_min = 1,
		.tseg2_max = 256,
		.sjw_max = 128,
		.brp_min = 2,
		.brp_max = 513,
		.brp_inc = 1, },

	.ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
		CAN_CTRLMODE_LISTENONLY |
		CAN_CTRLMODE_LOOPBACK |
		CAN_CTRLMODE_BERR_REPORTING |
		CAN_CTRLMODE_FD |
		CAN_CTRLMODE_FD_NON_ISO,

	/* size of device private data */
	.sizeof_dev_private = sizeof(const struct ixx_usb_device),

	/* give here messages in/out endpoints */
	.ep_msg_in = {  IXXAT_USBFD_EP_MSGIN_0, IXXAT_USBFD_EP_MSGIN_1,
			IXXAT_USBFD_EP_MSGIN_2, IXXAT_USBFD_EP_MSGIN_3,
			IXXAT_USBFD_EP_MSGIN_4 },
	.ep_msg_out = { IXXAT_USBFD_EP_MSGOUT_0, IXXAT_USBFD_EP_MSGOUT_1,
			IXXAT_USBFD_EP_MSGOUT_2, IXXAT_USBFD_EP_MSGOUT_3,
			IXXAT_USBFD_EP_MSGOUT_4 },

	/* size of rx/tx usb buffers */
	.rx_buffer_size = IXXAT_USBFD_RX_BUFFER_SIZE,
	.tx_buffer_size = IXXAT_USBFD_TX_BUFFER_SIZE,

	/* device callbacks */
	.intf_probe = ixx_usbfd_probe,
	.dev_get_dev_caps = ixx_usbfd_get_dev_caps,
	.dev_get_ctrl_caps = ixx_usbfd_get_ctrl_caps,
	.dev_init = ixx_usbfd_init,
	.dev_exit = ixx_usbfd_exit,
	.intf_get_info = ixx_usbfd_get_dev_info,
	.intf_get_fw_info = ixx_usbfd_get_fwinfo,
	.dev_decode_buf = ixx_usbfd_decode_buf,
	.dev_encode_msg = ixx_usbfd_encode_msg,
	.dev_start = ixx_usbfd_start,
	.dev_stop = ixx_usbfd_stop,
	.dev_power = ixx_usbfd_power_ctrl,
	.has_bgi_ep = 1,
};

/*
 * describes the USB-to-CAN FD compact adapter
 */
struct ixx_usb_adapter usb_to_can_fd_compact = {
	.name = "USB-to-CAN FD compact",
	.device_id = USB_TO_CAN_FD_COMPACT_PRODUCT_ID,
	.clock = {
		.freq = IFIFD_CRYSTAL_HZ,
	},

	.bittiming_const = {
		.name = "ifi_can",
		.tseg1_min = 1,
		.tseg1_max = 256,
		.tseg2_min = 1,
		.tseg2_max = 256,
		.sjw_max = 128,
		.brp_min = 2,
		.brp_max = 513,
		.brp_inc = 1, },

	.data_bittiming_const = {
		.name = "ifi_can",
		.tseg1_min = 1,
		.tseg1_max = 256,
		.tseg2_min = 1,
		.tseg2_max = 256,
		.sjw_max = 128,
		.brp_min = 2,
		.brp_max = 513,
		.brp_inc = 1, },

	.ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
		CAN_CTRLMODE_LISTENONLY |
		CAN_CTRLMODE_LOOPBACK |
		CAN_CTRLMODE_BERR_REPORTING |
		CAN_CTRLMODE_FD |
		CAN_CTRLMODE_FD_NON_ISO,

	/* size of device private data */
	.sizeof_dev_private = sizeof(const struct ixx_usb_device),

	/* give here messages in/out endpoints */
	.ep_msg_in = {  IXXAT_USBFD_EP_MSGIN_0, IXXAT_USBFD_EP_MSGIN_1,
			IXXAT_USBFD_EP_MSGIN_2, IXXAT_USBFD_EP_MSGIN_3,
			IXXAT_USBFD_EP_MSGIN_4 },
	.ep_msg_out = { IXXAT_USBFD_EP_MSGOUT_0, IXXAT_USBFD_EP_MSGOUT_1,
			IXXAT_USBFD_EP_MSGOUT_2, IXXAT_USBFD_EP_MSGOUT_3,
			IXXAT_USBFD_EP_MSGOUT_4 },

	/* size of rx/tx usb buffers */
	.rx_buffer_size = IXXAT_USBFD_RX_BUFFER_SIZE,
	.tx_buffer_size = IXXAT_USBFD_TX_BUFFER_SIZE,

	/* device callbacks */
	.intf_probe = ixx_usbfd_probe,
	.dev_get_dev_caps = ixx_usbfd_get_dev_caps,
	.dev_get_ctrl_caps = ixx_usbfd_get_ctrl_caps,
	.dev_init = ixx_usbfd_init,
	.dev_exit = ixx_usbfd_exit,
	.intf_get_info = ixx_usbfd_get_dev_info,
	.intf_get_fw_info = ixx_usbfd_get_fwinfo,
	.dev_decode_buf = ixx_usbfd_decode_buf,
	.dev_encode_msg = ixx_usbfd_encode_msg,
	.dev_start = ixx_usbfd_start,
	.dev_stop = ixx_usbfd_stop,
	.dev_power = ixx_usbfd_power_ctrl,
	.has_bgi_ep = 1,
};


/*
 * describes the USB-to-CAN FD professional adapter
 */
struct ixx_usb_adapter usb_to_can_fd_professional = {
	.name = "USB-to-CAN FD professional",
	.device_id = USB_TO_CAN_FD_PROFESSIONAL_PRODUCT_ID,
	.clock = {
		.freq = IFIFD_CRYSTAL_HZ,
	},

	.bittiming_const = {
		.name = "ifi_can",
		.tseg1_min = 1,
		.tseg1_max = 256,
		.tseg2_min = 1,
		.tseg2_max = 256,
		.sjw_max = 128,
		.brp_min = 2,
		.brp_max = 513,
		.brp_inc = 1, },

	.data_bittiming_const = {
		.name = "ifi_can",
		.tseg1_min = 1,
		.tseg1_max = 256,
		.tseg2_min = 1,
		.tseg2_max = 256,
		.sjw_max = 128,
		.brp_min = 2,
		.brp_max = 513,
		.brp_inc = 1, },

	.ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
		CAN_CTRLMODE_LISTENONLY |
		CAN_CTRLMODE_LOOPBACK |
		CAN_CTRLMODE_BERR_REPORTING |
		CAN_CTRLMODE_FD |
		CAN_CTRLMODE_FD_NON_ISO,

	/* size of device private data */
	.sizeof_dev_private = sizeof(const struct ixx_usb_device),

	/* give here messages in/out endpoints */
	.ep_msg_in = {  IXXAT_USBFD_EP_MSGIN_0, IXXAT_USBFD_EP_MSGIN_1,
			IXXAT_USBFD_EP_MSGIN_2, IXXAT_USBFD_EP_MSGIN_3,
			IXXAT_USBFD_EP_MSGIN_4 },
	.ep_msg_out = { IXXAT_USBFD_EP_MSGOUT_0, IXXAT_USBFD_EP_MSGOUT_1,
			IXXAT_USBFD_EP_MSGOUT_2, IXXAT_USBFD_EP_MSGOUT_3,
			IXXAT_USBFD_EP_MSGOUT_4 },

	/* size of rx/tx usb buffers */
	.rx_buffer_size = IXXAT_USBFD_RX_BUFFER_SIZE,
	.tx_buffer_size = IXXAT_USBFD_TX_BUFFER_SIZE,

	/* device callbacks */
	.intf_probe = ixx_usbfd_probe,
	.dev_get_dev_caps = ixx_usbfd_get_dev_caps,
	.dev_get_ctrl_caps = ixx_usbfd_get_ctrl_caps,
	.dev_init = ixx_usbfd_init,
	.dev_exit = ixx_usbfd_exit,
	.intf_get_info = ixx_usbfd_get_dev_info,
	.intf_get_fw_info = ixx_usbfd_get_fwinfo,
	.dev_decode_buf = ixx_usbfd_decode_buf,
	.dev_encode_msg = ixx_usbfd_encode_msg,
	.dev_start = ixx_usbfd_start,
	.dev_stop = ixx_usbfd_stop,
	.dev_power = ixx_usbfd_power_ctrl,
	.has_bgi_ep = 1,
};

/*
 * describes the USB-to-CAN FD PCIe mini adapter
 */
struct ixx_usb_adapter usb_to_can_fd_pcie_mini = {
	.name = "USB-to-CAN FD PCIe mini",
	.device_id = USB_TO_CAN_FD_PCIE_MINI_PRODUCT_ID,
	.clock = {
		.freq = IFIFD_CRYSTAL_HZ,
	},

	.bittiming_const = {
		.name = "ifi_can",
		.tseg1_min = 1,
		.tseg1_max = 256,
		.tseg2_min = 1,
		.tseg2_max = 256,
		.sjw_max = 128,
		.brp_min = 2,
		.brp_max = 513,
		.brp_inc = 1, },

	.data_bittiming_const = {
		.name = "ifi_can",
		.tseg1_min = 1,
		.tseg1_max = 256,
		.tseg2_min = 1,
		.tseg2_max = 256,
		.sjw_max = 128,
		.brp_min = 2,
		.brp_max = 513,
		.brp_inc = 1, },

	.ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
		CAN_CTRLMODE_LISTENONLY |
		CAN_CTRLMODE_LOOPBACK |
		CAN_CTRLMODE_BERR_REPORTING |
		CAN_CTRLMODE_FD |
		CAN_CTRLMODE_FD_NON_ISO,

	/* size of device private data */
	.sizeof_dev_private = sizeof(const struct ixx_usb_device),

	/* give here messages in/out endpoints */
	.ep_msg_in = {  IXXAT_USBFD_EP_MSGIN_0, IXXAT_USBFD_EP_MSGIN_1,
			IXXAT_USBFD_EP_MSGIN_2, IXXAT_USBFD_EP_MSGIN_3,
			IXXAT_USBFD_EP_MSGIN_4 },
	.ep_msg_out = { IXXAT_USBFD_EP_MSGOUT_0, IXXAT_USBFD_EP_MSGOUT_1,
			IXXAT_USBFD_EP_MSGOUT_2, IXXAT_USBFD_EP_MSGOUT_3,
			IXXAT_USBFD_EP_MSGOUT_4 },

	/* size of rx/tx usb buffers */
	.rx_buffer_size = IXXAT_USBFD_RX_BUFFER_SIZE,
	.tx_buffer_size = IXXAT_USBFD_TX_BUFFER_SIZE,

	/* device callbacks */
	.intf_probe = ixx_usbfd_probe,
	.dev_get_dev_caps = ixx_usbfd_get_dev_caps,
	.dev_get_ctrl_caps = ixx_usbfd_get_ctrl_caps,
	.dev_init = ixx_usbfd_init,
	.dev_exit = ixx_usbfd_exit,
	.intf_get_info = ixx_usbfd_get_dev_info,
	.intf_get_fw_info = ixx_usbfd_get_fwinfo,
	.dev_decode_buf = ixx_usbfd_decode_buf,
	.dev_encode_msg = ixx_usbfd_encode_msg,
	.dev_start = ixx_usbfd_start,
	.dev_stop = ixx_usbfd_stop,
	.dev_power = ixx_usbfd_power_ctrl,
	.has_bgi_ep = 1,
};

/*
 * describes the USB-to-CAR adapter
 */
struct ixx_usb_adapter usb_to_car = {
	.name = "USB-to-CAR",
	.device_id = USB_TO_CAR_ID,
	.clock = {
		.freq = IFIFD_CRYSTAL_HZ,
	},

	.bittiming_const = {
		.name = "ifi_can",
		.tseg1_min = 1,
		.tseg1_max = 256,
		.tseg2_min = 1,
		.tseg2_max = 256,
		.sjw_max = 128,
		.brp_min = 2,
		.brp_max = 513,
		.brp_inc = 1, },

	.data_bittiming_const = {
		.name = "ifi_can",
		.tseg1_min = 1,
		.tseg1_max = 256,
		.tseg2_min = 1,
		.tseg2_max = 256,
		.sjw_max = 128,
		.brp_min = 2,
		.brp_max = 513,
		.brp_inc = 1, },

	.ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
		CAN_CTRLMODE_LISTENONLY |
		CAN_CTRLMODE_LOOPBACK |
		CAN_CTRLMODE_BERR_REPORTING |
		CAN_CTRLMODE_FD |
		CAN_CTRLMODE_FD_NON_ISO,

	/* size of device private data */
	.sizeof_dev_private = sizeof(const struct ixx_usb_device),

	/* give here messages in/out endpoints */
	.ep_msg_in = {  IXXAT_USBFD_EP_MSGIN_0, IXXAT_USBFD_EP_MSGIN_1,
			IXXAT_USBFD_EP_MSGIN_2, IXXAT_USBFD_EP_MSGIN_3,
			IXXAT_USBFD_EP_MSGIN_4 },
	.ep_msg_out = { IXXAT_USBFD_EP_MSGOUT_0, IXXAT_USBFD_EP_MSGOUT_1,
			IXXAT_USBFD_EP_MSGOUT_2, IXXAT_USBFD_EP_MSGOUT_3,
			IXXAT_USBFD_EP_MSGOUT_4 },

	/* size of rx/tx usb buffers */
	.rx_buffer_size = IXXAT_USBFD_RX_BUFFER_SIZE,
	.tx_buffer_size = IXXAT_USBFD_TX_BUFFER_SIZE,

	/* device callbacks */
	.intf_probe = ixx_usbfd_probe,
	.dev_get_dev_caps = ixx_usbfd_get_dev_caps,
	.dev_get_ctrl_caps = ixx_usbfd_get_ctrl_caps,
	.dev_init = ixx_usbfd_init,
	.dev_exit = ixx_usbfd_exit,
	.intf_get_info = ixx_usbfd_get_dev_info,
	.intf_get_fw_info = ixx_usbfd_get_fwinfo,
	.dev_decode_buf = ixx_usbfd_decode_buf,
	.dev_encode_msg = ixx_usbfd_encode_msg,
	.dev_start = ixx_usbfd_start,
	.dev_stop = ixx_usbfd_stop,
	.dev_power = ixx_usbfd_power_ctrl,
	.has_bgi_ep = 1,
};


#endif // CANFD_CAPABLE
