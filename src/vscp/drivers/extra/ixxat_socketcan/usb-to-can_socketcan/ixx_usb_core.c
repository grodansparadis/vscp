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
#include <linux/init.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/usb.h>
#include <linux/errno.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <linux/can.h>
#include <linux/can/dev.h>
#include <linux/can/error.h>
#include <asm-generic/errno.h>

#include "ixx_usb_core.h"

MODULE_AUTHOR("Michael Hengler <mhengler@ixxat.de>");
MODULE_DESCRIPTION("CAN driver for IXXAT USB-to-CAN V2 adapters");
MODULE_LICENSE("GPL v2");

#define IXXAT_USB_DRIVER_NAME             "ixx_usb"

#define IXXAT_USB_BUS_CAN  1 // CAN
#define IXXAT_USB_BUS_TYPE(BusCtrl)  (u8)  ( ((BusCtrl) >> 8) & 0x00FF )
#define IXXAT_USB_VENDOR_ID             0x08d8

#define IXXAT_USB_STATE_CONNECTED 0x00000001
#define IXXAT_USB_STATE_STARTED   0x00000002


/* Table of devices that work with this driver */
static struct usb_device_id ixxat_usb_table[] = {
        {USB_DEVICE(IXXAT_USB_VENDOR_ID, USB_TO_CAN_V2_COMPACT_PRODUCT_ID)},
        {USB_DEVICE(IXXAT_USB_VENDOR_ID, USB_TO_CAN_V2_EMBEDDED_PRODUCT_ID)},
        {USB_DEVICE(IXXAT_USB_VENDOR_ID, USB_TO_CAN_V2_PROFESSIONAL_PRODUCT_ID)},
        {USB_DEVICE(IXXAT_USB_VENDOR_ID, USB_TO_CAN_V2_AUTOMOTIVE_PRODUCT_ID)},
        {USB_DEVICE(IXXAT_USB_VENDOR_ID, USB_TO_LIN_V2_PRODUCT_ID)},
        {USB_DEVICE(IXXAT_USB_VENDOR_ID, USB_TO_KLINE_V2_PRODUCT_ID)},
#ifdef CANFD_CAPABLE
        {USB_DEVICE(IXXAT_USB_VENDOR_ID, USB_TO_CAN_FD_AUTOMOTIVE_PRODUCT_ID)},
        {USB_DEVICE(IXXAT_USB_VENDOR_ID, USB_TO_CAN_FD_COMPACT_PRODUCT_ID)},
        {USB_DEVICE(IXXAT_USB_VENDOR_ID, USB_TO_CAN_FD_PROFESSIONAL_PRODUCT_ID)},
        {USB_DEVICE(IXXAT_USB_VENDOR_ID, USB_TO_CAN_FD_PCIE_MINI_PRODUCT_ID)},
        {USB_DEVICE(IXXAT_USB_VENDOR_ID, USB_TO_CAR_ID)},
#endif
        {} /* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, ixxat_usb_table);

/* List of supported IXX-USB adapters (NULL terminated list) */
static struct ixx_usb_adapter *ixx_usb_adapters_list[] = {
        &usb_to_can_v2_compact,
        &usb_to_can_v2_automotive,
        &usb_to_can_v2_embedded,
        &usb_to_can_v2_professional,
        &usb_to_can_v2_low_speed,
        &usb_to_can_v2_extended,
#ifdef CANFD_CAPABLE
        &usb_to_can_fd_automotive,
        &usb_to_can_fd_compact,
        &usb_to_can_fd_professional,
        &usb_to_can_fd_pcie_mini,
        &usb_to_car,
#endif
        NULL,
};

/*
 * dump memory
 */
#define DUMP_WIDTH    16
void ixxat_dump_mem(char *prompt, void *p, int l)
{
        pr_info("%s dumping %s (%d bytes):\n",
        IXXAT_USB_DRIVER_NAME, prompt ? prompt : "memory", l);
        print_hex_dump(KERN_INFO, IXXAT_USB_DRIVER_NAME " ", DUMP_PREFIX_NONE,
        DUMP_WIDTH, 1, p, l, false);
}

static void ixxat_usb_add_us(struct timeval *tv, u64 delta_us)
{
        /* number of s. to add to final time */
        u32 delta_s = div_u64(delta_us, 1000000);

        delta_us -= delta_s * 1000000;

        tv->tv_usec += delta_us;
        if (tv->tv_usec >= 1000000) {
                tv->tv_usec -= 1000000;
                delta_s++;
        }
        tv->tv_sec += delta_s;
}

void ixxat_usb_get_ts_tv(struct ixx_usb_device *dev, u32 ts, ktime_t *k_time)
{
        struct timeval tv = dev->time_ref.tv_host_0;

        if (ts < dev->time_ref.ts_dev_last) {
                ixxat_usb_update_ts_now(dev, ts);
        }

        dev->time_ref.ts_dev_last = ts;
        ixxat_usb_add_us(&tv, ts - dev->time_ref.ts_dev_0);

        if(k_time)
                *k_time = timeval_to_ktime(tv);
}

void ixxat_usb_update_ts_now(struct ixx_usb_device *dev, u32 hw_time_base)
{
        u64 timebase;

        timebase = (u64)0x00000000FFFFFFFF - (u64)dev->time_ref.ts_dev_0 + (u64)hw_time_base;

        ixxat_usb_add_us(&dev->time_ref.tv_host_0, timebase);

        dev->time_ref.ts_dev_0 = hw_time_base;
}

void ixxat_usb_set_ts_now(struct ixx_usb_device *dev, u32 hw_time_base)
{
        dev->time_ref.ts_dev_0 = hw_time_base;
        do_gettimeofday(&dev->time_ref.tv_host_0);
        dev->time_ref.ts_dev_last = hw_time_base;
}

/*
 * callback for bulk Rx urb
 */
static void ixxat_usb_read_bulk_callback(struct urb *urb)
{
        struct ixx_usb_device *dev = urb->context;
        struct net_device *netdev;
        int err;

        netdev = dev->netdev;

        if (!netif_device_present(netdev))
                return;

        /* check reception status */
        switch (urb->status) {
        case 0:
                /* success */
                break;

        case -EILSEQ:
        case -ENOENT:
        case -ECONNRESET:
        case -ESHUTDOWN:
                return;

        default:
                if (net_ratelimit())
                        netdev_err(netdev, "Rx urb aborted (%d)\n",
                                        urb->status);
                goto resubmit_urb;
        }

        /* protect from any incoming empty msgs */
        if ((urb->actual_length > 0) && (dev->adapter->dev_decode_buf)) {
                /* handle these kinds of msgs only if _start callback called */
                if (dev->state & IXXAT_USB_STATE_STARTED) 
                        err = dev->adapter->dev_decode_buf(dev, urb);
        }

        resubmit_urb: usb_fill_bulk_urb(urb, dev->udev,
                        usb_rcvbulkpipe(dev->udev, dev->ep_msg_in),
                        urb->transfer_buffer, dev->adapter->rx_buffer_size,
                        ixxat_usb_read_bulk_callback, dev);

        usb_anchor_urb(urb, &dev->rx_submitted);
        err = usb_submit_urb(urb, GFP_ATOMIC);
        if (!err)
                return;

        usb_unanchor_urb(urb);

        if (err == -ENODEV)
                netif_device_detach(netdev);
        else
                netdev_err(netdev, "failed resubmitting read bulk urb: %d\n",
                                err);
}

/*
 * callback for bulk Tx urb
 */
static void ixxat_usb_write_bulk_callback(struct urb *urb)
{
        struct ixx_tx_urb_context *context = urb->context;
        struct ixx_usb_device *dev;
        struct net_device *netdev;

        BUG_ON(!context);

        dev = context->dev;
        netdev = dev->netdev;

        atomic_dec(&dev->active_tx_urbs);

        if (!netif_device_present(netdev))
                return;

        /* check tx status */
        switch (urb->status) {
        case 0:
                /* transmission complete */
                netdev->stats.tx_packets += context->count;
                netdev->stats.tx_bytes += context->dlc;

                /* prevent tx timeout */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
                netif_trans_update(netdev);
#else
                netdev->trans_start = jiffies;
#endif
                break;


        case -EPROTO:
        case -ENOENT:
        case -ECONNRESET:
        case -ESHUTDOWN:
                break;
        default:
                if (net_ratelimit())
                        netdev_err(netdev, "Tx urb aborted (%d)\n",
                                        urb->status);
                break;
        }

        /* should always release echo skb and corresponding context */
        can_get_echo_skb(netdev, context->echo_index);
        context->echo_index = IXXAT_USB_MAX_TX_URBS;

        /* do wakeup tx queue in case of success only */
        if (!urb->status)
                netif_wake_queue(netdev);
}

/*
 * called by netdev to send one skb on the CAN interface.
 */
static netdev_tx_t ixxat_usb_ndo_start_xmit(struct sk_buff *skb,
                struct net_device *netdev)
{
        struct ixx_usb_device *dev = netdev_priv(netdev);
        struct ixx_tx_urb_context *context = NULL;
        struct net_device_stats *stats = &netdev->stats;
        struct canfd_frame *cf = (struct canfd_frame *) skb->data;
        struct urb *urb;
        u8 *obuf;
        int i, err;
        size_t size = dev->adapter->tx_buffer_size;

