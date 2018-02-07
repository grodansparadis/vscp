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
 * General Public License for more details
 */
#include <linux/kernel.h>
#include <asm/types.h>
#include <linux/init.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/pci.h>
#include <linux/errno.h>
#include <linux/skbuff.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/ktime.h>
#include <linux/can.h>
#include <linux/can/dev.h>
#include <linux/can/error.h>
#include <asm-generic/errno.h>

#include "ixx_pci_core.h"

MODULE_AUTHOR("Michael Hengler <mhengler@ixxat.de>");
MODULE_DESCRIPTION("CAN driver for IXXAT CAN-IBXXX adapters");
MODULE_LICENSE("GPL v2");

#define IXXAT_PCI_DRIVER_NAME             "ixx_pci"
#define IXXAT_PCI_BUS_TYPE(BusCtrl)       (u8)(((BusCtrl) >> 8) & 0x00FF)

static struct pci_device_id ixxat_pci_table[] =
{
        { vendor:      CAN_IB1X0_VENDOR_ID,
          device:      CAN_IB100_DEVICE_ID,
          subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB100_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
        { vendor:      CAN_IB1X0_VENDOR_ID,
          device:      CAN_IB110_DEVICE_ID,
          subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB110_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
        { vendor:      CAN_IB1X0_VENDOR_ID,
          device:      CAN_IB300_DEVICE_ID,
          subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB300_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
        { vendor:      CAN_IB1X0_VENDOR_ID,
          device:      CAN_IB310_DEVICE_ID,
          subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB310_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
        { vendor:      CAN_IB1X0_VENDOR_ID,
          device:      CAN_IB120_DEVICE_ID,
          subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB120_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
        { vendor:      CAN_IB1X0_VENDOR_ID,
          device:      CAN_IB130_DEVICE_ID,
          subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB130_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
#ifdef CANFD_CAPABLE
        { vendor:      CAN_IB1X0_VENDOR_ID,
          device:      CAN_IB500_DEVICE_ID,
          subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB500_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
        { vendor:      CAN_IB1X0_VENDOR_ID,
          device:      CAN_IB510_DEVICE_ID,
          subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB510_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
        { vendor:      CAN_IB1X0_VENDOR_ID,
          device:      CAN_IB520_DEVICE_ID,
          subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB520_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
        { vendor:      CAN_IB1X0_VENDOR_ID,
          device:      CAN_IB700_DEVICE_ID,
          subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB700_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
        { vendor:      CAN_IB1X0_VENDOR_ID,
          device:      CAN_IB710_DEVICE_ID,
          subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB710_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
#endif
        { vendor:      CAN_IB2X0_VENDOR_ID,
          device:      CAN_IB200_DEVICE_ID,
          subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB200_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
        { vendor:      CAN_IB2X0_VENDOR_ID,
          device:      CAN_IB210_DEVICE_ID,
          subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB210_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
        { vendor:      CAN_IB2X0_VENDOR_ID,
          device:      CAN_IB400_DEVICE_ID,
          subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB400_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
        { vendor:      CAN_IB2X0_VENDOR_ID,
          device:      CAN_IB410_DEVICE_ID,
          subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB410_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
        { vendor:      CAN_IB2X0_VENDOR_ID,
          device:      CAN_IB230_DEVICE_ID,
          subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB230_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
#ifdef CANFD_CAPABLE
	{ vendor:      CAN_IB2X0_VENDOR_ID,
          device:      CAN_IB600_DEVICE_ID,
          subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB600_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
        { vendor:      CAN_IB2X0_VENDOR_ID,
          device:      CAN_IB610_DEVICE_ID,
          subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB610_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
        { vendor:      CAN_IB2X0_VENDOR_ID,
          device:      CAN_IB800_DEVICE_ID,
          subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB800_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
        { vendor:      CAN_IB2X0_VENDOR_ID,
          device:      CAN_IB810_DEVICE_ID,
          subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
          subdevice:   CAN_IB810_SUB_DEVICE_ID,
          class:       0,
          class_mask:  0,
          driver_data: 0 },
#endif
        { },
};

MODULE_DEVICE_TABLE(pci, ixxat_pci_table);

/* List of supported IXX-pci adapters (NULL terminated list) */
static struct ixx_pci_adapter *ixxat_adapters_list[] = {
        /* passive interfaces */
        &can_ib_100,
        &can_ib_110,
        &can_ib_120,
        &can_ib_130,
        &can_ib_300,
        &can_ib_310,
#ifdef CANFD_CAPABLE
        &can_ib_500,
        &can_ib_510,
        &can_ib_520,
        &can_ib_700,
        &can_ib_710,
#endif
        /* active interfaces */
        &can_ib_200,
        &can_ib_210,
        &can_ib_230,
        &can_ib_400,
        &can_ib_410,
#ifdef CANFD_CAPABLE
        &can_ib_600,
        &can_ib_610,
        &can_ib_800,
        &can_ib_810,
#endif
        NULL,
};

/*
 * dump memory
 */
#define DUMP_WIDTH	16
void ixxat_dump_mem(char *prompt, void *p, int l)
{
        pr_info("%s dumping %s (%d bytes):\n", IXXAT_PCI_DRIVER_NAME,
                        prompt ? prompt : "memory", l);
        print_hex_dump(KERN_INFO, IXXAT_PCI_DRIVER_NAME " ", DUMP_PREFIX_NONE,
        DUMP_WIDTH, 1, p, l, false);
}

static void ixxat_pci_add_us(struct timeval *tv, u64 delta_us)
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

void ixxat_pci_get_ts_tv(struct ixx_pci_priv *dev, u32 ts, ktime_t *k_time)
{
        struct timeval tv = dev->time_ref.tv_host_0;

        if (ts < dev->time_ref.ts_dev_last) {
                ixxat_pci_update_ts_now(dev, ts);
        }

        dev->time_ref.ts_dev_last = ts;
        ixxat_pci_add_us(&tv, ts - dev->time_ref.ts_dev_0);

        *k_time = timeval_to_ktime(tv);
}

void ixxat_pci_update_ts_now(struct ixx_pci_priv *dev, u32 hw_time_base)
{
        u64 timebase;

        timebase = (u64)0x00000000FFFFFFFF - (u64)dev->time_ref.ts_dev_0 + (u64)hw_time_base;

        ixxat_pci_add_us(&dev->time_ref.tv_host_0, timebase);

        dev->time_ref.ts_dev_0 = hw_time_base;
}

void ixxat_pci_set_ts_now(struct ixx_pci_priv *dev, u32 hw_time_base)
{
        dev->time_ref.ts_dev_0 = hw_time_base;
        do_gettimeofday(&dev->time_ref.tv_host_0);
        dev->time_ref.ts_dev_last = hw_time_base;
}

int ixxat_pci_handle_frn(struct ixx_pci_priv *dev, u8 frn, u32 timestamp)
{
        int result = 0;
        u32 expectedfrn;
        struct sk_buff * skb = NULL;

        spin_lock(&dev->rcv_lock);
        expectedfrn = dev->frn_read + 1;
        if (expectedfrn > IXX_PCI_MAX_TX_TRANS)
                expectedfrn = 1;

        netdev_dbg(dev->netdev, "%s in frn %d expected frn %d frn_write %d",
                        __func__, frn, expectedfrn, dev->frn_write);

        if (expectedfrn == dev->frn_write) {
                netdev_dbg(dev->netdev, "expectedfrn == dev->frn_write");
//                can_free_echo_skb(dev->netdev, frn - 1);
        } else if (expectedfrn < dev->frn_write) {
                netdev_dbg(dev->netdev, "expectedfrn < dev->frn_write");
                if (frn == expectedfrn) {
                        netdev_dbg(dev->netdev, "frn == expectedfrn");
                        // frn ist richtig behandle nachricht
                        skb = dev->can.echo_skb[frn - 1];
                        if (skb)
                                ixxat_pci_get_ts_tv(dev, timestamp,
                                                &skb->tstamp);
                        can_get_echo_skb(dev->netdev, frn - 1);
                        dev->frn_read++;
                        result = 1;
                        netdev_dbg(dev->netdev, "can_get_echo_skb %d", frn - 1);
                } else if (frn < expectedfrn) {
                        netdev_dbg(dev->netdev, "frn < expectedfrn");
                        while (expectedfrn != frn) {
//                                can_free_echo_skb(dev->netdev, frn - 1);
                                frn++;
                        }
                } else if (frn > expectedfrn) {
                        netdev_dbg(dev->netdev, "frn > expectedfrn");
                        // durch überläufe sind viele nachrichten verloren gegangen
                        while (expectedfrn != frn) {
//                                can_free_echo_skb(dev->netdev, expectedfrn - 1);
                                expectedfrn++;
                        }
                        // frn ist jetzt richtig behandle nachricht
                        skb = dev->can.echo_skb[frn - 1];
                        if (skb)
                                ixxat_pci_get_ts_tv(dev, timestamp,
                                                &skb->tstamp);
                        can_get_echo_skb(dev->netdev, frn - 1);
                        dev->frn_read = expectedfrn;
                        result = 1;
                } else if (frn > expectedfrn && frn >= dev->frn_write) {
                        netdev_dbg(dev->netdev,
                                        "frn > expectedfrn && frn >= dev->frn_write");
                        while (expectedfrn != frn) {
//                                can_free_echo_skb(dev->netdev, frn - 1);
                                frn++;
                                if (frn > IXX_PCI_MAX_TX_TRANS)
                                        frn = 1;
                        }
                }
        } else if (expectedfrn > dev->frn_write) {
                netdev_dbg(dev->netdev, "expectedfrn > dev->frn_write");
                if (frn == expectedfrn) {
                        netdev_dbg(dev->netdev, "frn == expectedfrn");
                        // frn ist richtig behandle nachricht
                        skb = dev->can.echo_skb[frn - 1];
                        if (skb)
                                ixxat_pci_get_ts_tv(dev, timestamp,
                                                &skb->tstamp);
                        can_get_echo_skb(dev->netdev, frn - 1);
                        dev->frn_read++;
                        result = 1;
//                                                netdev_dbg(dev->netdev, "can_get_echo_skb %d", frn - 1);
                } else if (frn > expectedfrn) {
                        netdev_dbg(dev->netdev, "frn > expectedfrn");
                        while (expectedfrn != frn) {
//                                can_free_echo_skb(dev->netdev, expectedfrn - 1);
                                expectedfrn++;
                        }
                        // frn ist jetzt richtig behandle nachricht
                        skb = dev->can.echo_skb[frn - 1];
                        if (skb)
                                ixxat_pci_get_ts_tv(dev, timestamp,
                                                &skb->tstamp);
                        can_get_echo_skb(dev->netdev, frn - 1);
                        dev->frn_read = expectedfrn;
                        result = 1;
                } else if (frn < expectedfrn && frn >= dev->frn_write) {
                        netdev_dbg(dev->netdev,
                                        "frn < expectedfrn && frn >= dev->frn_write");
                        while (dev->frn_read != frn) {
//                                can_free_echo_skb(dev->netdev, frn - 1);
                                frn++;
                        }
                } else if (frn < expectedfrn && frn < dev->frn_write) {
                        // durch überläufe sind viele nachrichten verloren gegangen
                        netdev_dbg(dev->netdev,
                                        "frn < expectedfrn && frn < dev->frn_write");
                        while (expectedfrn != frn) {
//                                can_free_echo_skb(dev->netdev, expectedfrn - 1);
                                expectedfrn++;
                                if (expectedfrn > IXX_PCI_MAX_TX_TRANS)
                                        expectedfrn = 1;
                        }
                        // frn ist jetzt richtig behandle nachricht
                        skb = dev->can.echo_skb[frn - 1];
                        if (skb)
                                ixxat_pci_get_ts_tv(dev, timestamp,
                                                &skb->tstamp);
                        can_get_echo_skb(dev->netdev, frn - 1);
                        dev->frn_read = expectedfrn;
                        result = 1;
                }
        }
        if (dev->frn_read > IXX_PCI_MAX_TX_TRANS)
                dev->frn_read = 1;

        spin_unlock(&dev->rcv_lock);
        netdev_dbg(dev->netdev, "%s out", __func__);
        return result;
}

/*
 * called by netdev to send one skb on the CAN interface.
 */
static netdev_tx_t ixxat_pci_ndo_start_xmit(struct sk_buff *skb,
                struct net_device *netdev)
{
        int err;

        struct ixx_pci_priv* dev = netdev_priv(netdev);

        if (dev->adapter->dev_start_xmit) {
                err = dev->adapter->dev_start_xmit(skb, netdev);
                if (err)
                        return err;
        }

        return NETDEV_TX_OK;
}

/*
 * called by netdev to open the corresponding CAN interface.
 */
static int ixxat_pci_ndo_open(struct net_device *netdev)
{
        struct ixx_pci_priv *dev = netdev_priv(netdev);
        int err;

        /* common open */
        err = open_candev(netdev);
        if (err)
                return err;

        /* finally start device */
        if (dev->adapter->dev_start) {
                err = dev->adapter->dev_start(dev);
                if (err) {
                        netdev_err(netdev, "couldn't start device: %d\n", err);
                        close_candev(netdev);
                        return err;
                }
        }

        netif_start_queue(netdev);

        return 0;
}

/*
 * called by netdev to close the corresponding CAN interface.
 */
static int ixxat_pci_ndo_stop(struct net_device *netdev)
{
        struct ixx_pci_priv *dev = netdev_priv(netdev);
        int err;

        netif_stop_queue(netdev);

        if (dev->adapter->dev_stop) {
                err = dev->adapter->dev_stop(dev);
                if (err) {
                        netdev_err(netdev, "couldn't stop device: %d\n", err);
                        return err;
                }
        }

        close_candev(netdev);

        dev->can.state = CAN_STATE_STOPPED;

        return 0;
}

/*
 * candev callback used to change CAN mode.
 * Warning: this is called from a timer context!
 */
static int ixxat_pci_set_mode(struct net_device *netdev, enum can_mode mode)
{
        struct ixx_pci_priv *dev = netdev_priv(netdev);
        int err = 0;

        switch (mode) {
        case CAN_MODE_START:
                if (dev->adapter->dev_restart)
                        dev->adapter->dev_restart(dev);
                break;
        default:
                return -EOPNOTSUPP;
        }

        return err;
}

/*
 * candev callback used to set device bitrate.
 */
static int ixxat_pci_set_bittiming(struct net_device *netdev)
{
        struct ixx_pci_priv* dev = (struct ixx_pci_priv*) netdev_priv(netdev);
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

#ifdef CANFD_CAPABLE
/*
 * candev callback used to set device data bitrate.
 */
static int ixxat_pci_set_datatiming(struct net_device *netdev)
{
        struct ixx_pci_priv* dev = (struct ixx_pci_priv*) netdev_priv(netdev);
        struct can_bittiming *bt = &dev->can.data_bittiming;

        if (dev->adapter->dev_set_datatiming) {
                int err = dev->adapter->dev_set_datatiming(dev, bt);

                if (err)
                        netdev_info(netdev,
                                        "couldn't set data bitrate (err %d)\n",
                                        err);
                return err;
        }

        return 0;
}
#endif

static const struct net_device_ops ixxat_pci_netdev_ops = { .ndo_open =
                ixxat_pci_ndo_open, .ndo_stop = ixxat_pci_ndo_stop,
                .ndo_start_xmit = ixxat_pci_ndo_start_xmit, 
#ifdef CANFD_CAPABLE
.ndo_change_mtu = can_change_mtu, 
#endif
};

/*
 * create one device which is attached to CAN controller #ctrl_idx of the
 * pci adapter.
 */
static int ixxat_pci_create_dev(struct ixx_pci_adapter *ixxat_pci_adapter,
                struct pci_dev *pci_dev, struct ixx_pci_interface * intf,
                int ctrl_idx)
{
        int sizeof_candev = ixxat_pci_adapter->sizeof_dev_private;
        struct ixx_pci_priv *priv;
        struct net_device *netdev;
        int err = 0;

        if (sizeof_candev < sizeof(struct ixx_pci_priv))
                sizeof_candev = sizeof(struct ixx_pci_priv);

        netdev = alloc_candev(sizeof_candev, IXX_PCI_MAX_TX_TRANS);
        if (!netdev) {
                dev_err(&intf->pdev->dev, "%s: couldn't alloc candev\n",
                IXXAT_PCI_DRIVER_NAME);
                return -ENOMEM;
        }

        priv = netdev_priv(netdev);
        priv->intf = intf;

        priv->restart_flag = 0;
        priv->restart_task = 0;
        priv->must_quit = 0;
        init_waitqueue_head(&priv->wait_queue);

        priv->ctrl_opened_count = 0;

        priv->pdev = pci_dev;
        priv->netdev = netdev;
        priv->adapter = ixxat_pci_adapter;
        priv->ctrl_idx = ctrl_idx;

        spin_lock_init(&priv->rcv_lock);

        netif_napi_add(netdev, &priv->napi, priv->adapter->dev_napi_rx_poll, 2);

        priv->can.clock = ixxat_pci_adapter->clock;
        priv->can.bittiming_const = &ixxat_pci_adapter->bittiming_const;
#ifdef CANFD_CAPABLE
        priv->can.data_bittiming_const =
                        &ixxat_pci_adapter->data_bittiming_const;
#endif
        priv->can.do_set_bittiming = ixxat_pci_set_bittiming;
#ifdef CANFD_CAPABLE
        priv->can.do_set_data_bittiming = ixxat_pci_set_datatiming;
#endif
        priv->can.do_set_mode = ixxat_pci_set_mode;

        priv->can.ctrlmode_supported = ixxat_pci_adapter->ctrlmode_supported;

        netdev->netdev_ops = &ixxat_pci_netdev_ops;

        netdev->flags |= IFF_ECHO; /* we support local echo */

        priv->prev_siblings = pci_get_drvdata(pci_dev);
        pci_set_drvdata(pci_dev, priv);

        SET_NETDEV_DEV(netdev, &pci_dev->dev);

        err = register_candev(netdev);
        if (err) {
                dev_err(&intf->pdev->dev, "couldn't register CAN device: %d\n",
                                err);
                goto lbl_set_intf_data;
        }

        if (priv->prev_siblings)
                (priv->prev_siblings)->next_siblings = priv;

        if (!intf->priv)
                intf->priv = priv;

        if (priv->adapter->dev_init) {
                err = priv->adapter->dev_init(priv);
                if (err)
                        goto lbl_set_intf_data;
        }

        intf->intf_ctrl_count++;

        if (priv->adapter->intf_get_info)
                priv->adapter->intf_get_info(intf, &intf->dev_info);

        netdev_info(netdev, "attached to %s channel %u (interface %s)\n",
                        intf->dev_info.intf_name, ctrl_idx,
                        intf->dev_info.intf_id);

        return 0;

        lbl_set_intf_data: pci_set_drvdata(intf->pdev, priv->prev_siblings);
        free_candev(netdev);

        return err;
}

/*
 * called by the pci core when the device is unplugged from the system
 */
static void ixxat_pci_disconnect(struct pci_dev *dev)
{
        struct ixx_pci_priv *priv;
        struct ixx_pci_priv *priv_prev_siblings;

        /* unregister as many netdev devices as siblings */
        for (priv = pci_get_drvdata(dev); priv; priv = priv_prev_siblings) {
                struct net_device *netdev = priv->netdev;
                char name[IFNAMSIZ];
                printk("ixxat_pci_disconnect\n");
                strncpy(name, netdev->name, IFNAMSIZ);
                
                priv_prev_siblings = priv->prev_siblings;

                priv->next_siblings = NULL;
                if (priv->adapter->dev_free)
                        priv->adapter->dev_free(priv);

                unregister_netdev(netdev);
                
                if (!priv_prev_siblings) {
                        pci_set_drvdata(dev, NULL);
                        pci_disable_device(dev);
                }
                
                free_candev(netdev);
        }
}

static int ixxat_pci_do_device_exit(struct device *d, void *arg)
{
        struct pci_dev *pdev = (struct pci_dev*) to_pci_dev(d);
        struct ixx_pci_priv *priv;

        /* unregister as many netdev devices as siblings */
        for (priv = pci_get_drvdata(pdev); priv; priv = priv->prev_siblings) {
                struct net_device *netdev = priv->netdev;
                char name[IFNAMSIZ];

                strncpy(name, netdev->name, IFNAMSIZ);

                priv->next_siblings = NULL;
                if (priv->adapter->dev_free)
                        priv->adapter->dev_free(priv);

                unregister_netdev(netdev);
                free_candev(netdev);

                if (!priv->prev_siblings) {
                        pci_set_drvdata(pdev, NULL);
                        pci_disable_device(pdev);
                }
        }

        return 0;
}

/*
 * probe function for new CAN-IBXXX devices
 */
static int ixxat_pci_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
        struct ixx_pci_adapter *ixxat_pci_adapter, **pp;
        int i, err = -ENOMEM;
        struct ixx_pci_interface * intf;
        struct ixx_intf_caps intf_caps = { 0 };

        pci_enable_device(dev);
        pci_set_master(dev);

        /* get corresponding IXXAT-PCI adapter */
        for (pp = ixxat_adapters_list; *pp; pp++)
                if ((*pp)->device_id == id->device)
                        break;

        ixxat_pci_adapter = *pp;
        if (!ixxat_pci_adapter) {
                /* should never come except device_id bad usage in this file */
                pr_err("%s: didn't find device id. 0x%x in devices list\n",
                                IXXAT_PCI_DRIVER_NAME, id->device);
                return -ENODEV;
        }

        intf = kzalloc(sizeof(struct ixx_pci_interface), GFP_KERNEL);
        if (!intf)
                return -ENOMEM;

        intf->pdev = dev;
        mutex_init(&intf->cmd_lock);

        /* call the probe function of corresponding adapter */
        if (ixxat_pci_adapter->intf_probe) {
                err = ixxat_pci_adapter->intf_probe(dev, intf);
                if (err)
                        goto lbl_set_intf;
        }

        /* convey dma hardware address to the interface */
        if (ixxat_pci_adapter->intf_convey_dma) {
                err = ixxat_pci_adapter->intf_convey_dma(dev, intf, 1);
                if (err)
                        goto lbl_free_memory;
        }

        /* enable interface's pci interrupts  */
        if (ixxat_pci_adapter->intf_int_ena) {
                err = ixxat_pci_adapter->intf_int_ena(intf, 1);
                if (err)
                        goto lbl_free_complete;
        }

        /* reset the interface once */
        if (ixxat_pci_adapter->intf_mc_reset) {
                err = ixxat_pci_adapter->intf_mc_reset(intf);
                if (err)
                        goto lbl_free_complete;
        }

        /* trigger the pci interrupt to test if it works */
        if (ixxat_pci_adapter->intf_trigger_int) {
                err = ixxat_pci_adapter->intf_trigger_int(dev, intf);
                if (err)
                        goto lbl_free_complete;
        }

        /* send a test command to the device's command channel */
        if (ixxat_pci_adapter->intf_test_cmd) {
                err = ixxat_pci_adapter->intf_test_cmd(dev, intf);
                if (err)
                        goto lbl_free_complete;
        }

        /* test if the dma address was successfully delivered to the interface */
        if (ixxat_pci_adapter->intf_test_dma) {
                err = ixxat_pci_adapter->intf_test_dma(dev, intf);
                if (err)
                        goto lbl_free_complete;
        }

        /* upload the device's firmware */
        if (ixxat_pci_adapter->intf_upload_firmware) {
                err = ixxat_pci_adapter->intf_upload_firmware(dev, intf);
                if (err)
                        goto lbl_free_complete;
        }

        /* start uploaded firmware */
        if (ixxat_pci_adapter->intf_start_fw) {
                err = ixxat_pci_adapter->intf_start_fw(dev, intf);
                if (err)
                        goto lbl_free_complete;

                /* give device some time to start */
                msleep(100);
        }

        /* get the device's capabilities */
        if (ixxat_pci_adapter->intf_get_caps) {
                err = ixxat_pci_adapter->intf_get_caps(dev, intf, &intf_caps,
                                0);
                if (err)
                        goto lbl_free_complete;
        }

        for (i = 0; i < intf_caps.bus_ctrl_count; i++) {
                if ( IXX_PCI_BUS_CAN
                                == IXXAT_PCI_BUS_TYPE(
                                                intf_caps.bus_ctrl_types[i])) {
                        ixxat_pci_adapter->ctrl_count++;
                        err = ixxat_pci_create_dev(ixxat_pci_adapter, dev, intf,
                                        i);
                }
                if (err) {
                        /* deregister already created devices */
                        ixxat_pci_disconnect(dev);
                        goto lbl_free_complete;
                }
        }

        return err;

        lbl_free_complete:

        if (ixxat_pci_adapter->intf_convey_dma)
                ixxat_pci_adapter->intf_convey_dma(dev, intf, 1);

        if (ixxat_pci_adapter->intf_int_ena)
                ixxat_pci_adapter->intf_int_ena(intf, 0);

        lbl_free_memory:
        if (dev->irq)
                free_irq(dev->irq, intf);

        pci_disable_msi(dev);

        if (intf->addmemvadd) {
                kfree(intf->addmemvadd);
                intf->addmemvadd = NULL;
        }

        if (intf->dmavadd)
                pci_free_consistent(dev, intf->dmalen, intf->dmavadd,
                                intf->dmaadd);

        iounmap(intf->reg1vadd);
        release_mem_region(intf->reg1add, intf->reg1len);

        iounmap(intf->memvadd);
        release_mem_region(intf->memadd, intf->memlen);

        lbl_set_intf:
        pci_disable_device(dev);

        kfree(intf);

        return err;
}

/* pci specific object needed to register this driver with the pci subsystem */
static struct pci_driver ixx_pci_driver = { .name = IXXAT_PCI_DRIVER_NAME,
                .remove = ixxat_pci_disconnect, .probe = ixxat_pci_probe,
                .id_table = ixxat_pci_table, };

static int __init ixxat_pci_init(void)
{
        int err;

        /* register this driver with the pci subsystem */
        err = pci_register_driver(&ixx_pci_driver);
        if (err)
                pr_err("%s: pci_register failed (err %d)\n",
                                IXXAT_PCI_DRIVER_NAME, err);

        return err;
}

static void __exit ixxat_pci_exit(void)
{
        int err;

        /* last chance do send any synchronous commands here */
        err = driver_for_each_device(&ixx_pci_driver.driver, NULL,
        NULL, ixxat_pci_do_device_exit);
        if (err)
                pr_err("%s: failed to stop all can devices (err %d)\n",
                                IXXAT_PCI_DRIVER_NAME, err);

        /* deregister this driver with the pci subsystem */
        pci_unregister_driver(&ixx_pci_driver);

        pr_info("%s: IXX-pci interfaces driver unloaded\n",
                        IXXAT_PCI_DRIVER_NAME);
}

module_init(ixxat_pci_init);
module_exit(ixxat_pci_exit);
