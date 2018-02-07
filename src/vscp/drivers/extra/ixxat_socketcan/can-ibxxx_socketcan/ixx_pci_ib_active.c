/*
 * CAN driver for IXXAT CAN-IB2x0, CAN-IB4x0, CAN-IB6x0 and CAN-IB8x0 adapters
 *
 * Copyright (C) 2003-2014 Michael Hengler IXXAT Automation GmbH
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
#include <linux/pci.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/can.h>
#include <linux/can/dev.h>
#include <linux/can/error.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/gfp.h>
#include <linux/types.h>
#include <linux/can/netlink.h>
#include <linux/firmware.h>
#include <linux/time.h>

#include <asm-generic/kmap_types.h>
#include <asm-generic/errno.h>

#include <stdarg.h>
#include "ifi.h"
#include "ififd.h"
#include "ixx_pci_core.h"

MODULE_SUPPORTED_DEVICE("IXXAT Automation GmbH CAN-IB2x0, CAN-IB4x0, CAN-IB6x0 and CAN-IB8x0 interfaces");

#define IXX_FIRMWARE "ixx-can-ib-1.9.3.fw"

#define IXX_IFI_FIFOCMD    0x00
#define IXX_IFI_TXSUSPEND  0x01
#define IXX_IFI_TXREPCOUNT 0x02
#define IXX_IFI_DLC        0x03
#define IXX_IFI_ID         0x04
#define IXX_IFI_DATA       0x05

#define IXX_IFI_RXSTATISTICS 0x01
#define IXX_IFI_RXTIMESTAMP  0x02

#define IFI_HZ 80000000

#define CAN_IB2X0_VER_SIZE 0x10

#define CAN_IB2X0_MSG_TYPE_VCI 0x00
#define CAN_IB2X0_MSG_TYPE_IFI 0x01

/* Size of DMA Buffer 512kiB required DMA page size 4kiB => Add one page */
#define CAN_IB2X0_DMA_LEN    (512*1024 + 4*1024);
#define CAN_IB2X0_ADDMEM_LEN PAGE_SIZE
/* size of available mapped DMA (after possible page shifting) */
#define CAN_IB200_DMA_SIZE   0x80000

/* software reset */
#define CAN_IB2X0_PCIE_RESET        0x00000001
/* software reset delay = 10000 us */
#define CAN_IB2X0_PCIE_RESET_PERIOD 10000
/* software reset delay = 100000 us */
#define CAN_IB2X0_PCIE_RESET_DELAY  100000

#define CAN_IB2X0_PCIE_ALTERA_LCR_RESET_CARD 0x1000
#define CAN_IB2X0_PCIE_ALTERA_LCR_RESET_CARD_OFF \
                (CAN_IB2X0_PCIE_ALTERA_LCR_RESET_CARD<< 2)

#define CAN_IB2X0_PCIE_ALTERA_LCR_A2P_MAILBOXRO 0x0900

/* Address translation Table is from address 0x1000 - 0x2000 */
#define CAN_IB2X0_PCIE_ALTERA_LCR_ADRTRANSSTART       0x400
#define CAN_IB2X0_PCIE_ALTERA_LCR_ADRTRANSSTART_OFF   \
                (CAN_IB2X0_PCIE_ALTERA_LCR_ADRTRANSSTART<< 2)

#define CAN_IB2X0_PCIE_ALTERA_LCR_ADRTRANSEND         0x800
#define CAN_IB2X0_PCIE_ALTERA_LCR_ADRTRANSEND_OFF \
                (CAN_IB2X0_PCIE_ALTERA_LCR_ADRTRANSEND<< 2)

#define CAN_IB2X0_PCIE_ALTERA_LCR_P2A_MAILBOXRW     0x200
#define CAN_IB2X0_PCIE_ALTERA_LCR_P2A_MAILBOXRW_OFF \
                (CAN_IB2X0_PCIE_ALTERA_LCR_P2A_MAILBOXRW<< 2)
#define CAN_IB2X0_PCIE_ALTERA_LCR_P2A_MAILBOX_NO    8

/* boot manager command max size in [BYTE] */
#define CAN_IB2X0_BMG_PCR_CMD_MAX_SIZE 240

/* reception of 11-bit id messages */
#define CAN_IB2X0_OPMODE_STANDARD 0x01
/* reception of 29-bit id messages */
#define CAN_IB2X0_OPMODE_EXTENDED 0x02
/* enable reception of error frames */
#define CAN_IB2X0_OPMODE_ERRFRAME 0x04
/* listen only mode (TX passive) */
#define CAN_IB2X0_OPMODE_LISTONLY 0x08

/* no extended operation */
#define CAN_IB2X0_EXMODE_DISABLED  0x00
/* extended data length */
#define CAN_IB2X0_EXMODE_EXTDATA   0x01
/* fast data bit rate */
#define CAN_IB2X0_EXMODE_FASTDATA  0x02
/* ISO-CANFD */
#define CAN_IB2X0_EXMODE_ISOFD     0x04

/* Stuff error */
#define CAN_IB2X0_CAN_ERROR_STUFF  1
/* Form error */
#define CAN_IB2X0_CAN_ERROR_FORM   2
/* Acknowledgment error */
#define CAN_IB2X0_CAN_ERROR_ACK    3
/* Bit error */
#define CAN_IB2X0_CAN_ERROR_BIT    4
/* CRC error */
#define CAN_IB2X0_CAN_ERROR_CRC    6
/* Other (unspecified) error */
#define CAN_IB2X0_CAN_ERROR_OTHER  7

/* Data overrun occurred */
#define CAN_IB2X0_CAN_STATUS_OVRRUN 0x02
/* Error warning limit exceeded */
#define CAN_IB2X0_CAN_STATUS_ERRLIM 0x04
/* Bus off status */
#define CAN_IB2X0_CAN_STATUS_BUSOFF 0x08

#define CAN_IB2X0_CAN_INFO   0x01
#define CAN_IB2X0_CAN_ERROR  0x02
#define CAN_IB2X0_CAN_STATUS 0x03

#define CAN_IB2X0_MSG_FLAGS_TYPE 0x000000FF

#define CAN_IB2X0_BAL_CMD_CLASS 3
#define CAN_IB2X0_BRD_CMD_CLASS 4
#define CAN_IB2X0_BMG_CMD_CLASS 5

#define CAN_IB2X0_BRD_CMD_CAT 0
#define CAN_IB2X0_BMG_CMD_CAT 0

#define CAN_IB2X0_CMD_CAT 1

#define CAN_IB2X0_VCI_CMD_CODE(Class,Function) \
                (u32) ( ((Class) << 8) | (Function) )

#define CAN_IB2X0_BRD_CMD_CODE(Category,Function) \
                CAN_IB2X0_VCI_CMD_CODE(CAN_IB2X0_BRD_CMD_CLASS, \
                                ((Category) << 5) | (Function) )

#define CAN_IB2X0_BAL_CMD_CODE(Category,Function) \
                CAN_IB2X0_VCI_CMD_CODE(CAN_IB2X0_BAL_CMD_CLASS, \
                                ((Category) << 5) | (Function) )

#define CAN_IB2X0_BMG_CMD_CODE(Category,Function) \
                CAN_IB2X0_VCI_CMD_CODE(CAN_IB2X0_BMG_CMD_CLASS, \
                                ((Category) << 5) | (Function) )

#define CAN_IB2X0_BRD_GET_DEVCAPS_CMD \
                CAN_IB2X0_BRD_CMD_CODE(CAN_IB2X0_BRD_CMD_CAT, 1)

#define CAN_IB2X0_INIT_CMD \
                CAN_IB2X0_BAL_CMD_CODE(CAN_IB2X0_CMD_CAT, 5)
#define CAN_IB2X0_INIT_V2_CMD \
                CAN_IB2X0_BAL_CMD_CODE(CAN_IB2X0_CMD_CAT, 23)
#define CAN_IB2X0_START_CMD \
                CAN_IB2X0_BAL_CMD_CODE(CAN_IB2X0_CMD_CAT, 6)
#define CAN_IB2X0_STOP_CMD \
                CAN_IB2X0_BAL_CMD_CODE(CAN_IB2X0_CMD_CAT, 7)

/* host to device */
#define CAN_IB2X0_PCR_RESDIR_HTOD   0x0001
/* device to host */
#define CAN_IB2X0_PCR_RESDIR_DTOH   0x0002

#define CAN_IB2X0_PCR_RES_VER_OFF   0x10

#define CAN_IB2X0_PCR_RES_TAG       0x00
#define CAN_IB2X0_PCR_RES_SIZE      0x01
#define CAN_IB2X0_PCR_RES_DIR       0x02
#define CAN_IB2X0_PCR_RES_NUM_OBJ   0x03
#define CAN_IB2X0_PCR_RES_OBJ_SIZE  0x04
#define CAN_IB2X0_PCR_RES_WRITE_IDX 0x05
#define CAN_IB2X0_PCR_RES_READ_IDX  0x06
#define CAN_IB2X0_PCR_RES_DATA      0x07

#define CAN_IB2X0_PCR_RES_MAX_OBJ_SIZE 252
#define CAN_IB2X0_PCR_RES_HEADER_SIZE   28

#define CAN_IB2X0_BRD_GET_DEVINFO_CMD \
                CAN_IB2X0_BRD_CMD_CODE(CAN_IB2X0_BRD_CMD_CAT, 2)

#define CAN_IB2X0_BMG_LOOPBACK_CMD \
                CAN_IB2X0_BMG_CMD_CODE(CAN_IB2X0_BMG_CMD_CAT, 0)
#define CAN_IB2X0_BMG_STARTFIRMWARE_CMD \
                CAN_IB2X0_BMG_CMD_CODE(CAN_IB2X0_BMG_CMD_CAT, 1)
#define CAN_IB2X0_BMG_TRIGGER_INT_CMD \
                CAN_IB2X0_BMG_CMD_CODE(CAN_IB2X0_BMG_CMD_CAT, 2)
#define CAN_IB2X0_BMG_READ_BLOCK_CMD \
                CAN_IB2X0_BMG_CMD_CODE(CAN_IB2X0_BMG_CMD_CAT, 3)
#define CAN_IB2X0_BMG_WRITE_BLOCK_CMD \
                CAN_IB2X0_BMG_CMD_CODE(CAN_IB2X0_BMG_CMD_CAT, 4)

#define CAN_IB2X0_CMD_TIMEOUT_US    500000

/* request packet header  */
struct ixx_dal_req {
        u32 req_size;
        u16 req_port;
        u16 req_socket;
        u32 req_code;
}__packed;

/* response packet header */
struct ixx_dal_res {
        u32 res_size;
        u32 ret_size;
        u32 ret_code;
}__packed;

struct ixx_bmg_loopback_cmd_req {
        struct ixx_dal_req dal_req;
        u8 data[1]; /* buffer for data UINT8s */
};

struct ixx_bmg_loopback_cmd_res {
        struct ixx_dal_res dal_res;
        u8 data[1]; /* buffer for data UINT8s */
};

struct ixx_bmg_write_block_req {
        struct ixx_dal_req dal_req;
        u32 addr; /* address */
        u8 data[1]; /* data to be written */
};

struct ixx_bmg_write_block_res {
        struct ixx_dal_res dal_res;
};

struct ixx_bmg_read_block_req {
        struct ixx_dal_req dal_req;
        u32 addr; /* address */
};

struct ixx_bmg_read_block_res {
        struct ixx_dal_res dal_res;
        u8 data[1]; /* buffer for data bytes */
};

struct ixx_bmg_start_firmware_req {
        struct ixx_dal_req dal_req;
        u32 dummy; /* unused variable */
};

struct ixx_bmg_start_firmware_res {
        struct ixx_dal_res dal_res;
};

struct ixx_bmg_trigger_int_req {
        struct ixx_dal_req dal_req;
        u32 mailbox;
        u32 content;
};

struct ixx_bmg_trigger_int_res {
        struct ixx_dal_res dal_res;
};

struct ixx_intf_caps_req {
        struct ixx_dal_req dal_req;
}__packed;

struct ixx_intf_caps_res {
        struct ixx_dal_res dal_res;
        struct ixx_intf_caps intf_caps;
}__packed;

struct ixx_ctrl_init_req {
        struct ixx_dal_req dal_req;
        u8 mode; /* operation mode */
        u8 exmode; /* extended operation mode used for fd settings */
        struct ixx_can_btp bittiming; /* arbitration bittiming */
        struct ixx_can_btp data_bittiming; /* fast data bittiming */
}__packed;

struct ixx_ctrl_init_res {
        struct ixx_dal_res dal_res;
}__packed;

struct ixx_ctrl_start_req {
        struct ixx_dal_req dal_req;
}__packed;

struct ixx_ctrl_start_res {
        struct ixx_dal_res dal_res;
        u32 start_time; /* can controller start time*/
}__packed;

struct ixx_ctrl_stop_req {
        struct ixx_dal_req dal_req;
        u32 action; /* stop action */
}__packed;

struct ixx_ctrl_stop_res {
        struct ixx_dal_res dal_res;
}__packed;

struct ixx_brd_get_intf_info_req {
        struct ixx_dal_req dal_req;
}__packed;

struct ixx_brd_get_intf_info_res {
        struct ixx_dal_res dal_res;
        struct ixx_intf_info info; /* device capabilities */
}__packed;

static int ixx_act_ib_xxx_send_cmd(struct ixx_pci_interface *intf, u32 *tx_fifo,
                struct ixx_dal_req *dal_req, struct ixx_dal_res *dal_res)
{
        u32 size = dal_req->req_size + sizeof(struct ixx_dal_res);
        u32 write_index, read_index, obj_size;
        u8 data[CAN_IB2X0_PCR_RES_MAX_OBJ_SIZE] = { 0 };
        void* fifo;

        memcpy(data, dal_req, dal_req->req_size);

        if (tx_fifo[CAN_IB2X0_PCR_RES_WRITE_IDX]
                        == tx_fifo[CAN_IB2X0_PCR_RES_READ_IDX])
                return -ENOBUFS;

        if (tx_fifo[CAN_IB2X0_PCR_RES_DIR] != CAN_IB2X0_PCR_RESDIR_HTOD)
                return -EBADSLT;

        write_index = tx_fifo[CAN_IB2X0_PCR_RES_WRITE_IDX];
        read_index = tx_fifo[CAN_IB2X0_PCR_RES_READ_IDX];
        obj_size = tx_fifo[CAN_IB2X0_PCR_RES_OBJ_SIZE];

        memcpy(&data[dal_req->req_size], dal_res, sizeof(struct ixx_dal_res));

        fifo = ((u8*) (tx_fifo + CAN_IB2X0_PCR_RES_DATA))
                        + (write_index * obj_size);
        memcpy(fifo, &size, sizeof(u32));