        if (can_dropped_invalid_skb(netdev, skb))
                return NETDEV_TX_OK;

        for (i = 0; i < IXXAT_USB_MAX_TX_URBS; i++) {
                if (dev->tx_contexts[i].echo_index == IXXAT_USB_MAX_TX_URBS) {
                        context = dev->tx_contexts + i;
                        break;
                }
        }

        if (!context) {
                /* should not occur except during restart */
                return NETDEV_TX_BUSY;
        }

        urb = context->urb;
        obuf = urb->transfer_buffer;

        err = dev->adapter->dev_encode_msg(dev, skb, obuf, &size);

        context->echo_index = i;
        context->dlc = cf->len;
        context->count = 1;

        urb->transfer_buffer_length = size;

        if (err) {
                if (net_ratelimit())
                        netdev_err(netdev, "packet dropped\n");
                dev_kfree_skb(skb);
                stats->tx_dropped++;
                return NETDEV_TX_OK;
        }

        usb_anchor_urb(urb, &dev->tx_submitted);

        can_put_echo_skb(skb, netdev, context->echo_index);

        atomic_inc(&dev->active_tx_urbs);

        err = usb_submit_urb(urb, GFP_ATOMIC);
        if (err) {
                can_free_echo_skb(netdev, context->echo_index);

                usb_unanchor_urb(urb);

                /* this context is not used in fact */
                context->echo_index = IXXAT_USB_MAX_TX_URBS;

                atomic_dec(&dev->active_tx_urbs);

                switch (err) {
                case -ENODEV:
                        netif_device_detach(netdev);
                        break;
                case -ENOENT:
                        /* cable unplugged */
                        stats->tx_dropped++;
                        break;
                default:
                        stats->tx_dropped++;
                        netdev_warn(netdev, "tx urb submitting failed err=%d\n",
                                        err);
                }
        } else {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
                netif_trans_update(netdev);
#else
                netdev->trans_start = jiffies;
#endif

                /* slow down tx path */
                if (atomic_read(&dev->active_tx_urbs) >= IXXAT_USB_MAX_TX_URBS)
                        netif_stop_queue(netdev);
        }

