/*
 * CAN driver for IXXAT USB-to-CAN V2 adapters
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
#ifndef IXX_USB_CORE_H
#define IXX_USB_CORE_H

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 3)
#define CANFD_CAPABLE 1
#endif

extern struct ixx_usb_adapter usb_to_can_v2_compact;
extern struct ixx_usb_adapter usb_to_can_v2_automotive;
extern struct ixx_usb_adapter usb_to_can_v2_embedded;
extern struct ixx_usb_adapter usb_to_can_v2_professional;
extern struct ixx_usb_adapter usb_to_can_v2_low_speed;
extern struct ixx_usb_adapter usb_to_can_v2_extended;

#ifdef CANFD_CAPABLE
extern struct ixx_usb_adapter usb_to_can_fd_automotive;
extern struct ixx_usb_adapter usb_to_can_fd_compact;
extern struct ixx_usb_adapter usb_to_can_fd_professional;
extern struct ixx_usb_adapter usb_to_can_fd_pcie_mini;
extern struct ixx_usb_adapter usb_to_car;
#endif

#ifndef CAN_MAX_DLEN
#define CAN_MAX_DLEN 8
#endif

#ifndef CANFD_MAX_DLEN
#define CANFD_MAX_DLEN 64
#endif


/* supported device ids. */
#define USB_TO_CAN_V2_COMPACT_PRODUCT_ID       0x0008
#define USB_TO_CAN_V2_EMBEDDED_PRODUCT_ID      0x0009
#define USB_TO_CAN_V2_PROFESSIONAL_PRODUCT_ID  0x000A
#define USB_TO_CAN_V2_AUTOMOTIVE_PRODUCT_ID    0x000B
#define USB_TO_LIN_V2_PRODUCT_ID               0x000C
#define USB_TO_KLINE_V2_PRODUCT_ID             0x000D
#define USB_TO_CAN_V2_LOW_SPEED_PRODUCT_ID     0xFFFF
#define USB_TO_CAN_V2_EXTENDED_PRODUCT_ID      0x000E

#define USB_TO_CAN_FD_COMPACT_PRODUCT_ID       0x0014
#define USB_TO_CAN_FD_PROFESSIONAL_PRODUCT_ID  0x0016
#define USB_TO_CAN_FD_AUTOMOTIVE_PRODUCT_ID    0x0017
#define USB_TO_CAN_FD_PCIE_MINI_PRODUCT_ID     0x001B
#define USB_TO_CAR_ID                          0x001C

#define IXXAT_USB_MAX_CHANNEL  5

/* number of urbs that are submitted for rx/tx per channel */
#define IXXAT_USB_MAX_RX_URBS             4
#define IXXAT_USB_MAX_TX_URBS             10

#define IXX_BTMODE_NAT 0x01

#define IXXAT_USB_POWER_WAKEUP    0
#define IXXAT_USB_POWER_SLEEP     1

struct ixx_usb_device;

struct ixx_dev_caps
{
        u16 bus_ctrl_count;
        u16 bus_ctrl_types[32];
} __packed;

struct ixx_ctrl_caps
{
        u16 ctrl_type;
        u16 bus_coupling;
        u32 features;
        u32 clock_freq;
        u32 tsc_divisor;
        u32 cms_divisor;
        u32 cms_max_ticks;
        u32 dtx_divisor;
        u32 dtx_max_ticks;
} __packed;

struct canbtp
{
       u32 mode;   // timing mode (see CAN_BTMODE_ const)
       u32 bps;    // bits per second or prescaler (see CAN_BTMODE_)
       u16 ts1;    // length of time segment 1 in quantas
       u16 ts2;    // length of time segment 2 in quantas
       u16 sjw;    // re-synchronisation jump width in quantas
       u16 tdo;    // transceiver delay compensation offset in quantas
                   // (0 = disabled)
} __packed;

struct ixx_ctrl_caps_v2
{
        u16 ctrl_type;
        u16 bus_coupling;
        u32 features;

        u32 clock_freq;
        struct canbtp sdr_range_min;
        struct canbtp sdr_range_max;
        struct canbtp fdr_range_min;
        struct canbtp fdr_range_max;

        u32 tsc_freq;
        u32 tsc_divisor;

        u32 cms_freq;
        u32 cms_divisor;
        u32 cms_max_ticks;

        u32 dtx_freq;
        u32 dtx_divisor;
        u32 dtx_max_ticks;
} __packed;

struct ixx_intf_info
{
        char   device_name[16];       // device name
        char   device_id[16];         // device identification ( unique device id)
        u16    device_version;        // device version ( 0, 1, ...)
        u32    device_fpga_version;   // device version of FPGA design
} __packed;

struct ixx_intf_fw_info
{
        u32 firmware_type;  // type of currently running firmware
        u16 reserved;       // reserved
        u16 major_version;  // major firmware version number
        u16 minor_version;  // minor firmware version number
        u16 build_version;  // build firmware version number
} __packed;