        fifo = ((u8*) (tx_fifo + CAN_IB2X0_PCR_RES_DATA))
                        + (write_index * obj_size) + sizeof(u32);
        memcpy(fifo, data, size);

        tx_fifo[CAN_IB2X0_PCR_RES_WRITE_IDX] = (write_index + 1)
                        % tx_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ];

        return 0;
}

static int ixx_act_ib_xxx_rcv_cmd(struct ixx_pci_interface *intf, u32 *rx_fifo,
                struct ixx_dal_req *dal_req, struct ixx_dal_res *dal_res)
{
        int err = -ENODATA;
        u8* data;
        u32* res_size;
        struct ixx_dal_req *res_dal_req;
        u32 read_index, write_index, data_offset;
        struct timeval start, end;
        size_t dal_size = sizeof(struct ixx_dal_req);

        write_index = rx_fifo[CAN_IB2X0_PCR_RES_WRITE_IDX];
        read_index = rx_fifo[CAN_IB2X0_PCR_RES_READ_IDX];

        if (rx_fifo[CAN_IB2X0_PCR_RES_DIR] != CAN_IB2X0_PCR_RESDIR_DTOH)
                return -EBADSLT;

        do_gettimeofday(&start);
        end = start;

        while ((end.tv_usec - start.tv_usec) < CAN_IB2X0_CMD_TIMEOUT_US) {
                if (++read_index == rx_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ])
                        read_index = 0;

                if (write_index == rx_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ])
                        write_index = 0;

                // sleep for about 10 µsec and wait for data from the interface card
                if (read_index == write_index) {
                        read_index = rx_fifo[CAN_IB2X0_PCR_RES_READ_IDX];
                        write_index = rx_fifo[CAN_IB2X0_PCR_RES_WRITE_IDX];
                        usleep_range(9, 10);
                        goto cmd_continue;
                }

                data_offset = read_index * rx_fifo[CAN_IB2X0_PCR_RES_OBJ_SIZE];
                data = ((u8*) &rx_fifo[CAN_IB2X0_PCR_RES_DATA]) + data_offset;
                rx_fifo[CAN_IB2X0_PCR_RES_READ_IDX] = read_index;

                res_size = (u32*) data;

                if (*res_size != (dal_res->res_size + dal_size)) {
                        dev_err(&intf->pdev->dev,
                                        "received invalid sizes! returned size %d expected size %d",
                                        *res_size, dal_res->res_size);
                        goto cmd_continue;
                }

                res_dal_req = (struct ixx_dal_req *) (data + sizeof(u32));

                if (res_dal_req->req_code != dal_req->req_code) {
                        dev_err(&intf->pdev->dev,
                                        "received invalid cmd code! expected cmd code %x received cmd code %x",
                                        res_dal_req->req_code,
                                        dal_req->req_code);
                        goto cmd_continue;
                }

                if (res_dal_req->req_port != dal_req->req_port) {
                        dev_err(&intf->pdev->dev,
                                        "received invalid port index! expected port index %x received port index %x",
                                        res_dal_req->req_port,
                                        dal_req->req_port);
                        goto cmd_continue;
                }

                memcpy(dal_res, data + sizeof(u32) + dal_size,
                                dal_res->res_size);

                if (dal_res->ret_code)
                        dev_err(&intf->pdev->dev,
                                        "receiving command failure: %c",
                                        dal_res->ret_code);

                return dal_res->ret_code;

                cmd_continue: do_gettimeofday(&end);
        }

        return err;
}

static int ixx_act_ib_xxx_get_intf_caps(struct pci_dev *pdev,
                struct ixx_pci_interface * intf,
                struct ixx_intf_caps* intf_caps, u8 abs_idx)
{
        int err = -ENODEV, i;
        u8* fifo;
        u32 __iomem *altera_mailbox;
        u32 *tx_cmd_fifo, *rx_cmd_fifo;
        struct ixx_intf_caps_req intf_caps_req;
        struct ixx_intf_caps_res intf_caps_res;

        tx_cmd_fifo = intf->memvadd + CAN_IB2X0_PCR_RES_VER_OFF;

        intf_caps_req.dal_req.req_size = cpu_to_le32(
                        sizeof(struct ixx_intf_caps_req));
        intf_caps_req.dal_req.req_code = cpu_to_le32(
        CAN_IB2X0_BRD_GET_DEVCAPS_CMD);
        intf_caps_req.dal_req.req_port = 0xffff;
        intf_caps_req.dal_req.req_socket = 0xffff;

        intf_caps_res.dal_res.res_size = cpu_to_le32(
                        sizeof(struct ixx_intf_caps_res));
        intf_caps_res.dal_res.ret_size = 0;
        intf_caps_res.dal_res.ret_code = 0xffffffff;

        mutex_lock_interruptible(&intf->cmd_lock);
        err = ixx_act_ib_xxx_send_cmd(intf, tx_cmd_fifo, &intf_caps_req.dal_req,
                        &intf_caps_res.dal_res);
        if (err)
                goto error;

        altera_mailbox = (u32*) (intf->reg1vadd
                        + CAN_IB2X0_PCIE_ALTERA_LCR_P2A_MAILBOXRW_OFF);
        iowrite32(0xffffffff, altera_mailbox);

        fifo = ((u8*) tx_cmd_fifo);
        fifo += CAN_IB2X0_PCR_RES_HEADER_SIZE;
        fifo += (tx_cmd_fifo[CAN_IB2X0_PCR_RES_OBJ_SIZE]
                        * tx_cmd_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ]);
        rx_cmd_fifo = (u32*) fifo;
        err = ixx_act_ib_xxx_rcv_cmd(intf, rx_cmd_fifo, &intf_caps_req.dal_req,
                        &intf_caps_res.dal_res);
        if (err)
                goto error;

        memcpy(intf_caps, &intf_caps_res.intf_caps,
                        sizeof(struct ixx_intf_caps));

        intf_caps->bus_ctrl_count = le16_to_cpu(
                        intf_caps_res.intf_caps.bus_ctrl_count);
        for (i = 0; i < intf_caps->bus_ctrl_count; ++i)
                intf_caps->bus_ctrl_types[i] = le16_to_cpu(
                                intf_caps_res.intf_caps.bus_ctrl_types[i]);

        error: mutex_unlock(&intf->cmd_lock);

        return err;
}

static int ixx_act_ib_xxx_init_ctrl(struct ixx_pci_priv* priv)
{
        int err = -ENODEV;
        u8* fifo;
        u32 __iomem *altera_mailbox;
        u32 *tx_cmd_fifo, *rx_cmd_fifo;
        struct ixx_ctrl_init_req ctrl_init_req = { { 0 } };
        struct ixx_ctrl_init_res ctrl_init_res = { { 0 } };
        struct ixx_pci_interface *intf = priv->intf;
        u8 can_opmode, can_exmode = CAN_IB2X0_EXMODE_DISABLED;

        can_opmode = CAN_IB2X0_OPMODE_EXTENDED | CAN_IB2X0_OPMODE_STANDARD;

        if (priv->can.ctrlmode & CAN_CTRLMODE_BERR_REPORTING)
                can_opmode |= CAN_IB2X0_OPMODE_ERRFRAME;
        if (priv->can.ctrlmode & CAN_CTRLMODE_LISTENONLY)
                can_opmode |= CAN_IB2X0_OPMODE_LISTONLY;

#ifdef CANFD_CAPABLE
        if (priv->can.ctrlmode & (CAN_CTRLMODE_FD | CAN_CTRLMODE_FD_NON_ISO)) {
                can_exmode |= CAN_IB2X0_EXMODE_EXTDATA;
                can_exmode |= CAN_IB2X0_EXMODE_FASTDATA;
                can_exmode |= CAN_IB2X0_EXMODE_ISOFD;
        }
        if (priv->can.ctrlmode & CAN_CTRLMODE_FD_NON_ISO)
                can_exmode &= ~CAN_IB2X0_EXMODE_ISOFD;
#endif

        tx_cmd_fifo = priv->intf->memvadd + CAN_IB2X0_PCR_RES_VER_OFF;

        ctrl_init_req.dal_req.req_size = sizeof(struct ixx_ctrl_init_req);
        ctrl_init_req.dal_req.req_code = CAN_IB2X0_INIT_V2_CMD;
        ctrl_init_req.dal_req.req_port = priv->ctrl_idx;
        ctrl_init_req.dal_req.req_socket = 0xffff;
        ctrl_init_req.mode = can_opmode;
        ctrl_init_req.bittiming.mode = IXX_BTMODE_NAT;
        ctrl_init_req.bittiming.bps = priv->can.bittiming.brp;
        ctrl_init_req.bittiming.ts1 = priv->can.bittiming.phase_seg1
                        + priv->can.bittiming.prop_seg;
        ctrl_init_req.bittiming.ts2 = priv->can.bittiming.phase_seg2;
        ctrl_init_req.bittiming.sjw = priv->can.bittiming.sjw;
        ctrl_init_req.exmode = can_exmode;

#ifdef CANFD_CAPABLE
        if (can_exmode) {
                ctrl_init_req.data_bittiming.mode = IXX_BTMODE_NAT;
                ctrl_init_req.data_bittiming.bps = priv->can.data_bittiming.brp;
                ctrl_init_req.data_bittiming.ts1 =
                                priv->can.data_bittiming.phase_seg1
                                                + priv->can.data_bittiming.prop_seg;
                ctrl_init_req.data_bittiming.ts2 =
                                priv->can.data_bittiming.phase_seg2;
                ctrl_init_req.data_bittiming.sjw = priv->can.data_bittiming.sjw;
                ctrl_init_req.data_bittiming.tdo = (1
                                + priv->can.data_bittiming.phase_seg1
                                + priv->can.data_bittiming.prop_seg)
                                * priv->can.data_bittiming.brp;
        }
#endif

        ctrl_init_res.dal_res.res_size = sizeof(struct ixx_ctrl_init_res);
        ctrl_init_res.dal_res.ret_size = 0;
        ctrl_init_res.dal_res.ret_code = 0xffffffff;

        mutex_lock_interruptible(&intf->cmd_lock);
        err = ixx_act_ib_xxx_send_cmd(intf, tx_cmd_fifo, &ctrl_init_req.dal_req,
                        &ctrl_init_res.dal_res);

        if (err)
                goto error;

        altera_mailbox = (u32*) (intf->reg1vadd
                        + CAN_IB2X0_PCIE_ALTERA_LCR_P2A_MAILBOXRW_OFF);
        iowrite32(0xffffffff, altera_mailbox);

        fifo = ((u8*) tx_cmd_fifo);
        fifo += CAN_IB2X0_PCR_RES_HEADER_SIZE;
        fifo += (tx_cmd_fifo[CAN_IB2X0_PCR_RES_OBJ_SIZE]
                        * tx_cmd_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ]);
        rx_cmd_fifo = (u32*) fifo;
        err = ixx_act_ib_xxx_rcv_cmd(intf, rx_cmd_fifo, &ctrl_init_req.dal_req,
                        &ctrl_init_res.dal_res);
        if (err)
                goto error;

        netdev_dbg(priv->netdev,
                        "mode=0x%08x setting bps=0x%08x ts1=0x%08x ts2=0x%08x sjw=0x%08x\n",
                        can_opmode, ctrl_init_req.bittiming.bps,
                        ctrl_init_req.bittiming.ts1,
                        ctrl_init_req.bittiming.ts2,
                        ctrl_init_req.bittiming.sjw);
        netdev_dbg(priv->netdev,
                        "data setting bps=0x%08x ts1=0x%08x ts2=0x%08x sjw=0x%08x tdo=0x%08x\n",
                        ctrl_init_req.data_bittiming.bps,
                        ctrl_init_req.data_bittiming.ts1,
                        ctrl_init_req.data_bittiming.ts2,
                        ctrl_init_req.data_bittiming.sjw,
                        ctrl_init_req.data_bittiming.tdo);

        error: mutex_unlock(&priv->intf->cmd_lock);

        return ctrl_init_res.dal_res.ret_code;
}

static int ixx_act_ib_xxx_start_ctrl(struct ixx_pci_priv* priv, u32 * time_ref)
{
        int err = -ENODEV;
        u8* fifo;
        u32 __iomem *altera_mailbox;
        u32 *tx_cmd_fifo, *rx_cmd_fifo;
        struct ixx_ctrl_start_req ctrl_start_req = { { 0 } };
        struct ixx_ctrl_start_res ctrl_start_res = { { 0 } };
        struct ixx_pci_interface *intf = priv->intf;

        tx_cmd_fifo = intf->memvadd + CAN_IB2X0_PCR_RES_VER_OFF;

        ctrl_start_req.dal_req.req_size = sizeof(struct ixx_ctrl_start_req);
        ctrl_start_req.dal_req.req_code = CAN_IB2X0_START_CMD;
        ctrl_start_req.dal_req.req_port = priv->ctrl_idx;
        ctrl_start_req.dal_req.req_socket = 0xffff;

        ctrl_start_res.dal_res.res_size = sizeof(struct ixx_ctrl_start_res);
        ctrl_start_res.dal_res.ret_size = 0;
        ctrl_start_res.dal_res.ret_code = 0xffffffff;
        ctrl_start_res.start_time = 0;

        mutex_lock_interruptible(&priv->intf->cmd_lock);
        err = ixx_act_ib_xxx_send_cmd(intf, tx_cmd_fifo,
                        &ctrl_start_req.dal_req, &ctrl_start_res.dal_res);

        if (err)
                goto error;

        altera_mailbox = (u32*) (intf->reg1vadd
                        + CAN_IB2X0_PCIE_ALTERA_LCR_P2A_MAILBOXRW_OFF);
        iowrite32(0xffffffff, altera_mailbox);

        fifo = ((u8*) tx_cmd_fifo);
        fifo += CAN_IB2X0_PCR_RES_HEADER_SIZE;
        fifo += (tx_cmd_fifo[CAN_IB2X0_PCR_RES_OBJ_SIZE]
                        * tx_cmd_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ]);
        rx_cmd_fifo = (u32*) fifo;
        err = ixx_act_ib_xxx_rcv_cmd(intf, rx_cmd_fifo, &ctrl_start_req.dal_req,
                        &ctrl_start_res.dal_res);
        if (err)
                goto error;

        if (time_ref)
                *time_ref = ctrl_start_res.start_time;

        error: mutex_unlock(&priv->intf->cmd_lock);

        return err;
}