        return NETDEV_TX_OK;
}

/*
 * start the CAN interface.
 * Rx and Tx urbs are allocated here. Rx urbs are submitted here.
 */
static int ixxat_usb_start(struct ixx_usb_device *dev)
{
        struct net_device *netdev = dev->netdev;
        int err, i;

        for (i = 0; i < IXXAT_USB_MAX_RX_URBS; i++) {
                struct urb *urb;
                u8 *buf;

                /* create a URB, and a buffer for it, to receive usb messages */
                urb = usb_alloc_urb(0, GFP_KERNEL);
                if (!urb) {
                        netdev_err(netdev, "No memory left for URBs\n");
                        err = -ENOMEM;
                        break;
                }

                buf = kmalloc(dev->adapter->rx_buffer_size, GFP_KERNEL);
                if (!buf) {
                        usb_free_urb(urb);
                        err = -ENOMEM;
                        break;
                }

                usb_fill_bulk_urb(urb, dev->udev,
                                usb_rcvbulkpipe(dev->udev, dev->ep_msg_in), buf,
                                dev->adapter->rx_buffer_size,
                                ixxat_usb_read_bulk_callback, dev);

                /* ask last usb_free_urb() to also kfree() transfer_buffer */
                urb->transfer_flags |= URB_FREE_BUFFER;
                usb_anchor_urb(urb, &dev->rx_submitted);

                err = usb_submit_urb(urb, GFP_KERNEL);
                if (err) {
                        if (err == -ENODEV)
                                netif_device_detach(dev->netdev);

                        usb_unanchor_urb(urb);
                        kfree(buf);
                        usb_free_urb(urb);
                        break;
                }

                /* drop reference, USB core will take care of freeing it */
                usb_free_urb(urb);
        }

        /* did we submit any URBs? Warn if we was not able to submit all urbs */
        if (i < IXXAT_USB_MAX_RX_URBS) {
                if (i == 0) {
                        netdev_err(netdev, "couldn't setup any rx URB\n");
                        return err;
                }

                netdev_warn(netdev, "rx performance may be slow\n");
        }

        /* pre-alloc tx buffers and corresponding urbs */
        for (i = 0; i < IXXAT_USB_MAX_TX_URBS; i++) {
                struct ixx_tx_urb_context *context;
                struct urb *urb;
                u8 *buf;

                /* create a URB and a buffer for it, to transmit usb messages */
                urb = usb_alloc_urb(0, GFP_KERNEL);
                if (!urb) {
                        netdev_err(netdev, "No memory left for URBs\n");
                        err = -ENOMEM;
                        break;
                }

                buf = kmalloc(dev->adapter->tx_buffer_size, GFP_KERNEL);
                if (!buf) {
                        usb_free_urb(urb);
                        err = -ENOMEM;
                        break;
                }

                context = dev->tx_contexts + i;
                context->dev = dev;
                context->urb = urb;

                usb_fill_bulk_urb(urb, dev->udev,
                                usb_sndbulkpipe(dev->udev, dev->ep_msg_out),
                                buf, dev->adapter->tx_buffer_size,
                                ixxat_usb_write_bulk_callback, context);

                /* ask last usb_free_urb() to also kfree() transfer_buffer */
                urb->transfer_flags |= URB_FREE_BUFFER;
        }

        /* warn if we were not able to allocate enough tx contexts */
        if (i < IXXAT_USB_MAX_TX_URBS) {
                if (i == 0) {
                        netdev_err(netdev, "couldn't setup any tx URB\n");
                        goto err_tx;
                }

                netdev_warn(netdev, "tx performance may be slow\n");
        }

        if (dev->adapter->dev_start) {
                err = dev->adapter->dev_start(dev);
                if (err)
                        goto err_adapter;
        }

        dev->state |= IXXAT_USB_STATE_STARTED;

        dev->can.state = CAN_STATE_ERROR_ACTIVE;

        return 0;

        err_adapter: if (err == -ENODEV)
                netif_device_detach(dev->netdev);

        netdev_warn(netdev, "couldn't submit control: %d\n", err);

        for (i = 0; i < IXXAT_USB_MAX_TX_URBS; i++) {
                usb_free_urb(dev->tx_contexts[i].urb);
                dev->tx_contexts[i].urb = NULL;
        }
        err_tx: usb_kill_anchored_urbs(&dev->rx_submitted);

        return err;
}

/*
 * called by netdev to open the corresponding CAN interface.
 */
static int ixxat_usb_ndo_open(struct net_device *netdev)
{
        struct ixx_usb_device *dev = netdev_priv(netdev);
        int err;

        /* common open */
        err = open_candev(netdev);
        if (err)
                return err;

        /* finally start device */
        err = ixxat_usb_start(dev);
        if (err) {
                netdev_err(netdev, "couldn't start device: %d\n", err);
                close_candev(netdev);
                return err;
        }

        netif_start_queue(netdev);

        return 0;
}

/*
 * unlink in-flight Rx and Tx urbs and free their memory.
 */
static void ixxat_usb_unlink_all_urbs(struct ixx_usb_device *dev)
{
        int i;

        /* free all Rx (submitted) urbs */
        usb_kill_anchored_urbs(&dev->rx_submitted);

        /* free unsubmitted Tx urbs first */
        for (i = 0; i < IXXAT_USB_MAX_TX_URBS; i++) {
                struct urb *urb = dev->tx_contexts[i].urb;

                if (!urb
                                || dev->tx_contexts[i].echo_index
                                                != IXXAT_USB_MAX_TX_URBS) {
                        /*
                         * this urb is already released or always submitted,
                         * let usb core free by itself
                         */
                        continue;
                }

                usb_free_urb(urb);
                dev->tx_contexts[i].urb = NULL;
        }

        /* then free all submitted Tx urbs */
        usb_kill_anchored_urbs(&dev->tx_submitted);
        atomic_set(&dev->active_tx_urbs, 0);
}

/*
 * called by netdev to close the corresponding CAN interface.
 */
static int ixxat_usb_ndo_stop(struct net_device *netdev)
{
        struct ixx_usb_device *dev = netdev_priv(netdev);

        dev->state &= ~IXXAT_USB_STATE_STARTED;
        netif_stop_queue(netdev);

        /* unlink all pending urbs and free used memory */
        ixxat_usb_unlink_all_urbs(dev);

        if (dev->adapter->dev_stop)
                dev->adapter->dev_stop(dev);

        close_candev(netdev);

        dev->can.state = CAN_STATE_STOPPED;

        return 0;
}

/*
 * handle end of waiting for the device to reset
 */
void ixxat_usb_restart_complete(struct ixx_usb_device *dev)
{
        /* finally MUST update can state */
        dev->can.state = CAN_STATE_ERROR_ACTIVE;

        /* netdev queue can be awaken now */
        netif_wake_queue(dev->netdev);
}

void ixxat_usb_async_complete(struct urb *urb)
{
        kfree(urb->transfer_buffer);
        usb_free_urb(urb);
}

/*
 * candev callback used to change CAN mode.
 * Warning: this is called from a timer context!
 */
static int ixxat_usb_set_mode(struct net_device *netdev, enum can_mode mode)
{
        struct ixx_usb_device *dev = netdev_priv(netdev);
        int err = 0;

        switch (mode) {
        case CAN_MODE_START:
                dev->restart_flag = 1;
                wake_up_interruptible(&dev->wait_queue);
                break;
        default:
                return -EOPNOTSUPP;
        }

        return err;
}

/*
 * candev callback used to set device bitrate.
 */
static int ixxat_usb_set_bittiming(struct net_device *netdev)
{
        struct ixx_usb_device* dev = (struct ixx_usb_device*) netdev_priv(
                        netdev);
        struct can_bittiming *bt = &dev->can.bittiming;

        if (dev->adapter->dev_set_bittiming) {
                int err = dev->adapter->dev_set_bittiming(dev, bt);

                if (err)
                        netdev_info(netdev, "couldn't set bitrate (err %d)\n",
                                        err);
                return err;
        }

        return 0;
}

/*
 * candev callback used to set error counters.
 */
static int ixxat_usb_get_berr_counter(const struct net_device *netdev,
                                     struct can_berr_counter *bec)
{
        struct ixx_usb_device* dev = (struct ixx_usb_device*) netdev_priv(
                        netdev);
  
