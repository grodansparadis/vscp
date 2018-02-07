/*
 * CAN driver for IXXAT CAN-IBXXX adapters
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

#ifndef IXX_PCI_CORE_H
#define IXX_PCI_CORE_H

#include <linux/version.h>

#define _REAL_LOOPBACK 1

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 3)
#define CANFD_CAPABLE 1
#endif

#define CAN_IB_IFI_CAN_BASE          0x2000
#define CAN_IB_IFI_CAN_SIZE          0x1000

#define CAN_IB_BSP_FPGA_SCF_NCS      0
#define CAN_IB_BSP_FPGA_SCF_CLK      1
#define CAN_IB_BSP_FPGA_SCF_SDI      2
#define CAN_IB_BSP_FPGA_SCF_SDO      3

#define CAN_IB_EPCS_READ_BYTES       0x03

/* Sync I/O */
#define CAN_IB_SYNCIO_BASE           0x0060

/* FPGA Version */
#define CAN_IB_FPGAVER_BASE          0x0020

/* Hardware Version */
#define CAN_IB_HWVER_BASE            0x0010

/* Board Name */
#define CAN_IB_CARDNAME_BASE         0x0000
#define CAN_IB_CARDNAME_SIZE         0x0010

#define CAN_IB_HWSERIAL_SIZE         0x0010

#define CAN_IB_INFO_SECTOR           31
#define CAN_IB_INFO_SECTOR_SIZE      0x10000
#define CAN_IB_INFO_OFFSET_SERIAL    0

/* GPIO SPI */
#define CAN_IB_GPIOSPI_BASE          0x0080

/* Variant coding */
#define CAN_IB_VARIANT_BASE          0x1000
#define CAN_IB_VARIANT_STEP          0x0100
#define CAN_IB_VARIANT_COUNT         0x0004

/* Variant encoding */
#define CAN_IB_VC_CANHS              1  /*  BIT 0 CAN HS present */
/* BIT 1..5, inverted */
/* 0x1F = no variant! */
#define CAN_IB_VC_MASK               0x001F  /* variant bit mask */
#define CAN_IB_VC_CANLS              2  /* CAN LowSpeed extension board installed */
#define CAN_IB_VC_CANLS_LIN          4  /* CAN LowSpeed / K-Line extension board installed */
#define CAN_IB_VC_CANLS_KLINE        5  /* CAN LowSpeed / LIN extension board installed */

/* SYNC I/O bit encoding */
#define CAN_IB_SIO_EXPANSION         0x0004  /* Flag if 2 CAN expansion board is attached */

/* Vendor and Sub Vendor ID of CAN-IB1x0/PCIe Card */
#define CAN_IB1X0_VENDOR_ID                 0x1BEE
#define CAN_IB1X0_SUB_VENDOR_ID             0x1BEE

/* Device and Sub Device ID of CAN-IB100/PCIe Card */
#define CAN_IB100_DEVICE_ID                 0x0002
#define CAN_IB100_SUB_DEVICE_ID             0x0002

/* Device and Sub Device ID of CAN-IB110/XMC Card */
#define CAN_IB110_DEVICE_ID                 0x0013
#define CAN_IB110_SUB_DEVICE_ID             0x0013

/* Device and Sub Device ID of CAN-IB120/PCIe Mini Card */
#define CAN_IB120_DEVICE_ID                 0x0004
#define CAN_IB120_SUB_DEVICE_ID             0x0004

/* Device and Sub Device ID of CAN-IB130/PCIe 104 Card */
#define CAN_IB130_DEVICE_ID                 0x0005
#define CAN_IB130_SUB_DEVICE_ID             0x0005

/* Device and Sub Device ID of CAN-IB300/PCI Card */
#define CAN_IB300_DEVICE_ID                 0x0010
#define CAN_IB300_SUB_DEVICE_ID             0x0010

/* Device and Sub Device ID of CAN-IB310/PMC Card */
#define CAN_IB310_DEVICE_ID                 0x0015
#define CAN_IB310_SUB_DEVICE_ID             0x0015