static int ixx_act_ib_xxx_stop_ctrl(struct ixx_pci_priv* priv)
{
        int err = -ENODEV;
        u8* fifo;
        u32 __iomem *altera_mailbox;
        u32 *tx_cmd_fifo, *rx_cmd_fifo;
        struct ixx_ctrl_stop_req ctrl_stop_req = { { 0 } };
        struct ixx_ctrl_stop_res ctrl_stop_res = { { 0 } };
        struct ixx_pci_interface *intf = priv->intf;

        tx_cmd_fifo = intf->memvadd + CAN_IB2X0_PCR_RES_VER_OFF;

        ctrl_stop_req.dal_req.req_size = sizeof(struct ixx_ctrl_stop_req);
        ctrl_stop_req.dal_req.req_code = CAN_IB2X0_STOP_CMD;
        ctrl_stop_req.dal_req.req_port = priv->ctrl_idx;
        ctrl_stop_req.dal_req.req_socket = 0xffff;
        ctrl_stop_req.action = 0x3;

        ctrl_stop_res.dal_res.res_size = sizeof(struct ixx_ctrl_stop_res);
        ctrl_stop_res.dal_res.ret_size = 0;
        ctrl_stop_res.dal_res.ret_code = 0xffffffff;

        mutex_lock_interruptible(&priv->intf->cmd_lock);
        err = ixx_act_ib_xxx_send_cmd(intf, tx_cmd_fifo, &ctrl_stop_req.dal_req,
                        &ctrl_stop_res.dal_res);
        if (err)
                goto error;

        altera_mailbox = (u32*) (intf->reg1vadd
                        + CAN_IB2X0_PCIE_ALTERA_LCR_P2A_MAILBOXRW_OFF);
        iowrite32(0xffffffff, altera_mailbox);

        fifo = ((u8*) tx_cmd_fifo);
        fifo += CAN_IB2X0_PCR_RES_HEADER_SIZE;
        fifo += (tx_cmd_fifo[CAN_IB2X0_PCR_RES_OBJ_SIZE]
                        * tx_cmd_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ]);
        rx_cmd_fifo = (u32*) fifo;
        err = ixx_act_ib_xxx_rcv_cmd(intf, rx_cmd_fifo, &ctrl_stop_req.dal_req,
                        &ctrl_stop_res.dal_res);

        if (!err && !ctrl_stop_res.dal_res.ret_code)
                priv->can.state = CAN_STATE_STOPPED;

        error: mutex_unlock(&priv->intf->cmd_lock);

        return err;
}

static int ixx_act_ib_xxx_get_dev_info(struct ixx_pci_interface * intf,
                struct ixx_intf_info* dev_info)
{
        int err = -ENODEV;
        u8* fifo;
        u32 __iomem *altera_mailbox;
        u32 *tx_cmd_fifo, *rx_cmd_fifo;
        struct ixx_brd_get_intf_info_req dev_info_req = { { 0 } };
        struct ixx_brd_get_intf_info_res dev_info_res = { { 0 } };

        tx_cmd_fifo = intf->memvadd + CAN_IB2X0_PCR_RES_VER_OFF;

        dev_info_req.dal_req.req_size =
                        sizeof(struct ixx_brd_get_intf_info_req);
        dev_info_req.dal_req.req_code = CAN_IB2X0_BRD_GET_DEVINFO_CMD;
        dev_info_req.dal_req.req_port = 0xffff;
        dev_info_req.dal_req.req_socket = 0xffff;

        dev_info_res.dal_res.res_size =
                        sizeof(struct ixx_brd_get_intf_info_res);
        dev_info_res.dal_res.ret_size = 0;
        dev_info_res.dal_res.ret_code = 0xffffffff;

        mutex_lock_interruptible(&intf->cmd_lock);
        err = ixx_act_ib_xxx_send_cmd(intf, tx_cmd_fifo, &dev_info_req.dal_req,
                        &dev_info_res.dal_res);
        if (err)
                goto error;

        altera_mailbox = (u32*) (intf->reg1vadd
                        + CAN_IB2X0_PCIE_ALTERA_LCR_P2A_MAILBOXRW_OFF);
        iowrite32(0xffffffff, altera_mailbox);

        fifo = ((u8*) tx_cmd_fifo);
        fifo += CAN_IB2X0_PCR_RES_HEADER_SIZE;
        fifo += (tx_cmd_fifo[CAN_IB2X0_PCR_RES_OBJ_SIZE]
                        * tx_cmd_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ]);
        rx_cmd_fifo = (u32*) fifo;
        err = ixx_act_ib_xxx_rcv_cmd(intf, rx_cmd_fifo, &dev_info_req.dal_req,
                        &dev_info_res.dal_res);
        if (err)
                goto error;

        if (dev_info)
                memcpy(dev_info, &dev_info_res.info,
                                sizeof(dev_info_res.info));

        error: mutex_unlock(&intf->cmd_lock);

        return dev_info_res.dal_res.ret_code;
}

static int ixx_act_ib_xxx_int_ena_req(struct ixx_pci_interface * intf,
                u8 enable)
{
        u8 result = 0;
        unsigned int regval = 0;

        regval = le32_to_cpu(
                        ioread32(intf->reg1vadd + PCIE_ALTERALCR_A2P_INTENA));

        result = 0 != (regval & 0x00FF0000);

        if (enable) {
                iowrite32(cpu_to_le32(regval | 0x00FF0000),
                                intf->reg1vadd + PCIE_ALTERALCR_A2P_INTENA);
        } else {
                iowrite32(cpu_to_le32(regval & 0xFF00FFFF),
                                intf->reg1vadd + PCIE_ALTERALCR_A2P_INTENA);
        }

        return result;
}

static void ixx_act_ib_xxx_int_clr_req(struct ixx_pci_interface * intf)
{
        unsigned int regval;

        regval = ioread32(intf->reg1vadd + PCIE_ALTERA_LCR_INTCSR);
        iowrite32(regval, intf->reg1vadd + PCIE_ALTERA_LCR_INTCSR);

        return;
}

static int ixx_act_ib_xxx_int_get_stat(struct ixx_pci_interface * intf)
{
        unsigned int reg_val = 0;
        unsigned int result = 0;

        reg_val = le32_to_cpu(
                        ioread32(intf->reg1vadd + PCIE_ALTERA_LCR_INTCSR));

        result = reg_val & 0x00FF0080;

        return result;
}

static irqreturn_t ixx_act_ib_xxx_irq_handler(int irq, void *dev_id)
{
        struct ixx_pci_interface *intf = (struct ixx_pci_interface *) dev_id;
        struct ixx_pci_priv *priv;
        u8 enable = 0;
        u32 *rx_fifo;
        u32 read_index, write_index;

        if (!ixx_act_ib_xxx_int_get_stat(intf)) {
                return IRQ_NONE;
        }

        enable = ixx_act_ib_xxx_int_ena_req(intf, 0);
        ixx_act_ib_xxx_int_clr_req(intf);

        if (!enable) {
                enable = ixx_act_ib_xxx_int_ena_req(intf, 1);
                return IRQ_HANDLED;
        }

        for (priv = intf->priv; priv; priv = priv->next_siblings) {
                rx_fifo = priv->rx_fifo;
                read_index = rx_fifo[CAN_IB2X0_PCR_RES_READ_IDX] + 1;
                read_index = read_index == rx_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ] ?
                                0 : read_index;
                write_index = rx_fifo[CAN_IB2X0_PCR_RES_WRITE_IDX];

                if (priv->state != IXX_STATE_RUNNING)
                        continue;

                priv->intf->handle_data |= (0x01 << priv->ctrl_idx);

                napi_schedule(&priv->napi);
        }

        if ((intf->handle_data & intf->started_mask) != intf->started_mask) {
                ixx_act_ib_xxx_int_ena_req(intf, 1);
        }

        return IRQ_HANDLED;
}

#ifdef _REAL_LOOPBACK
static int ixx_act_ib_xxx_handle_srr_canmsg(struct ixx_pci_priv *priv,
                u32 * ifi_base)
{
        u8 frn;
        u32 ifi_reg_dlc;
        u32 timestamp;

        ifi_reg_dlc = le32_to_cpu(ifi_base[IFIREG_DLC]);

        priv->netdev->stats.tx_packets += 1;
        priv->netdev->stats.tx_bytes += ifi_reg_dlc & IFI_R0_RD_DLC;
        frn = (ifi_reg_dlc & IFI_R0_RD_FRN) >> IFI_R0_FRN_S;

        timestamp = le32_to_cpu(ifi_base[IFIREG_TIMESTAMP]);

        return ixxat_pci_handle_frn(priv, frn, timestamp);
}

#ifdef CANFD_CAPABLE
static int ixx_act_ib_xxx_handle_srr_canfdmsg(struct ixx_pci_priv *priv,
                u32 * ifi_base)
{
        u8 frn;
        u32 ifi_reg_dlc;
        u32 timestamp;

        ifi_reg_dlc = le32_to_cpu(ifi_base[IXX_IFI_DLC]);

        priv->netdev->stats.tx_packets += 1;
        priv->netdev->stats.tx_bytes += can_dlc2len(
                        ifi_reg_dlc & IFIFD_RXFIFO_DLC);
        frn = (ifi_reg_dlc & IFIFD_RXFIFO_FRN) >> IFIFD_RXFIFO_FRN_S;

        timestamp = le32_to_cpu(ifi_base[IXX_IFI_RXTIMESTAMP]);

        return ixxat_pci_handle_frn(priv, frn, timestamp);
}
#endif

#endif

static int ixx_act_ib_xxx_handle_canmsg(struct ixx_pci_priv *priv,
                u32 * ifi_base)
{
        struct can_frame *can_frame;
        struct sk_buff *skb;
        u32 raw_id, raw_dlc;
        u32* data_dst;
        int i, j;

        skb = alloc_can_skb(priv->netdev, &can_frame);
        if (!skb)
                return 0;

        data_dst = (u32*) can_frame->data;

        raw_dlc = le32_to_cpu(ifi_base[IFIREG_DLC]);
        raw_id = le32_to_cpu(ifi_base[IFIREG_IDENTIFER]);

        can_frame->can_dlc = (raw_dlc & IFI_R0_RD_DLC) >> IFI_R0_DLC_S;

        if (raw_id & IFI_R1_IDE) {
                can_frame->can_id |= CAN_EFF_FLAG;
                can_frame->can_id |= ((raw_id & IFI_R1_IDEXT_18_28) << 18)
                                + ((raw_id & IFI_R1_IDEXT_00_17) >> 11);
        } else
                can_frame->can_id |= (raw_id & IFI_R1_IDSTD) >> IFI_R1_IDSTD_S;

        if (raw_dlc & IFI_R0_RD_RTR)
                can_frame->can_id |= CAN_RTR_FLAG;
        else {
                for (i = 0, j = 0; i < can_frame->can_dlc;
                                ++j, i = i + sizeof(u32)) {
                        data_dst[j] = le32_to_cpu(ifi_base[IFIREG_DATA14 + j]);
                }
        }

        ixxat_pci_get_ts_tv(priv, le32_to_cpu(ifi_base[IFIREG_TIMESTAMP]),
                        &skb->tstamp);

        netif_receive_skb(skb);
        priv->netdev->stats.rx_packets++;
        priv->netdev->stats.rx_bytes += can_frame->can_dlc;

        return 1;
}

#ifdef CANFD_CAPABLE
static int ixx_act_ib_xxx_handle_canfdmsg(struct ixx_pci_priv *priv,
                u32 * ifi_base)
{
        struct canfd_frame *can_frame;
        struct sk_buff *skb;
        u32 raw_id, raw_dlc;
        u32* data_dst;
        int i, j;

        skb = alloc_canfd_skb(priv->netdev, &can_frame);
        if (!skb)
                return 0;

        data_dst = (u32*) can_frame->data;

        raw_dlc = le32_to_cpu(ifi_base[IXX_IFI_DLC]);
        raw_id = le32_to_cpu(ifi_base[IXX_IFI_ID]);

        can_frame->len = can_dlc2len(
                        (raw_dlc & IFIFD_RXFIFO_DLC) >> IFIFD_RXFIFO_DLC_S);

        if (raw_id & IFIFD_RXFIFO_IDE) {
                can_frame->can_id |= CAN_EFF_FLAG;
                can_frame->can_id |= ((raw_id & IFIFD_RXFIFO_IDEXT_18_28) << 18)
                                + ((raw_id & IFIFD_RXFIFO_IDEXT_00_17) >> 11);
        } else
                can_frame->can_id |= (raw_id & IFIFD_RXFIFO_IDSTD)
                                >> IFIFD_RXFIFO_IDSTD_S;

        if (raw_dlc & IFIFD_RXFIFO_RTR)
                can_frame->can_id |= CAN_RTR_FLAG;
        else {
                for (i = 0, j = 0; i < can_frame->len;
                                ++j, i = i + sizeof(u32)) {
                        data_dst[j] = le32_to_cpu(ifi_base[IXX_IFI_DATA + j]);
                }
        }

        if (raw_dlc & IFIFD_RXFIFO_BRS)
                can_frame->flags |= CANFD_BRS;
        if (raw_dlc & IFIFD_RXFIFO_ESI)
                can_frame->flags |= CANFD_ESI;

        ixxat_pci_get_ts_tv(priv, le32_to_cpu(ifi_base[IXX_IFI_RXTIMESTAMP]),
                        &skb->tstamp);

        netif_receive_skb(skb);
        priv->netdev->stats.rx_packets++;
        priv->netdev->stats.rx_bytes += can_frame->len;

        return 1;
}
#endif

static int ixx_act_ib_xxx_handle_ifi_msg(struct ixx_pci_priv *priv,
                u32 * ifi_base)
{
        u32 raw_dlc;
        int result = 0;

        raw_dlc = le32_to_cpu(ifi_base[IFIREG_DLC]);

        if (raw_dlc & IFI_R0_RD_FRN) {
#ifdef _REAL_LOOPBACK
                result += ixx_act_ib_xxx_handle_srr_canmsg(priv, ifi_base);
#else
                result += ixx_act_ib_xxx_handle_canmsg(priv, ifi_base);
#endif
        } else {
                result += ixx_act_ib_xxx_handle_canmsg(priv, ifi_base);
        }

        return result;
}

#ifdef CANFD_CAPABLE
static int ixx_act_ib_xxx_handle_ififd_msg(struct ixx_pci_priv *priv,
                u32 * ifi_base)
{
        u32 raw_dlc;
        int result = 0;

        raw_dlc = le32_to_cpu(ifi_base[IXX_IFI_DLC]);

        if (raw_dlc & IFIFD_RXFIFO_FRN) {
#ifdef _REAL_LOOPBACK
                result += ixx_act_ib_xxx_handle_srr_canfdmsg(priv, ifi_base);
#else
                result += ixx_act_ib_xxx_handle_canfdmsg(priv, ifi_base);
#endif
        } else {
                result += ixx_act_ib_xxx_handle_canfdmsg(priv, ifi_base);
        }

        return result;
}
#endif