        *bec = dev->bec;

        return 0;
}

static const struct net_device_ops ixx_usb_netdev_ops = { .ndo_open =
                ixxat_usb_ndo_open, .ndo_stop = ixxat_usb_ndo_stop,
                .ndo_start_xmit = ixxat_usb_ndo_start_xmit, 
#ifdef CANFD_CAPABLE
                .ndo_change_mtu = can_change_mtu, 
#endif
                                                        };

/*
 * create one device which is attached to CAN controller #ctrl_idx of the
 * usb adapter.
 */
static int ixxat_usb_create_dev(struct ixx_usb_adapter *ixx_usb_adapter,
                struct usb_interface *intf, int ctrl_idx)
{
        struct usb_device *usb_dev = interface_to_usbdev(intf);
        int sizeof_candev = ixx_usb_adapter->sizeof_dev_private;
        struct ixx_usb_device *dev;
        struct net_device *netdev;
        int i, err = 0, ep_off = 0;
        u16 tmp16;

        if (sizeof_candev < sizeof(struct ixx_usb_device))
                sizeof_candev = sizeof(struct ixx_usb_device);

        netdev = alloc_candev(sizeof_candev, IXXAT_USB_MAX_TX_URBS);
        if (!netdev) {
                dev_err(&intf->dev, "%s: couldn't alloc candev\n",
                IXXAT_USB_DRIVER_NAME);
                return -ENOMEM;
        }