/* Vendor and Sub Vendor ID of CAN-IB500/PCIe Card FD */
#define CAN_IB500_DEVICE_ID                 0x000e
#define CAN_IB500_SUB_DEVICE_ID             0x000e

/* Vendor and Sub Vendor ID of CAN-IB510/XMC Card FD */
#define CAN_IB510_DEVICE_ID                 0x0017
#define CAN_IB510_SUB_DEVICE_ID             0x0017

/* Vendor and Sub Vendor ID of CAN-IB520/PCIe Mini Card FD */
#define CAN_IB520_DEVICE_ID                 0x0012
#define CAN_IB520_SUB_DEVICE_ID             0x0012

/* Vendor and Sub Vendor ID of CAN-IB700/PCI Card FD */
#define CAN_IB700_DEVICE_ID                 0x0019
#define CAN_IB700_SUB_DEVICE_ID             0x0019

/* Vendor and Sub Vendor ID of CAN-IB710/PMC Card FD */
#define CAN_IB710_DEVICE_ID                 0x001A
#define CAN_IB710_SUB_DEVICE_ID             0x001A

/* Vendor and Sub Vendor ID of CAN-IB200/PCIe Card */
#define CAN_IB2X0_VENDOR_ID                 0x1BEE
#define CAN_IB2X0_SUB_VENDOR_ID             0x1BEE

/* Device and Sub Device ID of CAN-IB200/PCIe Card */
#define CAN_IB200_DEVICE_ID                 0x0003
#define CAN_IB200_SUB_DEVICE_ID             0x0003

/* Device and Sub Device ID of CAN-IB210/XMC Card */
#define CAN_IB210_DEVICE_ID                 0x0014
#define CAN_IB210_SUB_DEVICE_ID             0x0014

/* Device and Sub Device ID of CAN-IB230/PCIe 104 Card */
#define CAN_IB230_DEVICE_ID                 0x0006
#define CAN_IB230_SUB_DEVICE_ID             0x0006

/* Device and Sub Device ID of CAN-IB400/PCI Card */
#define CAN_IB400_DEVICE_ID                 0x0011
#define CAN_IB400_SUB_DEVICE_ID             0x0011

/* Device and Sub Device ID of CAN-IB410/PMC Card */
#define CAN_IB410_DEVICE_ID                 0x0016
#define CAN_IB410_SUB_DEVICE_ID             0x0016

/* Vendor and Sub Vendor ID of CAN-IB600/PCIe Card FD */
#define CAN_IB600_DEVICE_ID                 0x000f
#define CAN_IB600_SUB_DEVICE_ID             0x000f

/* Vendor and Sub Vendor ID of CAN-IB600/XMC Card FD */
#define CAN_IB610_DEVICE_ID                 0x0018
#define CAN_IB610_SUB_DEVICE_ID             0x0018

/* Vendor and Sub Vendor ID of CAN-IB800/PCIe Card FD */
#define CAN_IB800_DEVICE_ID                 0x001b
#define CAN_IB800_SUB_DEVICE_ID             0x001b

/* Vendor and Sub Vendor ID of CAN-IB800/PMC Card FD */
#define CAN_IB810_DEVICE_ID                 0x001c
#define CAN_IB810_SUB_DEVICE_ID             0x001c

#define IXX_STATE_RUNNING    0x01
#define IXX_STATE_STOPPED    0x02

#define PCIE_ALTERA_LCR_INTCSR              0x0040
#define PCIE_ALTERALCR_A2P_INTENA           0x0050

#define IXX_PCI_MAX_TX_TRANS    0x10

#define IXX_PCI_BUS_CAN  1

#define IXX_BTMODE_NAT 0x01

/* passive interfaces */
extern struct ixx_pci_adapter can_ib_100;
extern struct ixx_pci_adapter can_ib_110;
extern struct ixx_pci_adapter can_ib_120;
extern struct ixx_pci_adapter can_ib_130;
extern struct ixx_pci_adapter can_ib_300;
extern struct ixx_pci_adapter can_ib_310;
#ifdef CANFD_CAPABLE
extern struct ixx_pci_adapter can_ib_500;
extern struct ixx_pci_adapter can_ib_510;
extern struct ixx_pci_adapter can_ib_520;
extern struct ixx_pci_adapter can_ib_700;
extern struct ixx_pci_adapter can_ib_710;
#endif