static int ixx_act_ib_xxx_create_error_frame(struct ixx_pci_priv *priv,
                u32 raw_status, u32 time)
{
        struct net_device *netdev = priv->netdev;
        struct can_frame *can_frame;
        struct sk_buff *skb;

        /* allocate an skb to store the error frame */
        skb = alloc_can_err_skb(netdev, &can_frame);
        if (!skb)
                return 0;

        switch (raw_status) {
        case CAN_IB2X0_CAN_ERROR_ACK:
                can_frame->can_id |= CAN_ERR_ACK;
                netdev->stats.tx_errors++;
                break;
        case CAN_IB2X0_CAN_ERROR_BIT:
                can_frame->can_id |= CAN_ERR_PROT;
                can_frame->data[2] |= CAN_ERR_PROT_BIT;
                netdev->stats.rx_errors++;
                break;
        case CAN_IB2X0_CAN_ERROR_CRC:
                can_frame->can_id |= CAN_ERR_PROT;
                can_frame->data[3] |= CAN_ERR_PROT_LOC_CRC_SEQ;
                netdev->stats.rx_errors++;
                break;
        case CAN_IB2X0_CAN_ERROR_FORM:
                can_frame->can_id |= CAN_ERR_PROT;
                can_frame->data[2] |= CAN_ERR_PROT_FORM;
                netdev->stats.rx_errors++;
                break;
        case CAN_IB2X0_CAN_ERROR_STUFF:
                can_frame->can_id |= CAN_ERR_PROT;
                can_frame->data[2] |= CAN_ERR_PROT_STUFF;
                netdev->stats.rx_errors++;
                break;
        case CAN_IB2X0_CAN_ERROR_OTHER:
                can_frame->can_id |= CAN_ERR_PROT;
                can_frame->data[2] |= CAN_ERR_PROT_UNSPEC;
                netdev->stats.rx_errors++;
                break;
        default:
                can_frame->can_id |= CAN_ERR_PROT;
                netdev->stats.rx_errors++;
        }

        ixxat_pci_get_ts_tv(priv, le32_to_cpu(time), &skb->tstamp);

        netif_receive_skb(skb);
        netdev->stats.rx_packets++;
        netdev->stats.rx_bytes += can_frame->can_dlc;
        return 1;
}

static int ixx_act_ib_xxx_create_status_frame(struct ixx_pci_priv *priv,
                u32 raw_status, u32 time)
{
        struct net_device *netdev = priv->netdev;
        struct can_frame *can_frame;
        struct sk_buff *skb;
        u32 new_state = 0;
        unsigned long spin_flags;
#ifdef _REAL_LOOPBACK
        u32 tmp_read;
#endif

        /* nothing should be sent while in BUS_OFF state */
        if (priv->can.state == CAN_STATE_BUS_OFF)
                return 0;

        if (!raw_status) {
                /* no error bit (back to active state) */
                priv->can.state = CAN_STATE_ERROR_ACTIVE;
                return 0;
        }

        /* allocate an skb to store the error frame */
        skb = alloc_can_err_skb(netdev, &can_frame);
        if (!skb)
                return 0;

        if (raw_status & CAN_IB2X0_CAN_STATUS_BUSOFF) {
                can_frame->can_id |= CAN_ERR_BUSOFF;
                new_state = CAN_STATE_BUS_OFF;
                priv->can.can_stats.bus_off++;
                can_bus_off(netdev);
        } else {
                if (raw_status & CAN_IB2X0_CAN_STATUS_ERRLIM) {
                        can_frame->can_id |= CAN_ERR_CRTL;
                        can_frame->data[1] |= CAN_ERR_CRTL_TX_WARNING;
                        can_frame->data[1] |= CAN_ERR_CRTL_RX_WARNING;
                        priv->can.can_stats.error_warning++;
                        new_state = CAN_STATE_ERROR_WARNING;
                }

                if (raw_status & CAN_IB2X0_CAN_STATUS_OVRRUN) {
                        can_frame->can_id |= CAN_ERR_PROT;
                        can_frame->data[2] |= CAN_ERR_PROT_OVERLOAD;
                        netdev->stats.rx_over_errors++;

#ifdef _REAL_LOOPBACK
                        spin_lock_irqsave(&priv->rcv_lock, spin_flags);
                        tmp_read = priv->frn_read + 1;
                        if (tmp_read > IXX_PCI_MAX_TX_TRANS)
                                tmp_read = 1;

                        if (tmp_read != priv->frn_write) {
                                can_free_echo_skb(priv->netdev,
                                                priv->frn_read - 1);
                                priv->frn_read = tmp_read;
                        }
                        spin_unlock_irqrestore(&priv->rcv_lock, spin_flags);
#endif
                }

                if (!new_state)
                        new_state = CAN_STATE_ERROR_ACTIVE;
        }

        priv->can.state = new_state;

        ixxat_pci_get_ts_tv(priv, le32_to_cpu(time), &skb->tstamp);

        netif_receive_skb(skb);
        netdev->stats.rx_packets++;
        netdev->stats.rx_bytes += can_frame->can_dlc;

        return 1;
}

static int ixx_act_ib_xxx_handle_error(struct ixx_pci_priv *priv,
                struct ixx_can_msg *rx)
{
        u8 raw_status = 0;

        /* nothing should be sent while in BUS_OFF state */
        if (priv->can.state == CAN_STATE_BUS_OFF)
                return 0;

        raw_status = le32_to_cpu(rx->data[0]);

        return ixx_act_ib_xxx_create_error_frame(priv, raw_status,
                        le32_to_cpu(rx->time));
}

static int ixx_act_ib_xxx_handle_status(struct ixx_pci_priv *priv,
                struct ixx_can_msg *rx)
{
        u8 raw_status = 0;

        if (!rx)
                return 0;

        raw_status = le32_to_cpu(rx->data[0]);

        return ixx_act_ib_xxx_create_status_frame(priv, raw_status,
                        le32_to_cpu(rx->time));
}

#ifdef CANFD_CAPABLE
static int ixx_act_ib_xxx_handle_fd_error(struct ixx_pci_priv *priv,
                struct ixx_canfd_msg *rx)
{
        u8 raw_status = 0;

        /* nothing should be sent while in BUS_OFF state */
        if (priv->can.state == CAN_STATE_BUS_OFF)
                return 0;

        raw_status = le32_to_cpu(rx->data[0]);

        return ixx_act_ib_xxx_create_error_frame(priv, raw_status,
                        le32_to_cpu(rx->time));
}

static int ixx_act_ib_xxx_handle_fd_status(struct ixx_pci_priv *priv,
                struct ixx_canfd_msg *rx)
{
        u8 raw_status = 0;

        if (!rx)
                return 0;

        raw_status = le32_to_cpu(rx->data[0]);

        return ixx_act_ib_xxx_create_status_frame(priv, raw_status,
                        le32_to_cpu(rx->time));
}
#endif

static int ixx_act_ib_xxx_handle_vci_msg(struct ixx_pci_priv *priv, u32* data)
{
        u32 msg_type;
        u32 result = 0;
        struct ixx_can_msg *rx = (struct ixx_can_msg *) data;

        msg_type = (le32_to_cpu(rx->flags) & CAN_IB2X0_MSG_FLAGS_TYPE);
        switch (msg_type) {

        case CAN_IB2X0_CAN_STATUS:
                result = ixx_act_ib_xxx_handle_status(priv, rx);
                break;

        case CAN_IB2X0_CAN_ERROR:
                result = ixx_act_ib_xxx_handle_error(priv, rx);
                break;

        case CAN_IB2X0_CAN_INFO:
                result = 0;
                // can_usb_pro_handle_ts(priv, can_msg);
                break;

        default:
                netdev_err(priv->netdev,
                                "unhandled rec type 0x%02x (%d): ignored\n",
                                msg_type, msg_type);
                ixxat_dump_mem("unknown vci msg", rx, sizeof(*rx));
                break;
        }

        return result;
}

#ifdef CANFD_CAPABLE
static int ixx_act_ib_xxx_handle_vci_fd_msg(struct ixx_pci_priv *priv,
                u32* data)
{
        u32 msg_type;
        u32 result = 0;
        struct ixx_canfd_msg *rx = (struct ixx_canfd_msg *) data;

        msg_type = (le32_to_cpu(rx->flags) & CAN_IB2X0_MSG_FLAGS_TYPE);
        switch (msg_type) {

        case CAN_IB2X0_CAN_STATUS:
                result = ixx_act_ib_xxx_handle_fd_status(priv, rx);
                break;

        case CAN_IB2X0_CAN_ERROR:
                result = ixx_act_ib_xxx_handle_fd_error(priv, rx);
                break;

        case CAN_IB2X0_CAN_INFO:
                result = 0;
                // can_usb_pro_handle_ts(priv, can_msg);
                break;

        default:
                netdev_err(priv->netdev,
                                "unhandled rec type 0x%02x (%d): ignored\n",
                                msg_type, msg_type);
                ixxat_dump_mem("unknown vci msg", rx, sizeof(*rx));
                break;
        }

        return result;
}
#endif

static int ixx_act_ib_xxx_handle_devmsg(struct ixx_pci_priv *priv)
{
        u32 *rx_fifo;
        u32 * ifi_base;
        u32 read_index, write_index, msg_type;
        u32 result = 0;

        rx_fifo = priv->rx_fifo;

        write_index = rx_fifo[CAN_IB2X0_PCR_RES_WRITE_IDX];
        read_index = rx_fifo[CAN_IB2X0_PCR_RES_READ_IDX] + 1;
        read_index = read_index == rx_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ] ?
                        0 : read_index;

        if (read_index != write_index) {
                ifi_base =
                                (u32*) (((u8*) &rx_fifo[CAN_IB2X0_PCR_RES_DATA])
                                                + (read_index
                                                                * rx_fifo[CAN_IB2X0_PCR_RES_OBJ_SIZE]));
                msg_type = ifi_base[0];

                switch (msg_type) {
                case CAN_IB2X0_MSG_TYPE_IFI:
                        result = priv->adapter->handle_msg(priv, ++ifi_base);
                        break;
                case CAN_IB2X0_MSG_TYPE_VCI:
                        result = priv->adapter->handle_vci_msg(priv,
                                        ++ifi_base);
                        break;
                default:
                        netdev_warn(priv->netdev, "unknown message received");
                }

                rx_fifo[CAN_IB2X0_PCR_RES_READ_IDX] =
                                (rx_fifo[CAN_IB2X0_PCR_RES_READ_IDX] + 1)
                                                == rx_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ] ?
                                                0 :
                                                rx_fifo[CAN_IB2X0_PCR_RES_READ_IDX]
                                                                + 1;
        }

        return result;
}

static int ixx_act_ib_xxx_napi_rx_poll(struct napi_struct *napi, int quota)
{
        struct ixx_pci_priv *priv = netdev_priv(napi->dev);
        u32 read_index, write_index;
        u32 *rx_fifo;
        u32 *tx_fifo;
        struct ixx_pci_interface * intf = priv->intf;
        int nxpackets = 0;
        unsigned long spin_flags;

        while (nxpackets < quota) {
                rx_fifo = priv->rx_fifo;
                read_index = rx_fifo[CAN_IB2X0_PCR_RES_READ_IDX] + 1;
                read_index = read_index == rx_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ] ?
                                0 : read_index;
                write_index = rx_fifo[CAN_IB2X0_PCR_RES_WRITE_IDX];

                if (read_index == write_index ||
                    !(priv->intf->started_mask & (0x01 << priv->ctrl_idx)))
                        break;

                nxpackets += ixx_act_ib_xxx_handle_devmsg(priv);
        }

        spin_lock_irqsave(&priv->rcv_lock, spin_flags);
        if (priv->tx_fifo) {
                tx_fifo = priv->tx_fifo;

                if (tx_fifo[CAN_IB2X0_PCR_RES_READ_IDX]
                                != tx_fifo[CAN_IB2X0_PCR_RES_WRITE_IDX]
                                && priv->frn_write != priv->frn_read
                                && netif_queue_stopped(priv->netdev)) {
                        netif_wake_queue(priv->netdev);
                        netdev_dbg(priv->netdev, "napi netif_wake_queue");
                }
        }
        spin_unlock_irqrestore(&priv->rcv_lock, spin_flags);

        if (nxpackets < quota) {
                priv->intf->handle_data &= ~(0x01 << priv->ctrl_idx);
                napi_complete(&priv->napi);

                ixx_act_ib_xxx_int_ena_req(intf, 1);

        }

        return nxpackets;
}

static int ixx_act_ib_xxx_start_xmit(struct sk_buff *skb,
                struct net_device *netdev)
{
        int err;
        struct ixx_pci_priv* priv = netdev_priv(netdev);
        struct can_frame *cf = (struct can_frame *) skb->data;
        u32 __iomem *altera_mailbox;
        u32 *fifo = priv->tx_fifo;
        u32 *data_dst;
        u32 *fifo_data;
        u32 can_id, can_dlc;
        int i, j;
        u32 write_index, read_index, obj_size;
        unsigned long spin_flags;

        if (can_dropped_invalid_skb(netdev, skb)) {
                netdev_dbg(netdev, "dropped invalid skb\n");
                return NETDEV_TX_OK;
        }

        if (priv->state == IXX_STATE_STOPPED) {
                netif_stop_queue(netdev);
                return NETDEV_TX_OK;
        }

        if (priv->frn_write == priv->frn_read
                        || fifo[CAN_IB2X0_PCR_RES_WRITE_IDX]
                                        == fifo[CAN_IB2X0_PCR_RES_READ_IDX]) {
                /* should not occur except during restart */
                netdev_dbg(netdev,
                                "should never occur frn_write %d frn_read %d\n",
                                priv->frn_write, priv->frn_read);
                return NETDEV_TX_BUSY;
        }

        err = 0;

        write_index = fifo[CAN_IB2X0_PCR_RES_WRITE_IDX];
        read_index = fifo[CAN_IB2X0_PCR_RES_READ_IDX];
        obj_size = fifo[CAN_IB2X0_PCR_RES_OBJ_SIZE];

        fifo_data = (u32*) &((u8*) &fifo[CAN_IB2X0_PCR_RES_DATA])[write_index
                        * obj_size];
        fifo_data[0] = CAN_IB2X0_MSG_TYPE_IFI;
        fifo_data += 1;

        data_dst = &fifo_data[IFIREG_DATA14];

        can_dlc = 0;

        if (cf->can_id & CAN_EFF_FLAG) {
                can_id = (((cf->can_id & 0x0003FFFF) << 11)
                                + ((cf->can_id & 0x1FFC0000) >> 18))
                                | IFI_R1_IDE;
        } else {
                can_id = cf->can_id & IFI_R1_IDSTD;
        }

        if (cf->can_id & CAN_RTR_FLAG)
                can_dlc |= IFI_R0_WR_RTR;
        else
                can_dlc |= cf->can_dlc & IFI_R0_WR_DLC;

        if (priv->can.ctrlmode & CAN_CTRLMODE_ONE_SHOT)
                can_dlc |= IFI_R0_WR_SSM;