        dev = netdev_priv(netdev);

        dev->transmit_ptr = 0;
        dev->transmit_dlc = 0;
        dev->transmit_count = 0;

        dev->restart_flag = 0;
        dev->restart_task = 0;
        dev->must_quit = 0;
        init_waitqueue_head(&dev->wait_queue);

        dev->ctrl_opened_count = 0;

        dev->udev = usb_dev;
        dev->netdev = netdev;
        dev->adapter = ixx_usb_adapter;
        dev->ctrl_idx = ctrl_idx;
        dev->state = IXXAT_USB_STATE_CONNECTED;
        
        ep_off = ixx_usb_adapter->has_bgi_ep ? 1 : 0;

        /* Add +1 because of the bgi endpoint */
        dev->ep_msg_in = ixx_usb_adapter->ep_msg_in[ctrl_idx+ep_off];
        dev->ep_msg_out = ixx_usb_adapter->ep_msg_out[ctrl_idx+ep_off];

        dev->can.clock = ixx_usb_adapter->clock;
        dev->can.bittiming_const = &ixx_usb_adapter->bittiming_const;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 3)
        dev->can.data_bittiming_const = &ixx_usb_adapter->data_bittiming_const;
#endif
  
        dev->can.do_set_bittiming = ixxat_usb_set_bittiming;
        dev->can.do_set_mode = ixxat_usb_set_mode;
        dev->can.do_get_berr_counter = ixxat_usb_get_berr_counter;