/* active interfaces */
extern struct ixx_pci_adapter can_ib_200;
extern struct ixx_pci_adapter can_ib_210;
extern struct ixx_pci_adapter can_ib_230;
extern struct ixx_pci_adapter can_ib_400;
extern struct ixx_pci_adapter can_ib_410;
#ifdef CANFD_CAPABLE
extern struct ixx_pci_adapter can_ib_600;
extern struct ixx_pci_adapter can_ib_610;
extern struct ixx_pci_adapter can_ib_800;
extern struct ixx_pci_adapter can_ib_810;
#endif

struct ixx_intf_info {
        char intf_name[CAN_IB_CARDNAME_SIZE]; /* device name */
        char intf_id[CAN_IB_HWSERIAL_SIZE]; /* device identification ( unique device id) */
        u16 intf_version; /* device version ( 0, 1, ...) */
        u32 intf_fpga_version; /* device version of FPGA design */
        u16 reserved;
} __packed;

struct ixx_intf_fw_info {
        u32 firmware_type; /* type of currently running firmware */
        u16 reserved; /* reserved */
        u16 major_version; /* major firmware version number */
        u16 minor_version; /* minor firmware version number */
        u16 build_version; /* build firmware version number */
} __packed;

struct ixx_intf_caps {
        u16 bus_ctrl_count;
        u16 bus_ctrl_types[32];
        u16 reserved;
} __packed;

struct ixx_can_msg {
        u32 time;
        u32 msg_id;
        u32 flags;
        u8 data[8];
} __packed;

struct ixx_canfd_msg {
        u32 time;
        u32 msg_id;
        u32 flags;
        u32 client_id;
        u8 data[8];
} __packed;

struct ixx_time_ref {
        struct timeval tv_host_0;
        u32 ts_dev_0;
        u32 ts_dev_last;
};

struct ixx_can_btp {
        u32 mode;
        u32 bps;
        u16 ts1;
        u16 ts2;
        u16 sjw;
        u16 tdo;
} __packed;

struct ixx_pci_priv;
struct ixx_pci_adapter;

struct ixx_pci_interface {
        char name[100];

        /* virtual and real memory pointers */
        /* Interface PCI(e) registers 1 */
        phys_addr_t reg1add;
        void __iomem *reg1vadd;
        unsigned long reg1len;
        /* Interface PCI(e) registers 2 */
        phys_addr_t reg2add;
        void __iomem *reg2vadd;
        unsigned long reg2len;
        /* Interface PCI(e) memory */
        phys_addr_t memadd;
        void __iomem *memvadd;
        unsigned long memlen;
        /* coherent DMA memory */
        dma_addr_t dmaadd;
        void *dmavadd;
        unsigned long dmalen;
        /* additional special purpose memory */
        void *addmemvadd;
        unsigned long addmemlen;

        /* user mode wake-up mechanism */
        wait_queue_head_t waitqueue;

        /* Device interrupt line*/
        unsigned int device_irq;

        u32 int_counter;
        struct ixx_intf_info dev_info;
        struct ixx_intf_fw_info fw_info;

        struct pci_dev *pdev;

        struct ixx_pci_priv *priv;
        int cm_ignore_count;
        int intf_ctrl_count;
        int intf_running_ctrl_count;

        u32 board_ready;

        u8 started_mask;
        u8 handle_data;

        int opened_ctrl_count;

        struct mutex cmd_lock;
};

struct ixx_pci_priv {
        struct can_priv can;
        struct ixx_pci_adapter *adapter;
        unsigned int ctrl_idx;
        u32 state;

        struct pci_dev *pdev;
        struct net_device *netdev;

        struct ixx_time_ref time_ref;

        struct ixx_pci_interface *intf;