        if (!(cf->can_id & CAN_RTR_FLAG)) {
                for (i = 0, j = 0; i < cf->can_dlc; ++j, i = i + sizeof(u32)) {
                        data_dst[j] = cpu_to_le32(*((u32*) &cf->data[i]));
                }
        }

        /* prevent tx timeout */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
        netif_trans_update(netdev);
#else
        netdev->trans_start = jiffies;
#endif

        if (skb->pkt_type == PACKET_LOOPBACK) {
#ifdef _REAL_LOOPBACK
                spin_lock_irqsave(&priv->rcv_lock, spin_flags);
                if (priv->can.echo_skb[priv->frn_write - 1]) {
                        can_free_echo_skb(priv->netdev, priv->frn_write - 1);
                }

                can_put_echo_skb(skb, priv->netdev, priv->frn_write - 1);
                can_dlc |= (priv->frn_write << IFI_R0_FRN_S) & IFI_R0_RD_FRN;

                priv->frn_write++;
                if (IXX_PCI_MAX_TX_TRANS < priv->frn_write)
                        priv->frn_write = 1;
                spin_unlock_irqrestore(&priv->rcv_lock, spin_flags);
#else
                can_dlc |= (0xff << IFI_R0_FRN_S) & IFI_R0_RD_FRN;
                kfree_skb(skb);
#endif
        } else {
                kfree_skb(skb);
        }

        fifo_data[IFIREG_IDENTIFER] = cpu_to_le32(can_id);
        fifo_data[IFIREG_DLC] = cpu_to_le32(can_dlc);

        spin_lock_irqsave(&priv->rcv_lock, spin_flags);
        fifo[CAN_IB2X0_PCR_RES_WRITE_IDX] = (write_index + 1)
                        % fifo[CAN_IB2X0_PCR_RES_NUM_OBJ];
        if (priv->frn_write == priv->frn_read
                        || fifo[CAN_IB2X0_PCR_RES_WRITE_IDX]
                                        == fifo[CAN_IB2X0_PCR_RES_READ_IDX]) {
                netdev_dbg(netdev, "netif_stop_queue");
                netif_stop_queue(netdev);
        }
        spin_unlock_irqrestore(&priv->rcv_lock, spin_flags);

        altera_mailbox = (u32*) (priv->intf->reg1vadd
                        + CAN_IB2X0_PCIE_ALTERA_LCR_P2A_MAILBOXRW_OFF);
        iowrite32(0xffffffff, altera_mailbox + priv->ctrl_idx + 1);

        return NETDEV_TX_OK;
}

#ifdef CANFD_CAPABLE
static int ixx_act_ib_xxx_start_xmit_fd(struct sk_buff *skb,
                struct net_device *netdev)
{
        int err;
        struct ixx_pci_priv* priv = netdev_priv(netdev);
        struct canfd_frame *cf = (struct canfd_frame *) skb->data;
        u32 __iomem *altera_mailbox;
        u32 *fifo = priv->tx_fifo;
        u32 *data_dst;
        u32 *fifo_data;
        u32 can_id, can_dlc;
        int i, j;
        u32 write_index, read_index, obj_size;
        unsigned long spin_flags;

        if (can_dropped_invalid_skb(netdev, skb)) {
                netdev_info(netdev, "dropped invalid skb\n");
                return NETDEV_TX_OK;
        }

        if (priv->state == IXX_STATE_STOPPED) {
                netif_stop_queue(netdev);
                return NETDEV_TX_OK;
        }

        if (priv->frn_write == priv->frn_read
                        || fifo[CAN_IB2X0_PCR_RES_WRITE_IDX]
                                        == fifo[CAN_IB2X0_PCR_RES_READ_IDX]) {
                /* should not occur except during restart */
                netdev_info(netdev,
                                "should never occur frn_write %d frn_read %d\n",
                                priv->frn_write, priv->frn_read);
                return NETDEV_TX_BUSY;
        }

        err = 0;

        write_index = fifo[CAN_IB2X0_PCR_RES_WRITE_IDX];
        read_index = fifo[CAN_IB2X0_PCR_RES_READ_IDX];
        obj_size = fifo[CAN_IB2X0_PCR_RES_OBJ_SIZE];

        fifo_data = (u32*) &((u8*) &fifo[CAN_IB2X0_PCR_RES_DATA])[write_index
                        * obj_size];
        fifo_data[0] = CAN_IB2X0_MSG_TYPE_IFI;
        fifo_data += 1;

        data_dst = &fifo_data[IXX_IFI_DATA];

        can_dlc = 0;

        if (cf->can_id & CAN_EFF_FLAG) {
                can_id = (((cf->can_id & 0x0003FFFF) << 11)
                                + ((cf->can_id & 0x1FFC0000) >> 18))
                                | IFIFD_TXFIFO_IDE;
        } else {
                can_id = cf->can_id & IFIFD_TXFIFO_IDSTD;
        }

        if (cf->can_id & CAN_RTR_FLAG)
                can_dlc |= IFIFD_TXFIFO_RTR;
        else {
                can_dlc |= can_len2dlc(cf->len) & IFIFD_TXFIFO_DLC;
                if (cf->len > 8) {
                        can_dlc |= IFIFD_TXFIFO_EDL;
                }
        }
        if (priv->can.ctrlmode & CAN_CTRLMODE_ONE_SHOT)
                fifo_data[IXX_IFI_TXREPCOUNT] = 1;
        else
                fifo_data[IXX_IFI_TXREPCOUNT] = 0;

        if (!(cf->can_id & CAN_RTR_FLAG)) {
                if (cf->flags & CANFD_BRS) {
                        can_dlc |= IFIFD_TXFIFO_EDL | IFIFD_TXFIFO_BRS;
                }

                for (i = 0, j = 0; i < cf->len; ++j, i = i + sizeof(u32)) {
                        data_dst[j] = cpu_to_le32(*((u32*) &cf->data[i]));
                }
        }

        /* prevent tx timeout */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
        netif_trans_update(netdev);
#else
        netdev->trans_start = jiffies;
#endif

        if (skb->pkt_type == PACKET_LOOPBACK) {
#ifdef _REAL_LOOPBACK
                spin_lock_irqsave(&priv->rcv_lock, spin_flags);
                if (priv->can.echo_skb[priv->frn_write - 1]) {
                        can_free_echo_skb(priv->netdev, priv->frn_write - 1);
                }

                can_put_echo_skb(skb, priv->netdev, priv->frn_write - 1);
                can_dlc |= (priv->frn_write << IFIFD_TXFIFO_FRN_S)
                                & IFIFD_TXFIFO_FRN;

                priv->frn_write++;
                if (IXX_PCI_MAX_TX_TRANS < priv->frn_write)
                        priv->frn_write = 1;
                spin_unlock_irqrestore(&priv->rcv_lock, spin_flags);
#else
                can_dlc |= (0xff << IFIFD_TXFIFO_FRN_S) & IFIFD_TXFIFO_FRN;
                kfree_skb(skb);
#endif
        } else {
                kfree_skb(skb);
        }

        fifo_data[IXX_IFI_ID] = cpu_to_le32(can_id);
        fifo_data[IXX_IFI_DLC] = cpu_to_le32(can_dlc);
        fifo_data[IXX_IFI_FIFOCMD] |= IFIFD_R2_WR_ADD_MSG;

        spin_lock_irqsave(&priv->rcv_lock, spin_flags);
        fifo[CAN_IB2X0_PCR_RES_WRITE_IDX] = (write_index + 1)
                        % fifo[CAN_IB2X0_PCR_RES_NUM_OBJ];
        if (priv->frn_write == priv->frn_read
                        || fifo[CAN_IB2X0_PCR_RES_WRITE_IDX]
                                        == fifo[CAN_IB2X0_PCR_RES_READ_IDX]) {
                netif_stop_queue(netdev);
                netdev_dbg(priv->netdev, "netif_stop_queue");
        }
        spin_unlock_irqrestore(&priv->rcv_lock, spin_flags);

        altera_mailbox = (u32*) (priv->intf->reg1vadd
                        + CAN_IB2X0_PCIE_ALTERA_LCR_P2A_MAILBOXRW_OFF);
        iowrite32(0xffffffff, altera_mailbox + priv->ctrl_idx + 1);

        return NETDEV_TX_OK;
}
#endif

static int ixx_act_ib_xxx_start(struct ixx_pci_priv *priv)
{
        int err;
        u32 time_ref = 0;

        /* opening first device: */
        if (priv->ctrl_opened_count == 0) {
                err = ixx_act_ib_xxx_init_ctrl(priv);
                if (err)
                        return err;

                napi_enable(&priv->napi);
                err = ixx_act_ib_xxx_start_ctrl(priv, &time_ref);
                if (err)
                        return err;

                ixxat_pci_set_ts_now(priv, time_ref);
                priv->can.state = CAN_STATE_ERROR_ACTIVE;

                priv->frn_read = 0x1;
                priv->frn_write = 0x2;
                priv->state = IXX_STATE_RUNNING;
                priv->intf->started_mask |= (0x01 << priv->ctrl_idx);

                ixx_act_ib_xxx_int_ena_req(priv->intf, 1);
        }

        priv->ctrl_opened_count++;

        return err;
}

static int ixx_act_ib_xxx_stop(struct ixx_pci_priv *priv)
{
        int err = 0;

        if (priv->ctrl_opened_count == 1) {
                priv->can.state = CAN_STATE_STOPPED;
                priv->state = IXX_STATE_STOPPED;
                priv->intf->started_mask &= ~(0x01 << priv->ctrl_idx);

                err = ixx_act_ib_xxx_stop_ctrl(priv);

                napi_disable(&priv->napi);
                ixx_act_ib_xxx_int_ena_req(priv->intf, 0);

                priv->frn_read = 0x1;
                priv->frn_write = 0x2;

        }

        priv->ctrl_opened_count--;

        return err;
}

/*
 * handle restart but asynchronously
 */
static int ixx_act_ib_xxx_restart_task(void* user_data)
{
        struct ixx_pci_priv *priv = user_data;
        u32 time_ref = 0;
        int err, i;

        while (!kthread_should_stop()) {
                if (priv->must_quit) {
                        msleep(1);
                        continue;
                }

                wait_event_interruptible(priv->wait_queue, priv->restart_flag);
                if (priv->must_quit)
                        continue;

                napi_disable(&priv->napi);
                netif_tx_disable(priv->netdev);

                err = ixx_act_ib_xxx_stop_ctrl(priv);
                if (err) {
                        netdev_info(priv->netdev, "%s stop failed", __func__);
                        priv->can.state = CAN_STATE_BUS_OFF;
                        priv->restart_flag = 0;
                        ixx_act_ib_xxx_int_ena_req(priv->intf, 1);
                        continue;
                }

                priv->frn_read = 0x1;
                priv->frn_write = 0x2;

                for (i = 0; i < priv->can.echo_skb_max; ++i)
                        can_free_echo_skb(priv->netdev, i);

                priv->restart_flag = 0;
                priv->can.state = CAN_STATE_ERROR_ACTIVE;

                napi_enable(&priv->napi);
                ixx_act_ib_xxx_int_ena_req(priv->intf, 1);

                err = ixx_act_ib_xxx_start_ctrl(priv, &time_ref);
                if (err) {
                        netdev_info(priv->netdev, "%s start failed", __func__);
                        priv->can.state = CAN_STATE_BUS_OFF;
                        napi_disable(&priv->napi);
                        return err;
                }

                ixxat_pci_set_ts_now(priv, time_ref);
        }

        return 0;
}

/*
 * called when probing to initialize a device object.
 */
static int ixx_act_ib_xxx_init(struct ixx_pci_priv *priv)
{
        int length;
        int i = 0;

        priv->tx_fifo = priv->intf->memvadd +
        CAN_IB2X0_VER_SIZE;
        for (i = 0; i <= (priv->ctrl_idx + 2); ++i) {
                length = priv->tx_fifo[CAN_IB2X0_PCR_RES_SIZE];
                priv->tx_fifo = (u32*) (((char*) priv->tx_fifo) + length);
        }

        priv->rx_fifo = priv->intf->dmavadd;
        for (i = 0; i < priv->ctrl_idx; ++i) {
                length = priv->rx_fifo[CAN_IB2X0_PCR_RES_SIZE];
                priv->rx_fifo = (u32*) (((char*) priv->rx_fifo) + length);
        }

        priv->restart_task = kthread_run(&ixx_act_ib_xxx_restart_task, priv,
                        "restart_thread");
        if (!priv->restart_task) {
                return -ENOBUFS;
        }

        return 0;
}

static void ixx_act_ib_xxx_exit(struct ixx_pci_priv *priv)
{
        ixx_act_ib_xxx_stop_ctrl(priv);

        priv->must_quit = 1;
        priv->restart_flag = 1;
        wake_up_interruptible(&priv->wait_queue);
        if (priv->restart_task)
                kthread_stop(priv->restart_task);
}

/*
 * called when IXXAT CAN-IB2X0 adapter is unplugged
 */
static void ixx_act_ib_xxx_free(struct ixx_pci_priv *priv)
{
        if (!priv)
                return;
        netdev_dbg(priv->netdev, "%s", __func__);
        if (priv->state == IXX_STATE_RUNNING)
                ixx_act_ib_xxx_stop(priv);

        /* last device: can free ixx_act_ib_xxx_interface object now */
        if (!priv->prev_siblings && !priv->next_siblings) {
                ixx_act_ib_xxx_int_ena_req(priv->intf, 0);
                ixx_act_ib_xxx_exit(priv);

                if (priv->intf->device_irq) {
                        free_irq(priv->intf->device_irq, priv->intf);
                        pci_disable_msi(priv->pdev);
                }

                if (priv->intf->addmemvadd) {
                        kfree(priv->intf->addmemvadd);
                        priv->intf->addmemvadd = NULL;
                }

                if (priv->intf->dmavadd)
                        pci_free_consistent(priv->pdev, priv->intf->dmalen,
                                        priv->intf->dmavadd,
                                        priv->intf->dmaadd);

                iounmap(priv->intf->reg1vadd);
                release_mem_region(priv->intf->reg1add, priv->intf->reg1len);

                iounmap(priv->intf->memvadd);
                release_mem_region(priv->intf->memadd, priv->intf->memlen);
        }
}

/*
 * probe function for new IXXAT CAN-IB2X0 interface
 */
static int ixx_act_ib_xxx_probe(struct pci_dev *pdev,
                struct ixx_pci_interface * intf)
{
        int err;

        pci_set_master(pdev);

        intf->priv = 0;

        intf->reg1add = pci_resource_start(pdev, 0);
        intf->reg1len = pci_resource_len(pdev, 0);

        intf->memadd = pci_resource_start(pdev, 2);
        intf->memlen = pci_resource_len(pdev, 2);

        request_mem_region(intf->reg1add, intf->reg1len, "IXXAT PCI Registers");
        intf->reg1vadd = ioremap_nocache(intf->reg1add, intf->reg1len);
        if (!intf->reg1vadd) {
                printk("reg1vadd ioremap_nocache failed\n");
                err = -ENOBUFS;
                goto release_reg1;
        }