        dev->can.ctrlmode_supported = ixx_usb_adapter->ctrlmode_supported;

        netdev->netdev_ops = &ixx_usb_netdev_ops;

        netdev->flags |= IFF_ECHO; /* we support local echo */

        init_usb_anchor(&dev->rx_submitted);

        init_usb_anchor(&dev->tx_submitted);
        atomic_set(&dev->active_tx_urbs, 0);

        for (i = 0; i < IXXAT_USB_MAX_TX_URBS; i++)
                dev->tx_contexts[i].echo_index = IXXAT_USB_MAX_TX_URBS;

        dev->prev_siblings = usb_get_intfdata(intf);
        usb_set_intfdata(intf, dev);

        SET_NETDEV_DEV(netdev, &intf->dev);

        err = register_candev(netdev);
        if (err) {
                dev_err(&intf->dev, "couldn't register CAN device: %d\n", err);
                goto lbl_set_intf_data;
        }

        if (dev->prev_siblings)
                (dev->prev_siblings)->next_siblings = dev;

        /* keep hw revision into the netdevice */
        tmp16 = le16_to_cpu(usb_dev->descriptor.bcdDevice);
        dev->device_rev = tmp16 >> 8;

        if (dev->adapter->dev_init) {
                err = dev->adapter->dev_init(dev);
                if (err)
                        goto lbl_set_intf_data;
        }

        if (dev->adapter->intf_get_info)
                dev->adapter->intf_get_info(dev,
                                &dev->dev_info);

        netdev_info(netdev, "attached to %s channel %u (device %s)\n",
                        dev->dev_info.device_name, ctrl_idx,
                        dev->dev_info.device_id);

        return 0;

        lbl_set_intf_data: usb_set_intfdata(intf, dev->prev_siblings);
        free_candev(netdev);

        return err;
}

/*
 * called by the usb core when the device is unplugged from the system
 */
static void ixxat_usb_disconnect(struct usb_interface *intf)
{
        struct ixx_usb_device *dev;
        struct ixx_usb_device *dev_prev_siblings;

        /* unregister as many netdev devices as siblings */
        for (dev = usb_get_intfdata(intf); dev; dev = dev_prev_siblings) {
                struct net_device *netdev = dev->netdev;
                char name[IFNAMSIZ];

                dev_prev_siblings = dev->prev_siblings;
                dev->state &= ~IXXAT_USB_STATE_CONNECTED;
                strncpy(name, netdev->name, IFNAMSIZ);

                unregister_netdev(netdev);

                dev->next_siblings = NULL;
                if (dev->adapter->dev_free)
                        dev->adapter->dev_free(dev);
                    
                free_candev(netdev);
                dev_dbg(&intf->dev, "%s removed\n", name);
        }

        usb_set_intfdata(intf, NULL);
}

/*
 * probe function for new ixxat-usb devices
 */