struct ixx_usb_adapter {
        char *name;
        u32 device_id;
        struct can_clock clock;
        const struct can_bittiming_const bittiming_const;
        const struct can_bittiming_const data_bittiming_const;

        unsigned int ctrl_count;

        u32 ctrlmode_supported;

        int (*intf_probe)(struct usb_interface *intf);

        int (*dev_get_dev_caps)(struct usb_device *usb_dev, struct ixx_dev_caps* dev_caps);
        int (*dev_get_ctrl_caps)(struct usb_device *usb_dev, struct ixx_ctrl_caps* ctrl_caps, int index);

        int (*intf_get_info)(struct ixx_usb_device *dev, struct ixx_intf_info* intf_info);
        int (*intf_get_fw_info)(struct ixx_usb_device *dev, struct ixx_intf_fw_info* fw_info);

        int (*dev_init)(struct ixx_usb_device *dev);
        void (*dev_exit)(struct ixx_usb_device *dev);
        void (*dev_free)(struct ixx_usb_device *dev);
        int (*dev_open)(struct ixx_usb_device *dev);
        int (*dev_close)(struct ixx_usb_device *dev);
        int (*dev_set_bittiming)(struct ixx_usb_device *dev, struct can_bittiming *bt);
        int (*dev_set_bus)(struct ixx_usb_device *dev, u8 onoff);
        int (*dev_decode_buf)(struct ixx_usb_device *dev, struct urb *urb);
        int (*dev_encode_msg)(struct ixx_usb_device *dev, struct sk_buff *skb,
                u8 *obuf, size_t *size);
        int (*dev_start)(struct ixx_usb_device *dev);
        int (*dev_stop)(struct ixx_usb_device *dev);
        int (*dev_restart_async)(struct ixx_usb_device *dev, struct urb *urb,
                u8 *buf);
        int (*dev_power)(struct usb_device *usb_dev, u8 mode);
        u8 ep_msg_in[IXXAT_USB_MAX_CHANNEL];
        u8 ep_msg_out[IXXAT_USB_MAX_CHANNEL];

        int rx_buffer_size;
        int tx_buffer_size;
        int sizeof_dev_private;

        int has_bgi_ep;

};

struct ixx_time_ref {
        struct timeval tv_host_0;
        u32 ts_dev_0;
        u32 ts_dev_last;
};

struct ixx_tx_urb_context {
        struct ixx_usb_device *dev;
        u32 echo_index;
        u8 dlc;
        u8 count;
        struct urb *urb;
};

/*IXXAT USB device */
struct ixx_usb_device {
        struct can_priv can;
        struct ixx_usb_adapter *adapter;
        unsigned int ctrl_idx;
        u32 state;

        struct sk_buff *echo_skb[IXXAT_USB_MAX_TX_URBS];

        struct usb_device *udev;
        struct net_device *netdev;

        atomic_t active_tx_urbs;
        struct usb_anchor tx_submitted;
        struct ixx_tx_urb_context tx_contexts[IXXAT_USB_MAX_TX_URBS];

        struct usb_anchor rx_submitted;

        u32 device_number;
        u8 device_rev;

        u8 ep_msg_in;
        u8 ep_msg_out;

        u8 transmit_buffer[256];
        u8 transmit_ptr;
        u8 transmit_count;
        u8 transmit_dlc;

        struct task_struct *restart_task;
        u8 restart_flag;
        u8 must_quit;
        wait_queue_head_t wait_queue;

        struct ixx_usb_device *prev_siblings;
        struct ixx_usb_device *next_siblings;

        u8 btr0;
        u8 btr1;

        int ctrl_opened_count;

        struct ixx_time_ref time_ref;

        struct ixx_intf_info dev_info;
        struct ixx_intf_fw_info fw_info;

        struct can_berr_counter bec;
};

struct ixx_can_msg
{
        u8  size;
        u32 time;
        u32 msg_id;
        u32 flags;
        u8  data[CAN_MAX_DLEN];
} __packed;

struct ixx_can_msg_v2
{
        u8  size;
        u32 time;
        u32 msg_id;
        u32 flags;
        u32 client_id;
        u8  data[CANFD_MAX_DLEN];
} __packed;

void ixxat_dump_mem(char *prompt, void *p, int l);

void ixxat_usb_update_ts_now(struct ixx_usb_device *dev, u32 ts_now);
void ixxat_usb_set_ts_now(struct ixx_usb_device *dev, u32 ts_now);
void ixxat_usb_get_ts_tv(struct ixx_usb_device *dev, u32 ts,
      ktime_t* k_time);

void ixxat_usb_async_complete(struct urb *urb);
void ixxat_usb_restart_complete(struct ixx_usb_device *dev);
#endif