        request_mem_region(intf->memadd, intf->memlen, "IXXAT PCI Memory");
        intf->memvadd = ioremap_nocache(intf->memadd, intf->memlen);
        if (!intf->memvadd) {
                printk("memvadd ioremap_nocache failed\n");
                err = -ENOBUFS;
                goto release_memreg;
        }

        intf->addmemlen = CAN_IB2X0_ADDMEM_LEN;
        intf->dmalen = CAN_IB2X0_DMA_LEN;

        intf->dmavadd = pci_alloc_consistent(pdev, intf->dmalen, &intf->dmaadd);
        if (!intf->dmavadd) {
                printk("pci_alloc_consistent failed\n");
                err = -ENOBUFS;
                goto release_dma;
        } else
                memset(intf->dmavadd, 0, intf->dmalen);

        intf->addmemvadd = kzalloc(intf->addmemlen, GFP_KERNEL);
        if (!intf->addmemvadd) {
                printk("kzalloc failed\n");
                err = -ENOBUFS;
                goto release_addmem;
        }

        if (pci_enable_msi(pdev)) {
                dev_info(&pdev->dev, "msi interrupts not supported");
                pci_disable_msi(pdev);
        }

        intf->int_counter = 0;
        err = request_irq(pdev->irq, ixx_act_ib_xxx_irq_handler, IRQF_SHARED,
                        dev_name(&pdev->dev), intf);
        if (err) {
                printk("request_irq failed\n");
                goto release_irq;
        }

        intf->device_irq = pdev->irq;

        return 0;

        release_irq: kfree(intf->addmemvadd);
        pci_disable_msi(pdev);
        intf->addmemvadd = NULL;
        release_addmem: pci_free_consistent(pdev, intf->dmalen, intf->dmavadd,
                        intf->dmaadd);
        release_dma: iounmap(intf->reg1vadd);
        release_mem_region(intf->reg1add, intf->reg1len);
        release_memreg: iounmap(intf->memvadd);
        release_mem_region(intf->memadd, intf->memlen);
        release_reg1: return err;
}

static int ixx_act_ib_xxx_trigger_int(struct pci_dev *pdev,
                struct ixx_pci_interface * intf)
{
        int err;
        u8* fifo;
        u32 __iomem *altera_mailbox;
        u32 *tx_cmd_fifo, *rx_cmd_fifo;
        struct ixx_bmg_trigger_int_req trigger_req = { { 0 } };
        struct ixx_bmg_trigger_int_res trigger_res = { { 0 } };

        tx_cmd_fifo = intf->memvadd + CAN_IB2X0_PCR_RES_VER_OFF;

        /* Prepare Request */
        trigger_req.dal_req.req_size = sizeof(struct ixx_bmg_trigger_int_req);
        trigger_req.dal_req.req_code = CAN_IB2X0_BMG_TRIGGER_INT_CMD;
        trigger_req.dal_req.req_port = 0xFFFF;
        trigger_req.dal_req.req_socket = 0xFFFF;
        trigger_req.content = 0xFADECAFE;
        trigger_req.mailbox = 0x07;

        trigger_res.dal_res.res_size = sizeof(struct ixx_bmg_trigger_int_res);
        trigger_res.dal_res.ret_size = 0;
        trigger_res.dal_res.ret_code = 0xFFFFFFFF;

        mutex_lock_interruptible(&intf->cmd_lock);
        err = ixx_act_ib_xxx_send_cmd(intf, tx_cmd_fifo, &trigger_req.dal_req,
                        &trigger_res.dal_res);
        if (err)
                goto error;

        altera_mailbox = (u32*) (intf->reg1vadd
                        + CAN_IB2X0_PCIE_ALTERA_LCR_P2A_MAILBOXRW_OFF);
        iowrite32(0xffffffff, altera_mailbox);

        fifo = ((u8*) tx_cmd_fifo);
        fifo += CAN_IB2X0_PCR_RES_HEADER_SIZE;
        fifo += (tx_cmd_fifo[CAN_IB2X0_PCR_RES_OBJ_SIZE]
                        * tx_cmd_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ]);
        rx_cmd_fifo = (u32*) fifo;
        err = ixx_act_ib_xxx_rcv_cmd(intf, rx_cmd_fifo, &trigger_req.dal_req,
                        &trigger_res.dal_res);

        error: mutex_unlock(&intf->cmd_lock);

        return err;
}

static int ixx_act_ib_xxx_test_cmd(struct pci_dev *pdev,
                struct ixx_pci_interface * intf)
{
        int err = -EINVAL;
        u8* fifo;
        u8 test_request[CAN_IB2X0_BMG_PCR_CMD_MAX_SIZE] = { 0 };
        u8 test_response[CAN_IB2X0_BMG_PCR_CMD_MAX_SIZE] = { 0 };
        struct ixx_bmg_loopback_cmd_req* bmg_request =
                        (struct ixx_bmg_loopback_cmd_req*) test_request;
        struct ixx_bmg_loopback_cmd_res* bmg_response =
                        (struct ixx_bmg_loopback_cmd_res*) test_response;
        u16 wTestSize = sizeof(test_request) - sizeof(bmg_request->dal_req);
        u32 __iomem * altera_mailbox;
        u32 *tx_cmd_fifo, *rx_cmd_fifo;
        u32 index;

        tx_cmd_fifo = intf->memvadd + CAN_IB2X0_PCR_RES_VER_OFF;

        /* Prepare Request */
        bmg_request->dal_req.req_size = sizeof(test_request);
        bmg_request->dal_req.req_code = CAN_IB2X0_BMG_LOOPBACK_CMD;
        bmg_request->dal_req.req_port = 0xFFFF;
        bmg_request->dal_req.req_socket = 0xFFFF;

        bmg_response->dal_res.res_size = sizeof(test_response);
        bmg_response->dal_res.ret_size = 0;
        bmg_response->dal_res.ret_code = 0xFFFFFFFF;

        /* Fill with test data */
        for (index = 0; index < wTestSize; index++) {
                bmg_request->data[index] = (u8) index + 1;
        }

        mutex_lock_interruptible(&intf->cmd_lock);
        /* Execute command */
        err = ixx_act_ib_xxx_send_cmd(intf, tx_cmd_fifo, &bmg_request->dal_req,
                        &bmg_response->dal_res);

        altera_mailbox = (u32*) (intf->reg1vadd
                        + CAN_IB2X0_PCIE_ALTERA_LCR_P2A_MAILBOXRW_OFF);
        iowrite32(0xffffffff, altera_mailbox);

        fifo = ((u8*) tx_cmd_fifo);
        fifo += CAN_IB2X0_PCR_RES_HEADER_SIZE;
        fifo += (tx_cmd_fifo[CAN_IB2X0_PCR_RES_OBJ_SIZE]
                        * tx_cmd_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ]);
        rx_cmd_fifo = (u32*) fifo;
        err = ixx_act_ib_xxx_rcv_cmd(intf, rx_cmd_fifo, &bmg_request->dal_req,
                        &bmg_response->dal_res);
        if (err)
                goto error;

        /* Check command response */
        if (bmg_response->dal_res.ret_size
                        == (wTestSize + sizeof(bmg_request->dal_req))) {
                /* Check Response data */
                for (index = 0; index < wTestSize; index++) {
                        /* Check Response data */
                        if ((bmg_request->data[index])
                                        != bmg_response->data[index]) {
                                err = -EINVAL;
                        }
                }
        } else {
                err = -EINVAL;
        }

        error: mutex_unlock(&intf->cmd_lock);

        return err;
}

static int ixx_act_ib_xxx_test_dma(struct pci_dev *pdev,
                struct ixx_pci_interface * intf)
{
        u32 __iomem *altera_mailbox;
        u32 *tx_cmd_fifo, *rx_cmd_fifo;
        int err;
        u8* fifo;
        struct ixx_bmg_read_block_req read_req = { { 0 } };
        u8 dma_data[CAN_IB2X0_BMG_PCR_CMD_MAX_SIZE] = { 0 };

        struct ixx_bmg_read_block_res* read_res =
                        (struct ixx_bmg_read_block_res*) dma_data;

        sprintf((char*) intf->dmavadd, "DMA Memory Check");

        tx_cmd_fifo = intf->memvadd + CAN_IB2X0_PCR_RES_VER_OFF;

        //*** Prepare Request
        read_req.dal_req.req_size = sizeof(struct ixx_bmg_read_block_req);
        read_req.dal_req.req_code = CAN_IB2X0_BMG_READ_BLOCK_CMD;
        read_req.dal_req.req_port = 0xFFFF;
        read_req.dal_req.req_socket = 0xFFFF;
        read_req.addr = 0x80100000;

        read_res->dal_res.res_size = sizeof(read_res->dal_res)
                        + sizeof("DMA Memory Check");
        read_res->dal_res.ret_size = 0;
        read_res->dal_res.ret_code = 0xFFFFFFFF;

        mutex_lock_interruptible(&intf->cmd_lock);
        err = ixx_act_ib_xxx_send_cmd(intf, tx_cmd_fifo, &read_req.dal_req,
                        &read_res->dal_res);
        if (err)
                goto error;

        altera_mailbox = (u32*) (intf->reg1vadd
                        + CAN_IB2X0_PCIE_ALTERA_LCR_P2A_MAILBOXRW_OFF);
        iowrite32(0xffffffff, altera_mailbox);

        fifo = ((u8*) tx_cmd_fifo);
        fifo += CAN_IB2X0_PCR_RES_HEADER_SIZE;
        fifo += (tx_cmd_fifo[CAN_IB2X0_PCR_RES_OBJ_SIZE]
                        * tx_cmd_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ]);
        rx_cmd_fifo = (u32*) fifo;
        err = ixx_act_ib_xxx_rcv_cmd(intf, rx_cmd_fifo, &read_req.dal_req,
                        &read_res->dal_res);
        if (err)
                goto error;

        error: mutex_unlock(&intf->cmd_lock);

        err = memcmp(intf->dmavadd, read_res->data, sizeof("DMA Memory Check"));

        return err;
}

static int ixx_act_ib_xxx_upload_firmware(struct pci_dev *pdev,
                struct ixx_pci_interface * intf)
{
        u32 *tx_cmd_fifo, *rx_cmd_fifo, *db_fifo;
        u32 __iomem *altera_mailbox;
        u8 request[CAN_IB2X0_BMG_PCR_CMD_MAX_SIZE] = { 0 };
        int i, err;
        struct ixx_bmg_write_block_req* write_req;
        struct ixx_bmg_write_block_res write_res;
        const struct firmware* fw;
        u32 address;
        u16 length;
        const u8* data;

        tx_cmd_fifo = intf->memvadd + CAN_IB2X0_PCR_RES_VER_OFF;
        rx_cmd_fifo =
                        (u32*) (((u8*) tx_cmd_fifo)
                                        + (tx_cmd_fifo[CAN_IB2X0_PCR_RES_OBJ_SIZE]
                                                        * tx_cmd_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ])
                                        + CAN_IB2X0_PCR_RES_HEADER_SIZE);
        db_fifo =
                        (u32*) (((u8*) rx_cmd_fifo)
                                        + (rx_cmd_fifo[CAN_IB2X0_PCR_RES_OBJ_SIZE]
                                                        * rx_cmd_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ])
                                        + CAN_IB2X0_PCR_RES_HEADER_SIZE);

        write_req = (struct ixx_bmg_write_block_req*) request;

        write_req->dal_req.req_code = CAN_IB2X0_BMG_WRITE_BLOCK_CMD;
        write_req->dal_req.req_port = 0xFFFF;
        write_req->dal_req.req_socket = 0xFFFF;

        err = request_firmware(&fw, IXX_FIRMWARE, &pdev->dev);
        if (err) {
                dev_err(&pdev->dev, "request_firmware failed");
                return err;
        }

        for (i = 0; i < fw->size;) {
                address = be32_to_cpu(*((u32*) (fw->data + i)));
                length = be16_to_cpu(*((u16*) (fw->data + i + 4)));
                data = fw->data + i + 6;

                write_req->addr = address;
                write_req->dal_req.req_size = length
                                + sizeof(write_req->dal_req)
                                + sizeof(write_req->addr);
                memcpy(write_req->data, data, length);

                write_res.dal_res.res_size = sizeof(write_res);
                write_res.dal_res.ret_size = 0;
                write_res.dal_res.ret_code = 0xFFFFFFFF;

                err = ixx_act_ib_xxx_send_cmd(intf, tx_cmd_fifo,
                                &write_req->dal_req, &write_res.dal_res);
                if (err)
                        goto error;

                altera_mailbox = (u32*) (intf->reg1vadd
                                + CAN_IB2X0_PCIE_ALTERA_LCR_P2A_MAILBOXRW_OFF);
                iowrite32(0xffffffff, altera_mailbox);

                err = ixx_act_ib_xxx_rcv_cmd(intf, rx_cmd_fifo,
                                &write_req->dal_req, &write_res.dal_res);
                if (err) {
                        goto error;
                }

                i = i + 4 + 6 + length - 2;
        }

        error:

        return err;
}

static int ixx_act_ib_xxx_start_fw(struct pci_dev *pdev,
                struct ixx_pci_interface * intf)
{
        u32 __iomem *altera_mailbox;
        u32 *tx_cmd_fifo, *rx_cmd_fifo;
        int err;
        u8* fifo;
        struct ixx_bmg_start_firmware_req start_req = { { 0 } };
        struct ixx_bmg_start_firmware_res start_res = { { 0 } };

        tx_cmd_fifo = intf->memvadd + CAN_IB2X0_PCR_RES_VER_OFF;

        //*** Prepare Request
        start_req.dal_req.req_size = sizeof(struct ixx_bmg_start_firmware_req);
        start_req.dal_req.req_code = CAN_IB2X0_BMG_STARTFIRMWARE_CMD;
        start_req.dal_req.req_port = 0xFFFF;
        start_req.dal_req.req_socket = 0xFFFF;
        start_req.dummy = 0;

        start_res.dal_res.res_size = sizeof(struct ixx_bmg_start_firmware_res);
        start_res.dal_res.ret_size = 0;
        start_res.dal_res.ret_code = 0xFFFFFFFF;

        mutex_lock_interruptible(&intf->cmd_lock);
        err = ixx_act_ib_xxx_send_cmd(intf, tx_cmd_fifo, &start_req.dal_req,
                        &start_res.dal_res);
        if (err)
                goto error;

        altera_mailbox = (u32*) (intf->reg1vadd
                        + CAN_IB2X0_PCIE_ALTERA_LCR_P2A_MAILBOXRW_OFF);
        iowrite32(0xffffffff, altera_mailbox);