static int ixxat_usb_probe(struct usb_interface *intf,
                const struct usb_device_id *id)
{
        struct usb_device *usb_dev = interface_to_usbdev(intf);
        struct ixx_usb_adapter *ixx_usb_adapter, **pp;
        int i, err = -ENOMEM;
        struct ixx_dev_caps dev_caps;

        usb_dev = interface_to_usbdev(intf);

        usb_reset_configuration(usb_dev);

        /* get corresponding IXX-USB adapter */
        for (pp = ixx_usb_adapters_list; *pp; pp++)
                if ((*pp)->device_id == le16_to_cpu(usb_dev->descriptor.idProduct))
                        break;

        ixx_usb_adapter = *pp;
        if (!ixx_usb_adapter) {
                /* should never come except device_id bad usage in this file */
                pr_err("%s: didn't find device id. 0x%x in devices list\n",
                IXXAT_USB_DRIVER_NAME, le16_to_cpu(usb_dev->descriptor.idProduct));
                return -ENODEV;
        }

        /* got corresponding adapter: check if it handles current interface */
        if (ixx_usb_adapter->intf_probe) {
                err = ixx_usb_adapter->intf_probe(intf);
                if (err)
                        return err;
        }
  
        if (ixx_usb_adapter->dev_power) {
                err = ixx_usb_adapter->dev_power(usb_dev, IXXAT_USB_POWER_WAKEUP);
                if (err)
                        return err;
          
                /* Give usb device some time to start its can controllers */
                msleep(500);
        }

        /* got corresponding adapter: check the available controllers */
        if (ixx_usb_adapter->dev_get_dev_caps) {
                err = ixx_usb_adapter->dev_get_dev_caps(usb_dev, &dev_caps);
                if (err)
                        return err;

                for (i = 0; i < dev_caps.bus_ctrl_count; i++) {
                        if ( IXXAT_USB_BUS_CAN
                                        == IXXAT_USB_BUS_TYPE(dev_caps.bus_ctrl_types[i]))
                                ixx_usb_adapter->ctrl_count++;
                }

                for (i = 0; i < dev_caps.bus_ctrl_count; i++) {
                        if ( IXXAT_USB_BUS_CAN == IXXAT_USB_BUS_TYPE(dev_caps.bus_ctrl_types[i]))
                                err = ixxat_usb_create_dev(ixx_usb_adapter, intf, i);
                        if (err) {
                                /* deregister already created devices */
                                ixxat_usb_disconnect(intf);
                                break;
                        }
                }
        }

        return err;
}

/* usb specific object needed to register this driver with the usb subsystem */
static struct usb_driver ixx_usb_driver = { .name = IXXAT_USB_DRIVER_NAME,
                .disconnect = ixxat_usb_disconnect, .probe = ixxat_usb_probe,
                .id_table = ixxat_usb_table, };

static int __init ixx_usb_init(void)
{
        int err;

        /* register this driver with the USB subsystem */
        err = usb_register(&ixx_usb_driver);
        if (err)
                pr_err("%s: usb_register failed (err %d)\n",
                IXXAT_USB_DRIVER_NAME, err);

        return err;
}

static int ixxat_usb_do_device_exit(struct device *d, void *arg)
{
        struct usb_interface 
                *intf = (struct usb_interface*)to_usb_interface(d);
        struct ixx_usb_device *dev;

        /* stop as many netdev devices as siblings */
        for (dev = usb_get_intfdata(intf); dev; dev = dev->prev_siblings) {
                struct net_device *netdev = dev->netdev;

                if (netif_device_present(netdev))
                        if (dev->adapter->dev_exit)
                                dev->adapter->dev_exit(dev);
        }

        return 0;
}

static void __exit ixx_usb_exit(void)
{
        int err;

        /* last chance do send any synchronous commands here */
        err = driver_for_each_device(&ixx_usb_driver.drvwrap.driver, NULL,
                         NULL, ixxat_usb_do_device_exit);
        if (err)
            pr_err("%s: failed to stop all can devices (err %d)\n",
                IXXAT_USB_DRIVER_NAME, err);

        /* deregister this driver with the USB subsystem */
        usb_deregister(&ixx_usb_driver);

        pr_info("%s: IXX-USB interfaces driver unloaded\n",
            IXXAT_USB_DRIVER_NAME);
}

module_init(ixx_usb_init);
module_exit(ixx_usb_exit);