        struct ixx_pci_priv *prev_siblings;
        struct ixx_pci_priv *next_siblings;

        u32 __iomem *ifi_base;
        u32 __iomem *tx_fifo;
        u32 __iomem *rx_fifo;

        u8 can_mode;
        u8 can_exmode;

        struct task_struct *restart_task;
        u8 restart_flag;
        u8 must_quit;
        wait_queue_head_t wait_queue;

        int ctrl_opened_count;

        struct napi_struct napi;
        u8 interrupts_enabled;
        u32 frn_read;
        u32 frn_write;

        u32 ifi_reg_shadow_status;
        spinlock_t rcv_lock;
};

/* ixxat pci adapter descriptor */
struct ixx_pci_adapter {
        char *name;
        u32 device_id;
        struct can_clock clock;
        const struct can_bittiming_const bittiming_const;
        const struct can_bittiming_const data_bittiming_const;
        unsigned int ctrl_count;
        u32 ctrlmode_supported;

        int (*intf_probe)(struct pci_dev *dev, struct ixx_pci_interface * intf);
        int (*intf_convey_dma)(struct pci_dev *dev,
                        struct ixx_pci_interface * intf, u8 enable);
        int (*intf_mc_reset)(struct ixx_pci_interface * intf);
        int (*intf_int_ena)(struct ixx_pci_interface * intf, u8 enable);
        int (*intf_test_dma)(struct pci_dev *dev,
                        struct ixx_pci_interface * intf);
        int (*intf_test_cmd)(struct pci_dev *dev,
                        struct ixx_pci_interface * intf);
        int (*intf_upload_firmware)(struct pci_dev *dev,
                        struct ixx_pci_interface * intf);
        int (*intf_trigger_int)(struct pci_dev *dev,
                        struct ixx_pci_interface * intf);
        int (*intf_start_fw)(struct pci_dev *dev,
                        struct ixx_pci_interface * intf);

        int (*intf_get_caps)(struct pci_dev *pdev,
                        struct ixx_pci_interface * intf,
                        struct ixx_intf_caps* intf_caps, u8 abs_idx);

        int (*dev_init)(struct ixx_pci_priv *dev);
        void (*dev_exit)(struct ixx_pci_priv *dev);
        void (*dev_free)(struct ixx_pci_priv *dev);
        int (*dev_open)(struct ixx_pci_priv *dev);
        int (*dev_close)(struct ixx_pci_priv *dev);
        int (*dev_set_bittiming)(struct ixx_pci_priv *dev,
                        struct can_bittiming *bt);
        int (*dev_set_datatiming)(struct ixx_pci_priv *dev,
                        struct can_bittiming *bt);
        int (*intf_get_info)(struct ixx_pci_interface* intf,
                        struct ixx_intf_info* intf_info);
        int (*intf_get_fw_info)(struct ixx_pci_interface* intf,
                        struct ixx_intf_fw_info* fw_info);
        int (*dev_start)(struct ixx_pci_priv *dev);
        int (*dev_stop)(struct ixx_pci_priv *dev);
        int (*dev_restart)(struct ixx_pci_priv *dev);

        int (*dev_start_xmit)(struct sk_buff *skb, struct net_device *netdev);
        int (*dev_napi_rx_poll)(struct napi_struct *napi, int quota);
        int (*handle_msg)(struct ixx_pci_priv *dev, u32 * ifi_base);

        int (*handle_vci_msg)(struct ixx_pci_priv *dev, u32* data);

        int sizeof_dev_private;
};

void ixxat_dump_mem(char *prompt, void *p, int l);

void ixxat_pci_update_ts_now(struct ixx_pci_priv *dev, u32 ts_now);
void ixxat_pci_set_ts_now(struct ixx_pci_priv *dev, u32 ts_now);
void ixxat_pci_get_ts_tv(struct ixx_pci_priv *dev, u32 ts, ktime_t *k_time);

int ixxat_pci_handle_frn(struct ixx_pci_priv *dev, u8 frn, u32 timestamp);
#endif