        fifo = ((u8*) tx_cmd_fifo);
        fifo += CAN_IB2X0_PCR_RES_HEADER_SIZE;
        fifo += (tx_cmd_fifo[CAN_IB2X0_PCR_RES_OBJ_SIZE]
                        * tx_cmd_fifo[CAN_IB2X0_PCR_RES_NUM_OBJ]);
        rx_cmd_fifo = (u32*) fifo;
        err = ixx_act_ib_xxx_rcv_cmd(intf, rx_cmd_fifo, &start_req.dal_req,
                        &start_res.dal_res);
        if (err)
                return err;

        error: mutex_unlock(&intf->cmd_lock);

        return err;
}

static int ixx_act_ib_xxx_restart(struct ixx_pci_priv *priv)
{
        priv->restart_flag = 1;
        wake_up_interruptible(&priv->wait_queue);
        return 0;
}


static int ixx_act_ib_xxx_dma_get_info(struct ixx_pci_interface* ixx_dev,
                u8* map_pass_thru_bits, u32* page_size, u16* page_count)
{
        u32* __iomem adr_trans;
        u32* __iomem adr_trans_first;
        u16 entry_size = 0;
        u16 max_entry_num = 0;
        u16 entry = 0;
        u32 adr_trans_start = 0;
        u32 adr_trans_end = 0;
        u32 adr_lo = 0;
        u32 val = 0;

        *map_pass_thru_bits = 0;

        entry_size = sizeof(u32) * 2;
        adr_trans_start = CAN_IB2X0_PCIE_ALTERA_LCR_ADRTRANSSTART_OFF;
        adr_trans_end = CAN_IB2X0_PCIE_ALTERA_LCR_ADRTRANSEND_OFF - entry_size;
        max_entry_num = (u16)((adr_trans_end - adr_trans_start) / entry_size);

        adr_trans_first = (u32*) (ixx_dev->reg1vadd + adr_trans_start);

        iowrite32(cpu_to_le32(0xFFFFFFFC), adr_trans_first);
        iowrite32(cpu_to_le32(0xFFFFFFFF), adr_trans_first + 1);

        adr_lo = ioread32(adr_trans_first);
        *page_size = ~adr_lo + 1;
        val = ~adr_lo;

        while (0x01 == (val & 0x01)) {
                (*map_pass_thru_bits)++;
                val = (val >> 1);
        }

        *page_count = max_entry_num;
        for (entry = 1; entry < max_entry_num; entry++) {
                adr_trans = (u32*) (ixx_dev->reg1vadd + adr_trans_start
                                + entry * entry_size);

                if (ioread32(adr_trans + 1) != 0) {
                        *page_count = entry;
                        break;
                }

                iowrite32(0, adr_trans);
                iowrite32(0, adr_trans + 1);
        }

        iowrite32(0, adr_trans_first);
        iowrite32(0, adr_trans_first + 1);

        return 0;
}


static int ixx_act_ib_xxx_mc_reset(struct ixx_pci_interface* ixx_dev)
{
        u32* __iomem reset;
        u32 period = CAN_IB2X0_PCIE_RESET_PERIOD / 100; //Reset hold time in 100 us
        u8 reset_done = 0;

        reset = (u32*) (ixx_dev->reg1vadd + CAN_IB2X0_PCIE_ALTERA_LCR_RESET_CARD_OFF);

        //*** Reset Device
        iowrite32(cpu_to_le32(le32_to_cpu(ioread32(reset)) | CAN_IB2X0_PCIE_RESET),
                        reset);

        //*** Hold reset up to reset hold time
        do {
                //*** Wait 100 us
                usleep_range(99, 100);
                reset_done = le32_to_cpu(ioread32(reset)) & CAN_IB2X0_PCIE_RESET;
        } while (reset_done && period--);

        if (reset_done)
                iowrite32(
                                cpu_to_le32(
                                                le32_to_cpu(
                                                                ioread32(
                                                                                reset)) & ~CAN_IB2X0_PCIE_RESET),
                                reset);

        //*** Delay after reset in us
        msleep(CAN_IB2X0_PCIE_RESET_DELAY / 1000);

        return 0;
}


static int ixx_act_ib_xxx_convey_dma(struct pci_dev *pdev,
                struct ixx_pci_interface * intf, u8 enable)
{
        int i;
        u32 __iomem *lcr_adr_trans;
        u8 map_pass_thru_bits = 0, err;
        u32 page_size = 0;
        u16 page_count = 0;
        u32 dma_high_addr;
        u32 dma_low_addr;
        dma_addr_t dma_phys_addr = intf->dmaadd;

        lcr_adr_trans = (u32*) (intf->reg1vadd
                        + CAN_IB2X0_PCIE_ALTERA_LCR_ADRTRANSSTART_OFF);

        ixx_act_ib_xxx_mc_reset(intf);

        err = ixx_act_ib_xxx_dma_get_info(intf, &map_pass_thru_bits, &page_size,
                        &page_count);
        if (err)
                return err;

        if (enable)
                page_count = min(page_count,
                                (u16)(CAN_IB200_DMA_SIZE / page_size));
        else
                dma_phys_addr = 0;

        for (i = 0; i < page_count; i++) {
                dma_high_addr = dma_phys_addr >> 32;
                dma_low_addr = (dma_phys_addr & 0xFFFFFFFC);

                iowrite32(dma_low_addr, lcr_adr_trans);
                lcr_adr_trans++;
                iowrite32(dma_high_addr, lcr_adr_trans);
                lcr_adr_trans++;

                if (enable) {
                        dma_phys_addr += page_size;
                }
        }

        return 0;
}

/*
 * describe the IXXAT CAN-IB200 PCI adapter
 */
struct ixx_pci_adapter can_ib_200 = {
                .name = "CAN-IB200/PCIe",
                .device_id = CAN_IB200_DEVICE_ID,
                .clock = {
                                .freq = IFI_HZ,
                },
                .bittiming_const = {
                                .name = "ib_200",
                                .tseg1_min = 1,
                                .tseg1_max = 32,
                                .tseg2_min = 2,
                                .tseg2_max = 33,
                                .sjw_max = 4,
                                .brp_min = 2,
                                .brp_max = 257,
                                .brp_inc = 1,
                },

                .ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
                CAN_CTRLMODE_ONE_SHOT |
                CAN_CTRLMODE_LISTENONLY |
                CAN_CTRLMODE_LOOPBACK |
                CAN_CTRLMODE_BERR_REPORTING,

                /* size of device private data */
                .sizeof_dev_private = sizeof(struct ixx_pci_priv),

                /* device callbacks */
                .intf_probe = ixx_act_ib_xxx_probe,
                .intf_convey_dma = ixx_act_ib_xxx_convey_dma,
                .intf_mc_reset = ixx_act_ib_xxx_mc_reset,
                .intf_int_ena = ixx_act_ib_xxx_int_ena_req,
                .intf_test_cmd = ixx_act_ib_xxx_test_cmd,
                .intf_test_dma = ixx_act_ib_xxx_test_dma,
                .intf_get_caps = ixx_act_ib_xxx_get_intf_caps,
                .intf_upload_firmware = ixx_act_ib_xxx_upload_firmware,
                .intf_trigger_int     = ixx_act_ib_xxx_trigger_int,
                .intf_start_fw = ixx_act_ib_xxx_start_fw,
                .dev_init = ixx_act_ib_xxx_init,
                .dev_exit = ixx_act_ib_xxx_exit,
                .dev_free = ixx_act_ib_xxx_free,
                .intf_get_info = ixx_act_ib_xxx_get_dev_info,
                .dev_start = ixx_act_ib_xxx_start,
                .dev_stop = ixx_act_ib_xxx_stop,
                .dev_restart = ixx_act_ib_xxx_restart,
                .dev_start_xmit = ixx_act_ib_xxx_start_xmit,
                .dev_napi_rx_poll = ixx_act_ib_xxx_napi_rx_poll,
                .handle_msg = ixx_act_ib_xxx_handle_ifi_msg,
                .handle_vci_msg = ixx_act_ib_xxx_handle_vci_msg,
};

/*
 * describe the IXXAT CAN-IB210/XMC adapter
 */
struct ixx_pci_adapter can_ib_210 = {
                .name = "CAN-IB210/XMC",
                .device_id = CAN_IB210_DEVICE_ID,
                .clock = {
                                .freq = IFI_HZ,
                },
                .bittiming_const = {
                                .name = "ib_210",
                                .tseg1_min = 1,
                                .tseg1_max = 32,
                                .tseg2_min = 2,
                                .tseg2_max = 33,
                                .sjw_max = 4,
                                .brp_min = 2,
                                .brp_max = 257,
                                .brp_inc = 1,
                },

                .ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
                CAN_CTRLMODE_ONE_SHOT |
                CAN_CTRLMODE_LISTENONLY |
                CAN_CTRLMODE_LOOPBACK |
                CAN_CTRLMODE_BERR_REPORTING,

                /* size of device private data */
                .sizeof_dev_private = sizeof(struct ixx_pci_priv),

                /* device callbacks */
                .intf_probe = ixx_act_ib_xxx_probe,
                .intf_convey_dma = ixx_act_ib_xxx_convey_dma,
                .intf_mc_reset = ixx_act_ib_xxx_mc_reset,
                .intf_int_ena = ixx_act_ib_xxx_int_ena_req,
                .intf_test_cmd = ixx_act_ib_xxx_test_cmd,
                .intf_test_dma = ixx_act_ib_xxx_test_dma,
                .intf_get_caps = ixx_act_ib_xxx_get_intf_caps,
                .intf_upload_firmware = ixx_act_ib_xxx_upload_firmware,
                .intf_trigger_int     = ixx_act_ib_xxx_trigger_int,
                .intf_start_fw = ixx_act_ib_xxx_start_fw,
                .dev_init = ixx_act_ib_xxx_init,
                .dev_exit = ixx_act_ib_xxx_exit,
                .dev_free = ixx_act_ib_xxx_free,
                .intf_get_info = ixx_act_ib_xxx_get_dev_info,
                .dev_start = ixx_act_ib_xxx_start,
                .dev_stop = ixx_act_ib_xxx_stop,
                .dev_restart = ixx_act_ib_xxx_restart,
                .dev_start_xmit = ixx_act_ib_xxx_start_xmit,
                .dev_napi_rx_poll = ixx_act_ib_xxx_napi_rx_poll,
                .handle_msg = ixx_act_ib_xxx_handle_ifi_msg,
                .handle_vci_msg = ixx_act_ib_xxx_handle_vci_msg,
};

/*
 * describe the IXXAT CAN-IB230/104 Mini adapter
 */
struct ixx_pci_adapter can_ib_230 = {
                .name = "CAN-IB230/PCIe 104",
                .device_id = CAN_IB230_DEVICE_ID,
                .clock = {
                                .freq = IFI_HZ,
                },
                .bittiming_const = {
                                .name = "ib_230",
                                .tseg1_min = 1,
                                .tseg1_max = 32,
                                .tseg2_min = 2,
                                .tseg2_max = 33,
                                .sjw_max = 4,
                                .brp_min = 2,
                                .brp_max = 257,
                                .brp_inc = 1,
                },

                .ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
                CAN_CTRLMODE_ONE_SHOT |
                CAN_CTRLMODE_LISTENONLY |
                CAN_CTRLMODE_LOOPBACK |
                CAN_CTRLMODE_BERR_REPORTING,

                /* size of device private data */
                .sizeof_dev_private = sizeof(struct ixx_pci_priv),

                /* device callbacks */
                .intf_probe = ixx_act_ib_xxx_probe,
                .intf_convey_dma = ixx_act_ib_xxx_convey_dma,
                .intf_mc_reset = ixx_act_ib_xxx_mc_reset,
                .intf_int_ena = ixx_act_ib_xxx_int_ena_req,
                .intf_test_cmd = ixx_act_ib_xxx_test_cmd,
                .intf_test_dma = ixx_act_ib_xxx_test_dma,
                .intf_get_caps = ixx_act_ib_xxx_get_intf_caps,
                .intf_upload_firmware = ixx_act_ib_xxx_upload_firmware,
                .intf_trigger_int     = ixx_act_ib_xxx_trigger_int,
                .intf_start_fw = ixx_act_ib_xxx_start_fw,
                .dev_init = ixx_act_ib_xxx_init,
                .dev_exit = ixx_act_ib_xxx_exit,
                .dev_free = ixx_act_ib_xxx_free,
                .intf_get_info = ixx_act_ib_xxx_get_dev_info,
                .dev_start = ixx_act_ib_xxx_start,
                .dev_stop = ixx_act_ib_xxx_stop,
                .dev_restart = ixx_act_ib_xxx_restart,
                .dev_start_xmit = ixx_act_ib_xxx_start_xmit,
                .dev_napi_rx_poll = ixx_act_ib_xxx_napi_rx_poll,
                .handle_msg = ixx_act_ib_xxx_handle_ifi_msg,
                .handle_vci_msg = ixx_act_ib_xxx_handle_vci_msg,
};

/*
 * describe the IXXAT CAN-IB400/PCI adapter
 */
struct ixx_pci_adapter can_ib_400 = {
                .name = "CAN-IB400/PCI",
                .device_id = CAN_IB400_DEVICE_ID,
                .clock = {
                                .freq = IFI_HZ,
                },
                .bittiming_const = {
                                .name = "ib_400",
                                .tseg1_min = 1,
                                .tseg1_max = 32,
                                .tseg2_min = 2,
                                .tseg2_max = 33,
                                .sjw_max = 4,
                                .brp_min = 2,
                                .brp_max = 257,
                                .brp_inc = 1,
                },

                .ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
                CAN_CTRLMODE_ONE_SHOT |
                CAN_CTRLMODE_LISTENONLY |
                CAN_CTRLMODE_LOOPBACK |
                CAN_CTRLMODE_BERR_REPORTING,

                /* size of device private data */
                .sizeof_dev_private = sizeof(struct ixx_pci_priv),

                /* device callbacks */
                .intf_probe = ixx_act_ib_xxx_probe,
                .intf_convey_dma = ixx_act_ib_xxx_convey_dma,
                .intf_mc_reset = ixx_act_ib_xxx_mc_reset,
                .intf_int_ena = ixx_act_ib_xxx_int_ena_req,
                .intf_test_cmd = ixx_act_ib_xxx_test_cmd,
                .intf_test_dma = ixx_act_ib_xxx_test_dma,
                .intf_get_caps = ixx_act_ib_xxx_get_intf_caps,
                .intf_upload_firmware = ixx_act_ib_xxx_upload_firmware,
                .intf_trigger_int     = ixx_act_ib_xxx_trigger_int,
                .intf_start_fw = ixx_act_ib_xxx_start_fw,
                .dev_init = ixx_act_ib_xxx_init,
                .dev_exit = ixx_act_ib_xxx_exit,
                .dev_free = ixx_act_ib_xxx_free,
                .intf_get_info = ixx_act_ib_xxx_get_dev_info,
                .dev_start = ixx_act_ib_xxx_start,
                .dev_stop = ixx_act_ib_xxx_stop,
                .dev_restart = ixx_act_ib_xxx_restart,
                .dev_start_xmit = ixx_act_ib_xxx_start_xmit,
                .dev_napi_rx_poll = ixx_act_ib_xxx_napi_rx_poll,
                .handle_msg = ixx_act_ib_xxx_handle_ifi_msg,
                .handle_vci_msg = ixx_act_ib_xxx_handle_vci_msg,
};

/*
 * describe the IXXAT CAN-IB410/PMC adapter
 */
struct ixx_pci_adapter can_ib_410 = {
                .name = "CAN-IB410/PMC",
                .device_id = CAN_IB410_DEVICE_ID,
                .clock = {
                                .freq = IFI_HZ,
                },
                .bittiming_const = {
                                .name = "ib_410",
                                .tseg1_min = 1,
                                .tseg1_max = 32,
                                .tseg2_min = 2,
                                .tseg2_max = 33,
                                .sjw_max = 4,
                                .brp_min = 2,
                                .brp_max = 257,
                                .brp_inc = 1,
                },

                .ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
                CAN_CTRLMODE_ONE_SHOT |
                CAN_CTRLMODE_LISTENONLY |
                CAN_CTRLMODE_LOOPBACK |
                CAN_CTRLMODE_BERR_REPORTING,

                /* size of device private data */
                .sizeof_dev_private = sizeof(struct ixx_pci_priv),

                /* device callbacks */
                .intf_probe = ixx_act_ib_xxx_probe,
                .intf_convey_dma = ixx_act_ib_xxx_convey_dma,
                .intf_mc_reset = ixx_act_ib_xxx_mc_reset,
                .intf_int_ena = ixx_act_ib_xxx_int_ena_req,
                .intf_test_cmd = ixx_act_ib_xxx_test_cmd,
                .intf_test_dma = ixx_act_ib_xxx_test_dma,
                .intf_get_caps = ixx_act_ib_xxx_get_intf_caps,
                .intf_upload_firmware = ixx_act_ib_xxx_upload_firmware,
                .intf_trigger_int     = ixx_act_ib_xxx_trigger_int,
                .intf_start_fw = ixx_act_ib_xxx_start_fw,
                .dev_init = ixx_act_ib_xxx_init,
                .dev_exit = ixx_act_ib_xxx_exit,
                .dev_free = ixx_act_ib_xxx_free,
                .intf_get_info = ixx_act_ib_xxx_get_dev_info,
                .dev_start = ixx_act_ib_xxx_start,
                .dev_stop = ixx_act_ib_xxx_stop,
                .dev_restart = ixx_act_ib_xxx_restart,
                .dev_start_xmit = ixx_act_ib_xxx_start_xmit,
                .dev_napi_rx_poll = ixx_act_ib_xxx_napi_rx_poll,
                .handle_msg = ixx_act_ib_xxx_handle_ifi_msg,
                .handle_vci_msg = ixx_act_ib_xxx_handle_vci_msg,
};
#ifdef CANFD_CAPABLE
/*
 * describe the IXXAT CAN-IB600/PCIe FD adapter
 */
struct ixx_pci_adapter can_ib_600 = {
                .name = "CAN-IB600/PCIe FD",
                .device_id = CAN_IB600_DEVICE_ID,
                .clock = {
                                .freq = IFI_HZ,
                },
                .bittiming_const = {
                                .name = "ib_600",
                                .tseg1_min = 1,
                                .tseg1_max = 256,
                                .tseg2_min = 1,
                                .tseg2_max = 256,
                                .sjw_max = 128,
                                .brp_min = 2,
                                .brp_max = 513,
                                .brp_inc = 1,
                },
                .data_bittiming_const = {
                                .name = "ib_600",
                                .tseg1_min = 1,
                                .tseg1_max = 256,
                                .tseg2_min = 1,
                                .tseg2_max = 256,
                                .sjw_max = 128,
                                .brp_min = 2,
                                .brp_max = 513,
                                .brp_inc = 1,
                },

                .ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
                CAN_CTRLMODE_ONE_SHOT |
                CAN_CTRLMODE_LISTENONLY |
                CAN_CTRLMODE_LOOPBACK |
                CAN_CTRLMODE_BERR_REPORTING |
                CAN_CTRLMODE_FD |
                CAN_CTRLMODE_FD_NON_ISO,

                /* size of device private data */
                .sizeof_dev_private = sizeof(struct ixx_pci_priv),

                /* device callbacks */
                .intf_probe = ixx_act_ib_xxx_probe,
                .intf_convey_dma = ixx_act_ib_xxx_convey_dma,
                .intf_mc_reset = ixx_act_ib_xxx_mc_reset,
                .intf_int_ena = ixx_act_ib_xxx_int_ena_req,
                .intf_test_cmd = ixx_act_ib_xxx_test_cmd,
                .intf_test_dma = ixx_act_ib_xxx_test_dma,
                .intf_get_caps = ixx_act_ib_xxx_get_intf_caps,
                .intf_upload_firmware = ixx_act_ib_xxx_upload_firmware,
                .intf_trigger_int     = ixx_act_ib_xxx_trigger_int,
                .intf_start_fw = ixx_act_ib_xxx_start_fw,
                .dev_init = ixx_act_ib_xxx_init,
                .dev_exit = ixx_act_ib_xxx_exit,
                .dev_free = ixx_act_ib_xxx_free,
                .intf_get_info = ixx_act_ib_xxx_get_dev_info,
                .dev_start = ixx_act_ib_xxx_start,
                .dev_stop = ixx_act_ib_xxx_stop,
                .dev_restart = ixx_act_ib_xxx_restart,
                .dev_start_xmit = ixx_act_ib_xxx_start_xmit_fd,
                .dev_napi_rx_poll = ixx_act_ib_xxx_napi_rx_poll,
                .handle_msg = ixx_act_ib_xxx_handle_ififd_msg,
                .handle_vci_msg = ixx_act_ib_xxx_handle_vci_fd_msg,
};

/*
 * describe the IXXAT CAN-IB610/XMC FD adapter
 */
struct ixx_pci_adapter can_ib_610 = {
                .name = "CAN-IB610/XMC FD",
                .device_id = CAN_IB610_DEVICE_ID,
                .clock = {
                                .freq = IFI_HZ,
                },
                .bittiming_const = {
                                .name = "ib_610",
                                .tseg1_min = 1,
                                .tseg1_max = 256,
                                .tseg2_min = 1,
                                .tseg2_max = 256,
                                .sjw_max = 128,
                                .brp_min = 2,
                                .brp_max = 513,
                                .brp_inc = 1,
                },
                .data_bittiming_const = {
                                .name = "ib_610",
                                .tseg1_min = 1,
                                .tseg1_max = 256,
                                .tseg2_min = 1,
                                .tseg2_max = 256,
                                .sjw_max = 128,
                                .brp_min = 2,
                                .brp_max = 513,
                                .brp_inc = 1,
                },

                .ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
                CAN_CTRLMODE_ONE_SHOT |
                CAN_CTRLMODE_LISTENONLY |
                CAN_CTRLMODE_LOOPBACK |
                CAN_CTRLMODE_BERR_REPORTING |
                CAN_CTRLMODE_FD |
                CAN_CTRLMODE_FD_NON_ISO,

                /* size of device private data */
                .sizeof_dev_private = sizeof(struct ixx_pci_priv),

                /* device callbacks */
                .intf_probe = ixx_act_ib_xxx_probe,
                .intf_convey_dma = ixx_act_ib_xxx_convey_dma,
                .intf_mc_reset = ixx_act_ib_xxx_mc_reset,
                .intf_int_ena = ixx_act_ib_xxx_int_ena_req,
                .intf_test_cmd = ixx_act_ib_xxx_test_cmd,
                .intf_test_dma = ixx_act_ib_xxx_test_dma,
                .intf_get_caps = ixx_act_ib_xxx_get_intf_caps,
                .intf_upload_firmware = ixx_act_ib_xxx_upload_firmware,
                .intf_trigger_int     = ixx_act_ib_xxx_trigger_int,
                .intf_start_fw = ixx_act_ib_xxx_start_fw,
                .dev_init = ixx_act_ib_xxx_init,
                .dev_exit = ixx_act_ib_xxx_exit,
                .dev_free = ixx_act_ib_xxx_free,
                .intf_get_info = ixx_act_ib_xxx_get_dev_info,
                .dev_start = ixx_act_ib_xxx_start,
                .dev_stop = ixx_act_ib_xxx_stop,
                .dev_restart = ixx_act_ib_xxx_restart,
                .dev_start_xmit = ixx_act_ib_xxx_start_xmit_fd,
                .dev_napi_rx_poll = ixx_act_ib_xxx_napi_rx_poll,
                .handle_msg = ixx_act_ib_xxx_handle_ififd_msg,
                .handle_vci_msg = ixx_act_ib_xxx_handle_vci_fd_msg,
};

/*
 * describe the IXXAT CAN-IB800/PCI FD adapter
 */
struct ixx_pci_adapter can_ib_800 = {
                .name = "CAN-IB800/PCI FD",
                .device_id = CAN_IB800_DEVICE_ID,
                .clock = {
                                .freq = IFI_HZ,
                },
                .bittiming_const = {
                                .name = "ib_800",
                                .tseg1_min = 1,
                                .tseg1_max = 256,
                                .tseg2_min = 1,
                                .tseg2_max = 256,
                                .sjw_max = 128,
                                .brp_min = 2,
                                .brp_max = 513,
                                .brp_inc = 1,
                },
                .data_bittiming_const = {
                                .name = "ib_800",
                                .tseg1_min = 1,
                                .tseg1_max = 256,
                                .tseg2_min = 1,
                                .tseg2_max = 256,
                                .sjw_max = 128,
                                .brp_min = 2,
                                .brp_max = 513,
                                .brp_inc = 1,
                },

                .ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
                CAN_CTRLMODE_ONE_SHOT |
                CAN_CTRLMODE_LISTENONLY |
                CAN_CTRLMODE_LOOPBACK |
                CAN_CTRLMODE_BERR_REPORTING |
                CAN_CTRLMODE_FD |
                CAN_CTRLMODE_FD_NON_ISO,

                /* size of device private data */
                .sizeof_dev_private = sizeof(struct ixx_pci_priv),

                /* device callbacks */
                .intf_probe = ixx_act_ib_xxx_probe,
                .intf_convey_dma = ixx_act_ib_xxx_convey_dma,
                .intf_mc_reset = ixx_act_ib_xxx_mc_reset,
                .intf_int_ena = ixx_act_ib_xxx_int_ena_req,
                .intf_test_cmd = ixx_act_ib_xxx_test_cmd,
                .intf_test_dma = ixx_act_ib_xxx_test_dma,
                .intf_get_caps = ixx_act_ib_xxx_get_intf_caps,
                .intf_upload_firmware = ixx_act_ib_xxx_upload_firmware,
                .intf_trigger_int     = ixx_act_ib_xxx_trigger_int,
                .intf_start_fw = ixx_act_ib_xxx_start_fw,
                .dev_init = ixx_act_ib_xxx_init,
                .dev_exit = ixx_act_ib_xxx_exit,
                .dev_free = ixx_act_ib_xxx_free,
                .intf_get_info = ixx_act_ib_xxx_get_dev_info,
                .dev_start = ixx_act_ib_xxx_start,
                .dev_stop = ixx_act_ib_xxx_stop,
                .dev_restart = ixx_act_ib_xxx_restart,
                .dev_start_xmit = ixx_act_ib_xxx_start_xmit_fd,
                .dev_napi_rx_poll = ixx_act_ib_xxx_napi_rx_poll,
                .handle_msg = ixx_act_ib_xxx_handle_ififd_msg,
                .handle_vci_msg = ixx_act_ib_xxx_handle_vci_fd_msg,
};

/*
 * describe the IXXAT CAN-IB810/PMC FD adapter
 */
struct ixx_pci_adapter can_ib_810 = {
                .name = "CAN-IB810/PMC FD",
                .device_id = CAN_IB810_DEVICE_ID,
                .clock = {
                                .freq = IFI_HZ,
                },
                .bittiming_const = {
                                .name = "ib_810",
                                .tseg1_min = 1,
                                .tseg1_max = 256,
                                .tseg2_min = 1,
                                .tseg2_max = 256,
                                .sjw_max = 128,
                                .brp_min = 2,
                                .brp_max = 513,
                                .brp_inc = 1,
                },
                .data_bittiming_const = {
                                .name = "ib_810",
                                .tseg1_min = 1,
                                .tseg1_max = 256,
                                .tseg2_min = 1,
                                .tseg2_max = 256,
                                .sjw_max = 128,
                                .brp_min = 2,
                                .brp_max = 513,
                                .brp_inc = 1,
                },

                .ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
                CAN_CTRLMODE_ONE_SHOT |
                CAN_CTRLMODE_LISTENONLY |
                CAN_CTRLMODE_LOOPBACK |
                CAN_CTRLMODE_BERR_REPORTING |
                CAN_CTRLMODE_FD |
                CAN_CTRLMODE_FD_NON_ISO,

                /* size of device private data */
                .sizeof_dev_private = sizeof(struct ixx_pci_priv),

                /* device callbacks */
                .intf_probe = ixx_act_ib_xxx_probe,
                .intf_convey_dma = ixx_act_ib_xxx_convey_dma,
                .intf_mc_reset = ixx_act_ib_xxx_mc_reset,
                .intf_int_ena = ixx_act_ib_xxx_int_ena_req,
                .intf_test_cmd = ixx_act_ib_xxx_test_cmd,
                .intf_test_dma = ixx_act_ib_xxx_test_dma,
                .intf_get_caps = ixx_act_ib_xxx_get_intf_caps,
                .intf_upload_firmware = ixx_act_ib_xxx_upload_firmware,
                .intf_trigger_int     = ixx_act_ib_xxx_trigger_int,
                .intf_start_fw = ixx_act_ib_xxx_start_fw,
                .dev_init = ixx_act_ib_xxx_init,
                .dev_exit = ixx_act_ib_xxx_exit,
                .dev_free = ixx_act_ib_xxx_free,
                .intf_get_info = ixx_act_ib_xxx_get_dev_info,
                .dev_start = ixx_act_ib_xxx_start,
                .dev_stop = ixx_act_ib_xxx_stop,
                .dev_restart = ixx_act_ib_xxx_restart,
                .dev_start_xmit = ixx_act_ib_xxx_start_xmit_fd,
                .dev_napi_rx_poll = ixx_act_ib_xxx_napi_rx_poll,
                .handle_msg = ixx_act_ib_xxx_handle_ififd_msg,
                .handle_vci_msg = ixx_act_ib_xxx_handle_vci_fd_msg,
};
#endif
