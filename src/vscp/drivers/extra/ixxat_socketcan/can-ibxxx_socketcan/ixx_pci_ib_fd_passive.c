/*
 * CAN driver for IXXAT CAN-IB5X0 and CAN-IB7X0 adapters
 *
 * Copyright (C) 2003-2015 Michael Hengler IXXAT Automation GmbH
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

#include <asm-generic/errno.h>

#include <linux/netdevice.h>

#include <linux/can.h>
#include <linux/can/dev.h>
#include <linux/can/error.h>
#include <linux/delay.h>
#include <linux/gfp.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/types.h>

#include "ififd.h"
#include "ixx_pci_core.h"

#ifdef CANFD_CAPABLE

MODULE_SUPPORTED_DEVICE("IXXAT Automation GmbH CAN-IB5X0 and CAN-IB7X0 interfaces");

/* IFI-CANFD internal clock (Hz) */
#define IFIFD_CANFD_CLOCK_HZ 80000000

static void ixx_pas_ib_fd_xxx_gpio_set_pin(u8* gpio_base, u32 pin, u8 val)
{
        u8 act_val = 0;
        u8 mask = (0x01 << pin);

        if (!gpio_base)
                return;

        act_val = le32_to_cpu(ioread32(gpio_base));

        if (val == 0) {
                act_val &= ~mask;
        } else {
                act_val |= mask;
        }

        iowrite32(cpu_to_le32(act_val), gpio_base);
}

static void ixx_pas_ib_fd_xxx_gpio_write_byte(u8* gpio_base, u8 bData)
{
        int i;
        for (i = (sizeof(u8) * 8) - 1; (i >= 0); i--) {
                ixx_pas_ib_fd_xxx_gpio_set_pin(gpio_base,
                CAN_IB_BSP_FPGA_SCF_CLK, 0);
                ixx_pas_ib_fd_xxx_gpio_set_pin(gpio_base,
                CAN_IB_BSP_FPGA_SCF_SDI, (bData >> i) & 0x01);
                ixx_pas_ib_fd_xxx_gpio_set_pin(gpio_base,
                CAN_IB_BSP_FPGA_SCF_CLK, 1);
        }
}

static u8 ixx_pas_ib_fd_xxx_gpio_read_byte(u8* gpio_base)
{
        u8 bData = 0;
        int i;

        for (i = (sizeof(u8) * 8) - 1; (i >= 0); i--) {
                ixx_pas_ib_fd_xxx_gpio_set_pin(gpio_base,
                CAN_IB_BSP_FPGA_SCF_CLK, 1);
                ixx_pas_ib_fd_xxx_gpio_set_pin(gpio_base,
                CAN_IB_BSP_FPGA_SCF_CLK, 0);
                bData |= ((((le32_to_cpu(ioread32(gpio_base)))
                                >> CAN_IB_BSP_FPGA_SCF_SDO) & 0x01) << i);
        }

        return (bData);
}

static int ixx_pas_ib_fd_xxx_read_serial(struct ixx_pci_interface* intf,
                u8 max_size, char* hw_serial)
{
        int i;
        u8* gpio_base;
        u32 sect_addr = 0;
        u32 sect_size = CAN_IB_INFO_SECTOR_SIZE;

        gpio_base = intf->memvadd + CAN_IB_GPIOSPI_BASE;

        sect_addr = sect_size * CAN_IB_INFO_SECTOR;
        sect_addr += CAN_IB_INFO_OFFSET_SERIAL;

        ixx_pas_ib_fd_xxx_gpio_set_pin(gpio_base, CAN_IB_BSP_FPGA_SCF_NCS, 0);
        ixx_pas_ib_fd_xxx_gpio_write_byte(gpio_base, CAN_IB_EPCS_READ_BYTES);
        ixx_pas_ib_fd_xxx_gpio_write_byte(gpio_base, (sect_addr >> 16) & 0xFF);
        ixx_pas_ib_fd_xxx_gpio_write_byte(gpio_base, (sect_addr >> 8) & 0xFF);
        ixx_pas_ib_fd_xxx_gpio_write_byte(gpio_base, (sect_addr >> 0) & 0xFF);

        for (i = 0; i < (CAN_IB_HWSERIAL_SIZE - 1); i++) {
                *hw_serial++ = ixx_pas_ib_fd_xxx_gpio_read_byte(gpio_base);
        }

        ixx_pas_ib_fd_xxx_gpio_set_pin(gpio_base, CAN_IB_BSP_FPGA_SCF_NCS, 1);

        if (hw_serial[0] == ((char) 0xFF))
                return -EIO;

        return 0;
}

static int ixx_pas_ib_fd_xxx_get_dev_info(struct ixx_pci_interface* intf,
                struct ixx_intf_info* dev_info)
{
        int err;

        /* Reset return structure */
        memset(dev_info, 0, sizeof(struct ixx_intf_info));

        /* Copy board name */
        memcpy(dev_info->intf_name, (intf->memvadd + CAN_IB_CARDNAME_BASE),
                        min(sizeof(dev_info->intf_name) - 1,
                                        (size_t) CAN_IB_CARDNAME_SIZE));

        /* Copy Version Info */
        dev_info->intf_version = *(u16*) (intf->memvadd + CAN_IB_HWVER_BASE);
        dev_info->intf_fpga_version = *(u32*) (intf->memvadd
                        + CAN_IB_FPGAVER_BASE);

        /* Read hardware serial number */
        err = ixx_pas_ib_fd_xxx_read_serial(intf, sizeof(dev_info->intf_id) - 1,
                        dev_info->intf_id);

        return err;
}

static int ixx_pas_ib_fd_xxx_clear_int(struct ixx_pci_priv *priv)
{
        u32 ifi_reg_status;
        u32 ifi_reg_hidden_error;

        ifi_reg_status = le32_to_cpu(
                        ioread32(priv->ifi_base + IFIFDREG_INTPEND));
        iowrite32(cpu_to_le32(ifi_reg_status),
                        priv->ifi_base + IFIFDREG_INTPEND);

        ifi_reg_hidden_error = IFIFD_R17_WR_ERR_RST | IFIFD_R17_WR_OVR_RST
                        | IFIFD_R17_WR_ERF_RST | IFIFD_R17_WR_ERR_ENA;

        iowrite32(cpu_to_le32(ifi_reg_hidden_error),
                        priv->ifi_base + IFIFDREG_ERROR_HIDDEN);

        return 0;
}

static int ixx_pas_ib_fd_xxx_enable_int(struct ixx_pci_priv *priv, u8 enable,
                u8 clear)
{
        int err = 0;
        u32 ifi_int_mask = 0;

        priv->interrupts_enabled = enable;

        /* Set flags to change IFI CAN interrupts */
        ifi_int_mask |= IFIFD_R4_WR_SET_TX_IRQ;
        ifi_int_mask |= IFIFD_R4_WR_SET_RX_IRQ;
        ifi_int_mask |= IFIFD_R4_WR_SET_TIM_IRQ;
        ifi_int_mask |= IFIFD_R4_WR_SET_ERR_IRQ;

        /* Must enable IFI CAN interrupts */
        if (enable) {
                ifi_int_mask |= IFIFD_IRQ_RFIFO_NEMPTY
                                | IFIFD_IRQ_RFIFO_OVERFLOW;
                ifi_int_mask |= IFIFD_IRQ_TFIFO_OK | IFIFD_IRQ_TFIFO_OVERFLOW;
                ifi_int_mask |= IFIFD_IRQ_EW | IFIFD_IRQ_BO;
                ifi_int_mask |= IFIFD_IRQ_ERR_INFO;
        }

        iowrite32(cpu_to_le32(ifi_int_mask), priv->ifi_base + IFIFDREG_INTMASK);

        if (clear) {
                err = ixx_pas_ib_fd_xxx_clear_int(priv);
                if (err)
                        return err;
        }

        return err;
}

#ifdef _REAL_LOOPBACK
static int ixx_pas_ib_fd_xxx_handle_srr_canmsg(struct ixx_pci_priv *priv)
{
        u8 frn;
        u32 ifi_reg_dlc;
        u32 timestamp;

        ifi_reg_dlc = le32_to_cpu(
                        ioread32(priv->ifi_base + IFIFDREG_RX_FIFO_DLC));

        priv->netdev->stats.tx_packets += 1;
        priv->netdev->stats.tx_bytes += can_dlc2len(
                        ifi_reg_dlc & IFIFD_RXFIFO_DLC);
        frn = (ifi_reg_dlc & IFIFD_RXFIFO_FRN) >> IFIFD_RXFIFO_FRN_S;

        timestamp = le32_to_cpu(ioread32(priv->ifi_base + IFIFDREG_RX_FIFO_TS));

        return ixxat_pci_handle_frn(priv, frn, timestamp);
}
#endif

static int ixx_pas_ib_fd_xxx_handle_canmsg(struct ixx_pci_priv *priv)
{
        struct net_device *netdev = priv->netdev;
        struct canfd_frame *can_frame;
        struct sk_buff *skb;
        u32 raw_id, raw_dlc, timestamp;
        u32* data_dst;
        int i, j, err;

        skb = alloc_canfd_skb(netdev, &can_frame);
        if (!skb)
                return -ENOMEM;

        data_dst = (u32*) can_frame->data;

        raw_id = le32_to_cpu(ioread32(priv->ifi_base + IFIFDREG_RX_FIFO_ID));
        raw_dlc = le32_to_cpu(ioread32(priv->ifi_base + IFIFDREG_RX_FIFO_DLC));

        can_frame->len = can_dlc2len(
                        le32_to_cpu(
                                        (raw_dlc & IFIFD_RXFIFO_DLC)
                                                        >> IFIFD_RXFIFO_DLC_S));

        if (raw_id & IFIFD_RXFIFO_IDE) {
                can_frame->can_id |= CAN_EFF_FLAG;
                can_frame->can_id |= ((raw_id & IFIFD_RXFIFO_IDEXT_18_28) << 18)
                                + ((raw_id & IFIFD_RXFIFO_IDEXT_00_17) >> 11);
        } else {
                can_frame->can_id |= (raw_id & IFIFD_RXFIFO_IDSTD)
                                >> IFIFD_RXFIFO_IDSTD_S;
                can_frame->can_id &= ~CAN_EFF_FLAG;
        }
        if (raw_dlc & IFIFD_RXFIFO_RTR)
                can_frame->can_id |= CAN_RTR_FLAG;
        else {
                for (i = 0, j = 0; i < can_frame->len;
                                ++j, i = i + sizeof(u32)) {
                        data_dst[j] = ioread32(
                                        priv->ifi_base + IFIFDREG_RX_FIFO_DATA14
                                                        + j);
                        data_dst[j] = le32_to_cpu(data_dst[j]);
                }
                if (raw_dlc & IFIFD_RXFIFO_BRS)
                        can_frame->flags |= CANFD_BRS;
                if (raw_dlc & IFIFD_RXFIFO_ESI)
                        can_frame->flags |= CANFD_ESI;
        }

        timestamp = ioread32(priv->ifi_base + IFIFDREG_RX_FIFO_TS);
        timestamp = le32_to_cpu(timestamp);
        ixxat_pci_get_ts_tv(priv, timestamp, &skb->tstamp);

        err = netif_receive_skb(skb);
        if (err) {
                netdev_err(priv->netdev,
                                "netif_receive_skb failed with error code %d",
                                err);
                return 0;
        }

        netdev->stats.rx_packets++;
        netdev->stats.rx_bytes += can_frame->len;

        return 1;
}

static int ixx_pas_ib_fd_xxx_handle_error(struct ixx_pci_priv *priv)
{
        struct net_device *netdev = priv->netdev;
        struct can_frame *can_frame;
        struct sk_buff *skb;
        u8 raw_status = 0;
        u32 timestamp;

        /* nothing should be sent while in BUS_OFF state */
        if (priv->can.state == CAN_STATE_BUS_OFF)
                return 0;

        raw_status = le32_to_cpu(
                        ioread32(priv->ifi_base + IFIFDREG_ERROR_HIDDEN));

        /* allocate an skb to store the error frame */
        skb = alloc_can_err_skb(netdev, &can_frame);
        if (!skb)
                return -ENOMEM;

        switch (raw_status) {
        case IFIFD_R17_CANERR_ACK:
                can_frame->can_id |= CAN_ERR_ACK;
                netdev->stats.tx_errors++;
                break;
        case IFIFD_R17_CANERR_BIT0:
                can_frame->can_id |= CAN_ERR_PROT;
                can_frame->data[2] |= CAN_ERR_PROT_BIT0;
                netdev->stats.rx_errors++;
                break;
        case IFIFD_R17_CANERR_BIT1:
                can_frame->can_id |= CAN_ERR_PROT;
                can_frame->data[2] |= CAN_ERR_PROT_BIT1;
                netdev->stats.rx_errors++;
                break;
        case IFIFD_R17_CANERR_CRC:
                can_frame->can_id |= CAN_ERR_PROT;
                can_frame->data[3] |= CAN_ERR_PROT_LOC_CRC_SEQ;
                netdev->stats.rx_errors++;
                break;
        case IFIFD_R17_CANERR_FORM:
                can_frame->can_id |= CAN_ERR_PROT;
                can_frame->data[2] |= CAN_ERR_PROT_FORM;
                netdev->stats.rx_errors++;
                break;
        case IFIFD_R17_CANERR_STUFF:
                can_frame->can_id |= CAN_ERR_PROT;
                can_frame->data[2] |= CAN_ERR_PROT_STUFF;
                netdev->stats.rx_errors++;
                break;
        case IFIFD_R17_CANERR_FAST:
                can_frame->can_id |= CAN_ERR_PROT;
                can_frame->data[2] |= CAN_ERR_PROT_STUFF;
                netdev->stats.rx_errors++;
                break;
        default:
                can_frame->can_id |= CAN_ERR_PROT;
                netdev->stats.rx_errors++;
        }

        timestamp = ioread32(priv->ifi_base + IFIFDREG_TIM_ACTUAL);
        timestamp = le32_to_cpu(timestamp);
        ixxat_pci_get_ts_tv(priv, timestamp, &skb->tstamp);

        netif_receive_skb(skb);
        netdev->stats.rx_packets++;
        netdev->stats.rx_bytes += can_frame->can_dlc;

        return 1;
}

static int ixx_pas_ib_fd_xxx_receive_message(struct ixx_pci_priv *priv,
                u32 ifi_reg_status)
{
        u32 ifi_reg_dlc, ifi_reg_fifo, ifi_reg_err;
        u32 result = 0;
        u32 rx_msg_count;

        ifi_reg_fifo = ioread32(priv->ifi_base + IFIFDREG_RX_STSCMD);
        ifi_reg_fifo = le32_to_cpu(ifi_reg_fifo);

        rx_msg_count = (ifi_reg_fifo & IFIFD_R1_RD_RBC) >> IFIFD_R1_RBC_S;

        ifi_reg_err = ioread32(priv->ifi_base + IFIFDREG_ERROR_HIDDEN);
        ifi_reg_err = le32_to_cpu(ifi_reg_err);

        if (rx_msg_count) {
                ifi_reg_dlc = ioread32(priv->ifi_base + IFIFDREG_RX_FIFO_DLC);
                ifi_reg_dlc = le32_to_cpu(ifi_reg_dlc);
                if (ifi_reg_dlc & IFIFD_RXFIFO_FRN) {
                        /* handle self reception messages */
#ifdef _REAL_LOOPBACK
                        result += ixx_pas_ib_fd_xxx_handle_srr_canmsg(priv);
#else
                        result += ixx_pas_ib_fd_xxx_handle_canmsg(priv);
#endif
                        iowrite32(cpu_to_le32(IFIFD_R1_WR_REM_MSG),
                                        priv->ifi_base + IFIFDREG_RX_STSCMD);
                } else {
                        /* handle can messages */
                        result += ixx_pas_ib_fd_xxx_handle_canmsg(priv);
                        iowrite32(cpu_to_le32(IFIFD_R1_WR_REM_MSG),
                                        priv->ifi_base + IFIFDREG_RX_STSCMD);
                }
        } else if (ifi_reg_err & IFIFD_R17_RD_BIT_POS) {
                /* handle error messages */
                result += ixx_pas_ib_fd_xxx_handle_error(priv);
                ifi_reg_err = IFIFD_R17_WR_ERR_ENA | IFIFD_R17_WR_ERR_RST
                                | IFIFD_R17_WR_ERF_RST | IFIFD_R17_WR_OVR_RST;
                ifi_reg_err = cpu_to_le32(ifi_reg_err);
                iowrite32(ifi_reg_err, priv->ifi_base + IFIFDREG_ERROR_HIDDEN);
        }

        return result;
}

static int ixx_pas_ib_fd_xxx_handle_status(struct ixx_pci_priv *priv,
                u32 ifi_reg_status)
{
        struct sk_buff *skb;
        struct can_frame *can_frame;
        u32 ifi_reg_cmd, ifi_reg_txcmd, ifi_reg_intpend;
        unsigned long spin_flags;
#ifdef _REAL_LOOPBACK
        u32 tmp_read;
#endif

        if (priv->can.state == CAN_STATE_BUS_OFF)
                return -ENETDOWN;

        ifi_reg_cmd = le32_to_cpu(ioread32(priv->ifi_base + IFIFDREG_STSCMD));
        ifi_reg_txcmd = le32_to_cpu(
                        ioread32(priv->ifi_base + IFIFDREG_TX_STSCMD));
        ifi_reg_intpend = le32_to_cpu(
                        ioread32(priv->ifi_base + IFIFDREG_INTPEND));

        if (ifi_reg_status & IFIFD_IRQ_BO) {
                /* allocate an skb to store the error frame */
                skb = alloc_can_err_skb(priv->netdev, &can_frame);
                if (!skb)
                        return -ENOMEM;

                priv->can.state = CAN_STATE_BUS_OFF;
                priv->can.can_stats.bus_off++;
                can_bus_off(priv->netdev);

                can_frame->can_id |= CAN_ERR_BUSOFF;
                netif_receive_skb(skb);
                priv->netdev->stats.rx_packets++;
                priv->netdev->stats.rx_bytes += can_frame->can_dlc;

                return 1;
        }
        if (ifi_reg_status & IFIFD_IRQ_EW)
                priv->can.state = CAN_STATE_ERROR_WARNING;
        if (ifi_reg_cmd & IFIFD_R0_RD_ERRACT)
                priv->can.state = CAN_STATE_ERROR_ACTIVE;
        if (ifi_reg_cmd & IFIFD_R0_RD_ERRPAS)
                priv->can.state = CAN_STATE_ERROR_PASSIVE;
        if (ifi_reg_status & IFIFD_IRQ_RFIFO_OVERFLOW) {
#ifdef _REAL_LOOPBACK
                spin_lock_irqsave(&priv->rcv_lock, spin_flags);
                tmp_read = priv->frn_read + 1;
                if (tmp_read > IXX_PCI_MAX_TX_TRANS)
                        tmp_read = 1;

                if (tmp_read != priv->frn_write) {
                        can_free_echo_skb(priv->netdev, priv->frn_read - 1);
                        priv->frn_read = tmp_read;
                }
                spin_unlock_irqrestore(&priv->rcv_lock, spin_flags);
#endif
                priv->netdev->stats.rx_over_errors++;
        }
        if (ifi_reg_txcmd & IFIFD_R2_RD_OVERFLOW)
                priv->netdev->stats.tx_dropped++;

        return 0;
}

static int ixx_pas_ib_fd_xxx_start_xmit(struct sk_buff *skb,
                struct net_device *netdev)
{
        struct ixx_pci_priv* priv = netdev_priv(netdev);
        struct canfd_frame *cf = (struct canfd_frame *) skb->data;
        u32 __iomem * data_dst = priv->ifi_base + IFIFDREG_TX_FIFO_DATA14;
        u32 can_id, can_dlc, tx_reg;
        int i, j;
        unsigned long spin_flags;

        can_dlc = 0;

        if (can_dropped_invalid_skb(netdev, skb)) {
                netdev_info(netdev, "dropped invalid skb\n");
                return NETDEV_TX_OK;
        }

        if (priv->frn_write == priv->frn_read
                        || (ioread32(priv->ifi_base + IFIFDREG_TX_STSCMD)
                                        & IFIFD_R2_RD_FULL)) {
                /* should not occur except during restart */
                return NETDEV_TX_BUSY;
        }

        if (cf->can_id & CAN_EFF_FLAG) {
                can_id = (((cf->can_id & 0x0003FFFF) << 11)
                                + ((cf->can_id & 0x1FFC0000) >> 18))
                                | IFIFD_TXFIFO_IDE;
        } else {
                can_id = cf->can_id & IFIFD_TXFIFO_IDSTD;
        }

        if (cf->can_id & CAN_RTR_FLAG)
                can_dlc |= IFIFD_TXFIFO_RTR;

        can_dlc |= can_len2dlc(cf->len) & IFIFD_TXFIFO_DLC;

        if (skb->len == CANFD_MTU) {
                can_dlc |= IFIFD_TXFIFO_EDL;

                if (!(cf->can_id & CAN_RTR_FLAG) && (cf->flags & CANFD_BRS)) {
                        can_dlc |= IFIFD_TXFIFO_BRS | IFIFD_TXFIFO_EDL;
                }
        }

        if (skb->pkt_type == PACKET_LOOPBACK) {
#ifdef _REAL_LOOPBACK
                spin_lock_irqsave(&priv->rcv_lock, spin_flags);
                if (priv->can.echo_skb[priv->frn_write - 1]) {
                        can_free_echo_skb(priv->netdev, priv->frn_write - 1);
                }

                can_put_echo_skb(skb, priv->netdev, priv->frn_write - 1);
                can_dlc |= ((priv->frn_write << IFIFD_TXFIFO_FRN_S)
                                & IFIFD_TXFIFO_FRN);

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

        if (!(cf->can_id & CAN_RTR_FLAG)) {
                for (i = 0, j = 0; i < cf->len; ++j, i = i + sizeof(u32)) {
                        iowrite32(cpu_to_le32(*((u32*) &cf->data[i])),
                                        &data_dst[j]);
                }
        }

        iowrite32(cpu_to_le32(can_id), priv->ifi_base + IFIFDREG_TX_FIFO_ID);
        iowrite32(cpu_to_le32(can_dlc), priv->ifi_base + IFIFDREG_TX_FIFO_DLC);
        iowrite32(cpu_to_le32(IFIFD_R2_WR_ADD_MSG),
                        priv->ifi_base + IFIFDREG_TX_STSCMD);

        /* prevent tx timeout */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
        netif_trans_update(netdev);
#else
        netdev->trans_start = jiffies;
#endif
        spin_lock_irqsave(&priv->rcv_lock, spin_flags);
        tx_reg = le32_to_cpu(ioread32(priv->ifi_base + IFIFDREG_TX_STSCMD));
        if ((tx_reg & IFIFD_R2_RD_FULL)
                        || (priv->frn_write == priv->frn_read)) {
                netif_stop_queue(netdev);
        }
        spin_unlock_irqrestore(&priv->rcv_lock, spin_flags);

        return NETDEV_TX_OK;
}

static int ixx_pas_ib_fd_xxx_get_intf_caps(struct pci_dev *priv,
                struct ixx_pci_interface * intf,
                struct ixx_intf_caps* intf_caps, u8 abs_idx)
{
        int i;
        u8 has_expansion_board = 0;
        char card_name[CAN_IB_CARDNAME_SIZE];
        u32* __iomem var_reg;
        u32 variant;
        u8 is_high_speed;
        u8 is_low_speed;

        /* Reset return structure */
        memset(intf_caps, 0, sizeof(*intf_caps));

        /* Show Board Info */
        memcpy_fromio(card_name, intf->memvadd + CAN_IB_CARDNAME_BASE,
                        sizeof(card_name) - 1);

        dev_dbg(&priv->dev, "Card Name     : %s\n", card_name);
        dev_dbg(&priv->dev, "HW Version    : 0x%04X\n",
                        *(u16*) (intf->memvadd + CAN_IB_HWVER_BASE));
        dev_dbg(&priv->dev, "FPGA Version  : 0x%08X\n",
                        *(u32*) (intf->memvadd + CAN_IB_FPGAVER_BASE));

        /* Check if expansion board is present */
        if (CAN_IB_SIO_EXPANSION
                        == (ioread32(intf->memvadd + CAN_IB_SYNCIO_BASE)
                                        & CAN_IB_SIO_EXPANSION)) {
                has_expansion_board = 1;
        }

        /* Iterate over all variant register */
        for (i = 0; i < CAN_IB_VARIANT_COUNT; i++) {
                var_reg = NULL;
                variant = 0;
                is_high_speed = 0;
                is_low_speed = 0;

                /* Select correct variant register */
                var_reg = (u32*) (intf->memvadd + CAN_IB_VARIANT_BASE +
                CAN_IB_VARIANT_STEP * i);
                variant = ioread32(var_reg);

                /* Check if CAN HighSpeed is present on extension board, if available */
                /* ECI_IB5X0_VC_CANHS is set always for CAN3 and CAN4 */
                if ((i < 2) || (has_expansion_board)) {
                        /* Check if CAN HighSpeed is present */
                        if (variant & CAN_IB_VC_CANHS) {
                                is_high_speed = 1;
                        }
                }

                /* Check what type of extension board is attached */
                variant = ~(variant >> 1);
                variant &= CAN_IB_VC_MASK;
                switch (variant) {
                case CAN_IB_VC_CANLS:
                case CAN_IB_VC_CANLS_LIN:
                case CAN_IB_VC_CANLS_KLINE:
                        is_low_speed = 1;
                        break;
                default:
                        break;
                }

                /* CAN Controller */
                if ((is_high_speed | is_low_speed)
                                && (ARRAY_SIZE(intf_caps->bus_ctrl_types)
                                                > intf_caps->bus_ctrl_count)) {
                        intf_caps->bus_ctrl_types[intf_caps->bus_ctrl_count++] =
                        IXX_PCI_BUS_CAN << 8;
                }
        }

        return 0;
}

static int ixx_pas_ib_fd_xxx_set_filter(struct ixx_pci_priv* priv, u8 enable)
{
        int i;
        u32 filter_num, comp_para;
        u32 *filter_id = kzalloc(sizeof(u32) * IFIFDREG_FILTERCOUNT,
        GFP_KERNEL);
        u32 *filter_mask = kzalloc(sizeof(u32) * IFIFDREG_FILTERCOUNT,
        GFP_KERNEL);

        /* Filter size LUT */
        comp_para = ioread32(priv->ifi_base + IFIFDREG_COMP_PARA);
        comp_para = le32_to_cpu(comp_para);
        comp_para &= IFIFD_R18_RD_NO_FILTER;

        if (comp_para)
                filter_num = 0;
        else
                filter_num = IFIFDREG_FILTERCOUNT;

        if (enable) {
                filter_mask[0] = IFIFD_R512_RD_FMVALID | IFIFD_R512_RD_FMEXT;
                filter_id[0] = IFIFD_R513_RD_FIDVALID;
                filter_mask[1] = IFIFD_R512_RD_FMVALID | IFIFD_R512_RD_FMEXT;
                filter_id[1] = IFIFD_R513_RD_FIDVALID | IFIFD_R513_RD_FIDEXT;
        }

        for (i = 0; i < filter_num; ++i) {
                iowrite32(cpu_to_le32(filter_mask[i]),
                                priv->ifi_base + IFIFDREG_FILTERMASK + (i * 2));
                iowrite32(cpu_to_le32(filter_id[i]),
                                priv->ifi_base + IFIFDREG_FILTERID + (i * 2));
        }

        kfree(filter_id);
        kfree(filter_mask);

        return 0;
}

static int ixx_pas_ib_fd_xxx_start_ctrl(struct ixx_pci_priv* priv,
                u32 * time_ref)
{
        int err;
        u32 ifi_sts_cmd = 0;

        ifi_sts_cmd = le32_to_cpu(ioread32(priv->ifi_base + IFIFDREG_STSCMD));
        ifi_sts_cmd |= IFIFD_R0_WR_N_MODE_START | IFIFD_R0_ENA;

        *time_ref = le32_to_cpu(ioread32(priv->ifi_base + IFIFDREG_TIM_ACTUAL));

        iowrite32(cpu_to_le32(ifi_sts_cmd), priv->ifi_base + IFIFDREG_STSCMD);

        err = ixx_pas_ib_fd_xxx_set_filter(priv, 1);
        if (err)
                return err;

        return err;
}

static int ixx_pas_ib_fd_xxx_set_opmode(struct ixx_pci_priv* priv, u8 mode)
{
        u32 ifi_sts_cmd;

        ifi_sts_cmd = le32_to_cpu(ioread32(priv->ifi_base + IFIFDREG_STSCMD));

        if (CAN_CTRLMODE_LISTENONLY & mode)
                ifi_sts_cmd |= IFIFD_R0_BUSMON;
        else
                ifi_sts_cmd &= ~IFIFD_R0_BUSMON;

        if ((CAN_CTRLMODE_FD | CAN_CTRLMODE_FD_NON_ISO) & mode)
                ifi_sts_cmd &= ~IFIFD_R0_CANFD_DIS;
        else
                ifi_sts_cmd |= IFIFD_R0_CANFD_DIS;

        if (CAN_CTRLMODE_FD_NON_ISO & mode)
                ifi_sts_cmd &= ~IFIFD_R0_ISO_ENA;
        else
                ifi_sts_cmd |= IFIFD_R0_ISO_ENA;

        iowrite32(cpu_to_le32(ifi_sts_cmd), priv->ifi_base + IFIFDREG_STSCMD);

        return 0;
}

static int ixx_pas_ib_fd_xxx_set_one_shot(struct ixx_pci_priv* priv,
                u8 one_shot)
{
        u32 ifi_tx_repeat_count;

        if (one_shot)
                ifi_tx_repeat_count = 1;
        else
                ifi_tx_repeat_count = 0;

        iowrite32(cpu_to_le32(ifi_tx_repeat_count),
                        priv->ifi_base + IFIFDREG_TRA_REPCOUNT);

        return 0;
}

static int ixx_pas_ib_fd_xxx_set_berr_reporting(struct ixx_pci_priv* priv,
                u8 berr_reporting)
{
        u32 ifi_error_detail;

        if (berr_reporting)
                ifi_error_detail = (IFIFD_R17_WR_ERR_ENA | IFIFD_R17_WR_ERR_RST
                                | IFIFD_R17_WR_ERF_RST | IFIFD_R17_WR_OVR_RST);
        else
                ifi_error_detail = (IFIFD_R17_WR_ERR_RST | IFIFD_R17_WR_ERF_RST
                                | IFIFD_R17_WR_OVR_RST);

        iowrite32(cpu_to_le32(ifi_error_detail),
                        priv->ifi_base + IFIFDREG_ERROR_HIDDEN);

        return 0;
}

static int ixx_pas_ib_fd_xxx_stop_ctrl(struct ixx_pci_priv* priv)
{
        int err = 0, i;
        u32 ifi_sts_cmd = 0, ifi_tx_cmd, ifi_rx_cmd;

        ifi_sts_cmd = le32_to_cpu(ioread32(priv->ifi_base + IFIFDREG_STSCMD));
        ifi_sts_cmd &= ~IFIFD_R0_ENA;

        iowrite32(cpu_to_le32(ifi_sts_cmd), priv->ifi_base + IFIFDREG_STSCMD);

        err = ixx_pas_ib_fd_xxx_set_opmode(priv,
                        (priv->can.ctrlmode | CAN_CTRLMODE_LISTENONLY));
        err = ixx_pas_ib_fd_xxx_set_filter(priv, 0);

        i = 0;
        do {
                ifi_tx_cmd = cpu_to_le32(
                IFIFD_R2_WR_FIFO_CLEAR | IFIFD_R2_WR_REM_PEND);
                iowrite32(ifi_tx_cmd, priv->ifi_base + IFIFDREG_TX_STSCMD);
                ifi_tx_cmd = le32_to_cpu(
                                ioread32(priv->ifi_base + IFIFDREG_TX_STSCMD));
        } while (!(IFIFD_R2_RD_EMPTY & ifi_tx_cmd) && (1000 > i++));

        i = 0;
        do {
                ifi_rx_cmd = cpu_to_le32(IFIFD_R1_WR_FIFO_CLEAR);
                iowrite32(ifi_rx_cmd, priv->ifi_base + IFIFDREG_RX_STSCMD);
                ifi_rx_cmd = le32_to_cpu(
                                ioread32(priv->ifi_base + IFIFDREG_RX_STSCMD));
        } while (!(IFIFD_R1_RD_EMPTY & ifi_rx_cmd) && (1000 > i++));

        iowrite32(cpu_to_le32(0), priv->ifi_base + IFIFDREG_TX_STSCMD);
        iowrite32(cpu_to_le32(0), priv->ifi_base + IFIFDREG_RX_STSCMD);

        return err;
}

static int ixx_pas_ib_fd_xxx_set_bittiming(struct ixx_pci_priv *priv,
                struct can_bittiming *bt)
{
        u32 ifi_timing_control = 0;

        ifi_timing_control = ((bt->brp - can_ib_500.bittiming_const.brp_min)
                        << IFIFD_BDRT_ISO_PRESCALE_S) & IFIFD_BDRT_ISO_PRESCALE;

        ifi_timing_control |= ((bt->sjw - 1) << IFIFD_BDRT_ISO_SJW_S)
                        & IFIFD_BDRT_ISO_SJW;

        ifi_timing_control |= (((bt->phase_seg1 + bt->prop_seg)
                        - can_ib_500.bittiming_const.tseg1_min)
                        << IFIFD_BDRT_ISO_TIMEA_S) & IFIFD_BDRT_ISO_TIMEA;

        ifi_timing_control |= ((bt->phase_seg2
                        - can_ib_500.bittiming_const.tseg2_min - 1)
                        << IFIFD_BDRT_ISO_TIMEB_S) & IFIFD_BDRT_ISO_TIMEB;

        iowrite32(ifi_timing_control, priv->ifi_base + IFIFDREG_TIMCTRL);

        netdev_dbg(priv->netdev,
                        "setting brp=%d sjw=%d phase_seg1=%d phase_seg2=%d prop_seg=%d bt->tq=%d\n",
                        bt->brp, bt->sjw, bt->phase_seg1, bt->phase_seg2,
                        bt->prop_seg, bt->tq);

        return 0;
}

static int ixx_pas_ib_fd_xxx_set_datatiming(struct ixx_pci_priv *priv,
                struct can_bittiming *bt)
{
        u32 ifi_timing_control = 0;
        u32 ifi_transmitter_delay = 0;

        ifi_timing_control =
                        ((bt->brp - can_ib_500.data_bittiming_const.brp_min)
                                        << IFIFD_BDRT_ISO_PRESCALE_S)
                                        & IFIFD_BDRT_ISO_PRESCALE;

        ifi_timing_control |= ((bt->sjw - 1) << IFIFD_BDRT_ISO_SJW_S)
                        & IFIFD_BDRT_ISO_SJW;

        ifi_timing_control |= (((bt->phase_seg1 + bt->prop_seg)
                        - can_ib_500.data_bittiming_const.tseg1_min)
                        << IFIFD_BDRT_ISO_TIMEA_S) & IFIFD_BDRT_ISO_TIMEA;

        ifi_timing_control |= ((bt->phase_seg2
                        - can_ib_500.data_bittiming_const.tseg2_min - 1)
                        << IFIFD_BDRT_ISO_TIMEB_S) & IFIFD_BDRT_ISO_TIMEB;

        iowrite32(ifi_timing_control, priv->ifi_base + IFIFDREG_TIMCTRL_FAST);

        ifi_transmitter_delay |= (1 + bt->phase_seg1 + bt->prop_seg) * bt->brp;
        ifi_transmitter_delay &= IFIFD_R7_TX_DELAY_ISO_CFG;
        ifi_transmitter_delay = ifi_transmitter_delay
                        << IFIFD_R7_TX_DELAY_ISO_CFG_S;
        ifi_transmitter_delay |= IFIFD_R7_TX_DELAY_ISO_ENA;

        iowrite32(ifi_transmitter_delay, priv->ifi_base + IFIFDREG_TRA_DELAY);

        netdev_dbg(priv->netdev,
                        "data setting brp=%d sjw=%d phase_seg1=%d phase_seg2=%d prop_seg=%d bt->tq=%d\n",
                        bt->brp, bt->sjw, bt->phase_seg1, bt->phase_seg2,
                        bt->prop_seg, bt->tq);

        return 0;
}

static int ixx_pas_ib_fd_xxx_int_ena_req(struct ixx_pci_interface *intf,
                u8 enable)
{
        u8 result = 0;
        unsigned int reg_val = 0;

        reg_val = le32_to_cpu(
                        ioread32(intf->reg1vadd + PCIE_ALTERALCR_A2P_INTENA));

        result = 0 != (reg_val & 0x00000080);

        if (enable)
                iowrite32(cpu_to_le32(reg_val | 0x00000080),
                                intf->reg1vadd + PCIE_ALTERALCR_A2P_INTENA);
        else
                iowrite32(cpu_to_le32(reg_val & 0xFFFFFF7F),
                                intf->reg1vadd + PCIE_ALTERALCR_A2P_INTENA);

        return result;
}

static void ixx_pas_ib_fd_xxx_int_clr_req(struct ixx_pci_interface *intf)
{
        unsigned int reg_val = 0;
        unsigned int status = 0;

        /* read interrupt flags */
        reg_val = le32_to_cpu(
                        ioread32(intf->reg1vadd + PCIE_ALTERA_LCR_INTCSR));

        /* mask relevant sources */
        status = reg_val & 0x00FF0080;

        /* reset interrupt flags */
        iowrite32(cpu_to_le32(reg_val),
                        intf->reg1vadd + PCIE_ALTERA_LCR_INTCSR);

        return;
}

static int ixx_pas_ib_fd_xxx_int_get_stat(struct ixx_pci_interface *intf)
{
        unsigned int reg_val = 0;
        unsigned int result = 0;

        reg_val = le32_to_cpu(
                        ioread32(intf->reg1vadd + PCIE_ALTERA_LCR_INTCSR));
        result = reg_val & 0x00FF0080;

        return result;
}

static irqreturn_t ixx_pas_ib_fd_xxx_irq_handler(int irq, void *dev_id)
{
        struct ixx_pci_interface *intf = (struct ixx_pci_interface *) dev_id;
        struct ixx_pci_priv *priv;
        u8 enable = 0;

        if (!ixx_pas_ib_fd_xxx_int_get_stat(intf))
                return IRQ_NONE;

        /* disable interrupt */
        enable = ixx_pas_ib_fd_xxx_int_ena_req(intf, 0);

        /* clear the hardware interrupt request */
        ixx_pas_ib_fd_xxx_int_clr_req(intf);

        /* Only if interrupt was enabled */
        if (!enable)
                return IRQ_HANDLED;

        for (priv = intf->priv; priv; priv = priv->next_siblings) {
                if (priv->state != IXX_STATE_RUNNING)
                        continue;

                if (priv->interrupts_enabled) {
                        ixx_pas_ib_fd_xxx_enable_int(priv, 0, 0);
                        priv->ifi_reg_shadow_status = ioread32(
                                        priv->ifi_base + IFIFDREG_INTPEND);
                        priv->ifi_reg_shadow_status = le32_to_cpu(
                                        priv->ifi_reg_shadow_status);
                        napi_schedule(&priv->napi);

                        iowrite32(le32_to_cpu(priv->ifi_reg_shadow_status),
                                        priv->ifi_base + IFIFDREG_INTPEND);
                }
        }

        /* enable interrupt */
        ixx_pas_ib_fd_xxx_int_ena_req(intf, 1);

        return IRQ_HANDLED;
}

static int ixx_pas_ib_fd_xxx_napi_rx_poll(struct napi_struct *napi, int quota)
{
        struct net_device *netdev = napi->dev;
        struct ixx_pci_priv *priv = netdev_priv(netdev);
        u32 ifi_reg_status, ifi_rx_cmd;
        int nxpackets = 0, err;
        unsigned long spin_flags;

        ifi_reg_status = priv->ifi_reg_shadow_status;

        err = ixx_pas_ib_fd_xxx_handle_status(priv, ifi_reg_status);
        if (err) {
                napi_complete(&priv->napi);
                if (err > 0)
                        return err;
                return 0;
        }

        while (nxpackets < quota) {
                nxpackets += ixx_pas_ib_fd_xxx_receive_message(priv,
                                ifi_reg_status);
                ifi_rx_cmd = ioread32(priv->ifi_base + IFIFDREG_RX_STSCMD);
                ifi_rx_cmd = le32_to_cpu(ifi_rx_cmd);
                if (ifi_rx_cmd & IFIFD_R1_RD_EMPTY)
                        break;
        }

        spin_lock_irqsave(&priv->rcv_lock, spin_flags);
        if (((ifi_reg_status & IFIFD_IRQ_TFIFO_OK) == IFIFD_IRQ_TFIFO_OK)
                        && priv->frn_read != priv->frn_write
                        && netif_queue_stopped(netdev)) {
                netif_wake_queue(priv->netdev);
        }
        spin_unlock_irqrestore(&priv->rcv_lock, spin_flags);

        if (nxpackets < quota) {
                netdev_dbg(priv->netdev,
                                "go back to interrupt mode nxpackets %d",
                                nxpackets);
                napi_complete(&priv->napi);
                ixx_pas_ib_fd_xxx_enable_int(priv, 1, 0);
        }

        return nxpackets;
}

static int ixx_pas_ib_fd_xxx_start(struct ixx_pci_priv *priv)
{
        int err;
        u32 time_ref;

        err = ixx_pas_ib_fd_xxx_set_opmode(priv, priv->can.ctrlmode);
        if (err)
                return err;

        err = ixx_pas_ib_fd_xxx_set_one_shot(priv,
                        priv->can.ctrlmode & CAN_CTRLMODE_ONE_SHOT);
        if (err)
                return err;

        err = ixx_pas_ib_fd_xxx_set_berr_reporting(priv,
                        priv->can.ctrlmode & CAN_CTRLMODE_BERR_REPORTING);
        if (err)
                return err;

        /* opening first device: */
        if (priv->ctrl_opened_count == 0) {
                priv->frn_read = 0x1;
                priv->frn_write = 0x2;

                err = ixx_pas_ib_fd_xxx_start_ctrl(priv, &time_ref);
                if (err)
                        return err;

                ixxat_pci_set_ts_now(priv, time_ref);
                priv->can.state = CAN_STATE_ERROR_ACTIVE;
                priv->state = IXX_STATE_RUNNING;

                napi_enable(&priv->napi);
                err = ixx_pas_ib_fd_xxx_enable_int(priv, 1, 1);
        }
        priv->ctrl_opened_count++;

        return err;
}

static int ixx_pas_ib_fd_xxx_stop(struct ixx_pci_priv *priv)
{
        int err;

        if (priv->ctrl_opened_count == 1) {
                err = ixx_pas_ib_fd_xxx_stop_ctrl(priv);
                if (err)
                        return err;

                priv->can.state = CAN_STATE_STOPPED;
                priv->state = IXX_STATE_STOPPED;
                napi_disable(&priv->napi);

                err = ixx_pas_ib_fd_xxx_enable_int(priv, 0, 1);

                priv->frn_read = 0x1;
                priv->frn_write = 0x2;
        }

        priv->ctrl_opened_count--;

        return 0;
}

static int ixx_pas_ib_fd_xxx_restart(struct ixx_pci_priv *priv)
{
        int err, i;
        u32 time_ref;

        err = ixx_pas_ib_fd_xxx_stop_ctrl(priv);
        if (err)
                return err;

        priv->frn_read = 0x1;
        priv->frn_write = 0x2;

        for (i = 0; i < priv->can.echo_skb_max; ++i)
                can_free_echo_skb(priv->netdev, i);

        netif_wake_queue(priv->netdev);

        err = ixx_pas_ib_fd_xxx_set_opmode(priv, priv->can.ctrlmode);
        if (err)
                return err;

        err = ixx_pas_ib_fd_xxx_set_one_shot(priv,
                        priv->can.ctrlmode & CAN_CTRLMODE_ONE_SHOT);
        if (err)
                return err;

        err = ixx_pas_ib_fd_xxx_set_berr_reporting(priv,
                        priv->can.ctrlmode & CAN_CTRLMODE_BERR_REPORTING);
        if (err)
                return err;

        err = ixx_pas_ib_fd_xxx_start_ctrl(priv, &time_ref);
        if (err)
                return err;

        ixxat_pci_set_ts_now(priv, time_ref);
        priv->can.state = CAN_STATE_ERROR_ACTIVE;

        err = ixx_pas_ib_fd_xxx_enable_int(priv, 1, 1);

        return err;
}

/*
 * called when probing to initialize a device object.
 */
static int ixx_pas_ib_fd_xxx_init(struct ixx_pci_priv *priv)
{
        struct ixx_pci_interface *intf = priv->intf;
        u32 ifi_sts_cmd;

        priv->ifi_base = intf->memvadd + CAN_IB_IFI_CAN_BASE
                        + priv->ctrl_idx * CAN_IB_IFI_CAN_SIZE;

        ixx_pas_ib_fd_xxx_set_filter(priv, 0);
        ixx_pas_ib_fd_xxx_enable_int(priv, 0, 1);

        ifi_sts_cmd = le32_to_cpu(ioread32(priv->ifi_base + IFIFDREG_STSCMD));
        ifi_sts_cmd |= IFIFD_R0_7_9_8_8_BT;
        iowrite32(cpu_to_le32(ifi_sts_cmd), priv->ifi_base + IFIFDREG_STSCMD);

        return 0;
}

/*
 * called when IXXAT CAN-IB5x0 adapter is unplugged
 */
static void ixx_pas_ib_fd_xxx_free(struct ixx_pci_priv *priv)
{
        if (!priv)
                return;

        if (priv->state == IXX_STATE_RUNNING)
                ixx_pas_ib_fd_xxx_stop(priv);

        /* last device: can free ixx_pas_ib_fd_xxx_interface object now */
        if (!priv->prev_siblings && !priv->next_siblings) {
                ixx_pas_ib_fd_xxx_int_ena_req(priv->intf, 0);

                if (priv->intf->device_irq) {
                        free_irq(priv->intf->device_irq, priv->intf);
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

static void ixx_pas_ib_fd_xxx_exit(struct ixx_pci_priv *priv)
{
        ixx_pas_ib_fd_xxx_stop_ctrl(priv);
        ixx_pas_ib_fd_xxx_free(priv);
}

/*
 * probe function for new IXXAT CAN-IB5x0 interface
 */
static int ixx_pas_ib_fd_xxx_probe(struct pci_dev *pdev,
                struct ixx_pci_interface * intf)
{
        int err;

        pci_enable_device(pdev);

        intf->reg1add = pci_resource_start(pdev, 0);
        intf->reg1len = pci_resource_len(pdev, 0);

        intf->memadd = pci_resource_start(pdev, 2);
        intf->memlen = pci_resource_len(pdev, 2);

        request_mem_region(intf->memadd, intf->memlen, "IXXAT PCI Memory");
        intf->memvadd = ioremap_nocache(intf->memadd, intf->memlen);
        if (!intf->memvadd) {
                printk("memvadd ioremap_nocache failed\n");
                err = -ENOBUFS;
                goto release_memreg;
        }

        request_mem_region(intf->reg1add, intf->reg1len, "IXXAT PCI Registers");
        intf->reg1vadd = ioremap_nocache(intf->reg1add, intf->reg1len);
        if (!intf->reg1vadd) {
                printk("reg1vadd ioremap_nocache failed\n");
                err = -ENOBUFS;
                goto release_reg1;
        }

        intf->int_counter = 0;
        err = request_irq(pdev->irq, ixx_pas_ib_fd_xxx_irq_handler, IRQF_SHARED,
                        dev_name(&pdev->dev), intf);
        if (err) {
                printk("request_irq failed\n");
                goto release_irq;
        }

        intf->device_irq = pdev->irq;

        return 0;

        release_irq: iounmap(intf->reg1vadd);
        release_mem_region(intf->reg1add, intf->reg1len);
        release_reg1: iounmap(intf->memvadd);
        release_mem_region(intf->memadd, intf->memlen);
        release_memreg:

        return err;
}

/*
 * describes the CAN-IB500/PCIe FD adapter
 */
struct ixx_pci_adapter can_ib_500 = {
                .name = "CAN-IB500/PCIe FD",
                .device_id = CAN_IB500_DEVICE_ID,
                .clock = { .freq = IFIFD_CANFD_CLOCK_HZ, },
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

                /* size of device private data */
                .sizeof_dev_private = sizeof(struct ixx_pci_priv),

                .ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
                                      CAN_CTRLMODE_ONE_SHOT |
                                      CAN_CTRLMODE_LISTENONLY |
                                      CAN_CTRLMODE_LOOPBACK |
                                      CAN_CTRLMODE_BERR_REPORTING |
                                      CAN_CTRLMODE_FD |
                                      CAN_CTRLMODE_FD_NON_ISO,

                /* device callbacks */
                .intf_probe          = ixx_pas_ib_fd_xxx_probe,
                .intf_int_ena        = ixx_pas_ib_fd_xxx_int_ena_req,
                .intf_get_caps       = ixx_pas_ib_fd_xxx_get_intf_caps,
                .dev_init            = ixx_pas_ib_fd_xxx_init,
                .dev_exit            = ixx_pas_ib_fd_xxx_exit,
                .dev_free            = ixx_pas_ib_fd_xxx_free,
                .dev_set_bittiming   = ixx_pas_ib_fd_xxx_set_bittiming,
                .dev_set_datatiming  = ixx_pas_ib_fd_xxx_set_datatiming,
                .intf_get_info       = ixx_pas_ib_fd_xxx_get_dev_info,
                .dev_start           = ixx_pas_ib_fd_xxx_start,
                .dev_stop            = ixx_pas_ib_fd_xxx_stop,
                .dev_restart         = ixx_pas_ib_fd_xxx_restart,
                .dev_start_xmit      = ixx_pas_ib_fd_xxx_start_xmit,
                .dev_napi_rx_poll    = ixx_pas_ib_fd_xxx_napi_rx_poll,
};

/*
 * describes the CAN-IB510/XMC FD adapter
 */
struct ixx_pci_adapter can_ib_510 = {
                .name = "CAN-IB510/XMC FD",
                .device_id = CAN_IB510_DEVICE_ID,
                .clock = { .freq = IFIFD_CANFD_CLOCK_HZ, },
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

                /* size of device private data */
                .sizeof_dev_private = sizeof(struct ixx_pci_priv),

                .ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
                                      CAN_CTRLMODE_ONE_SHOT |
                                      CAN_CTRLMODE_LISTENONLY |
                                      CAN_CTRLMODE_LOOPBACK |
                                      CAN_CTRLMODE_BERR_REPORTING |
                                      CAN_CTRLMODE_FD |
                                      CAN_CTRLMODE_FD_NON_ISO,

                /* device callbacks */
                .intf_probe          = ixx_pas_ib_fd_xxx_probe,
                .intf_int_ena        = ixx_pas_ib_fd_xxx_int_ena_req,
                .intf_get_caps       = ixx_pas_ib_fd_xxx_get_intf_caps,
                .dev_init            = ixx_pas_ib_fd_xxx_init,
                .dev_exit            = ixx_pas_ib_fd_xxx_exit,
                .dev_free            = ixx_pas_ib_fd_xxx_free,
                .dev_set_bittiming   = ixx_pas_ib_fd_xxx_set_bittiming,
                .dev_set_datatiming  = ixx_pas_ib_fd_xxx_set_datatiming,
                .intf_get_info       = ixx_pas_ib_fd_xxx_get_dev_info,
                .dev_start           = ixx_pas_ib_fd_xxx_start,
                .dev_stop            = ixx_pas_ib_fd_xxx_stop,
                .dev_restart         = ixx_pas_ib_fd_xxx_restart,
                .dev_start_xmit      = ixx_pas_ib_fd_xxx_start_xmit,
                .dev_napi_rx_poll    = ixx_pas_ib_fd_xxx_napi_rx_poll,
};

/*
 * describes the CAN-IB520/PCIe Mini FD adapter
 */
struct ixx_pci_adapter can_ib_520 = {
                .name = "CAN-IB520/PCIe Mini FD",
                .device_id = CAN_IB520_DEVICE_ID,
                .clock = { .freq = IFIFD_CANFD_CLOCK_HZ, },
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

                /* size of device private data */
                .sizeof_dev_private = sizeof(struct ixx_pci_priv),

                .ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
                                      CAN_CTRLMODE_ONE_SHOT |
                                      CAN_CTRLMODE_LISTENONLY |
                                      CAN_CTRLMODE_LOOPBACK |
                                      CAN_CTRLMODE_BERR_REPORTING |
                                      CAN_CTRLMODE_FD |
                                      CAN_CTRLMODE_FD_NON_ISO,

                /* device callbacks */
                .intf_probe          = ixx_pas_ib_fd_xxx_probe,
                .intf_int_ena        = ixx_pas_ib_fd_xxx_int_ena_req,
                .intf_get_caps       = ixx_pas_ib_fd_xxx_get_intf_caps,
                .dev_init            = ixx_pas_ib_fd_xxx_init,
                .dev_exit            = ixx_pas_ib_fd_xxx_exit,
                .dev_free            = ixx_pas_ib_fd_xxx_free,
                .dev_set_bittiming   = ixx_pas_ib_fd_xxx_set_bittiming,
                .dev_set_datatiming  = ixx_pas_ib_fd_xxx_set_datatiming,
                .intf_get_info       = ixx_pas_ib_fd_xxx_get_dev_info,
                .dev_start           = ixx_pas_ib_fd_xxx_start,
                .dev_stop            = ixx_pas_ib_fd_xxx_stop,
                .dev_restart         = ixx_pas_ib_fd_xxx_restart,
                .dev_start_xmit      = ixx_pas_ib_fd_xxx_start_xmit,
                .dev_napi_rx_poll    = ixx_pas_ib_fd_xxx_napi_rx_poll,
};


/*
 * describes the CAN-IB700/PCI FD adapter
 */
struct ixx_pci_adapter can_ib_700 = {
                .name = "CAN-IB700/PCI FD",
                .device_id = CAN_IB700_DEVICE_ID,
                .clock = { .freq = IFIFD_CANFD_CLOCK_HZ, },
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

                /* size of device private data */
                .sizeof_dev_private = sizeof(struct ixx_pci_priv),

                .ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
                                      CAN_CTRLMODE_ONE_SHOT |
                                      CAN_CTRLMODE_LISTENONLY |
                                      CAN_CTRLMODE_LOOPBACK |
                                      CAN_CTRLMODE_BERR_REPORTING |
                                      CAN_CTRLMODE_FD |
                                      CAN_CTRLMODE_FD_NON_ISO,

                /* device callbacks */
                .intf_probe          = ixx_pas_ib_fd_xxx_probe,
                .intf_int_ena        = ixx_pas_ib_fd_xxx_int_ena_req,
                .intf_get_caps       = ixx_pas_ib_fd_xxx_get_intf_caps,
                .dev_init            = ixx_pas_ib_fd_xxx_init,
                .dev_exit            = ixx_pas_ib_fd_xxx_exit,
                .dev_free            = ixx_pas_ib_fd_xxx_free,
                .dev_set_bittiming   = ixx_pas_ib_fd_xxx_set_bittiming,
                .dev_set_datatiming  = ixx_pas_ib_fd_xxx_set_datatiming,
                .intf_get_info       = ixx_pas_ib_fd_xxx_get_dev_info,
                .dev_start           = ixx_pas_ib_fd_xxx_start,
                .dev_stop            = ixx_pas_ib_fd_xxx_stop,
                .dev_restart         = ixx_pas_ib_fd_xxx_restart,
                .dev_start_xmit      = ixx_pas_ib_fd_xxx_start_xmit,
                .dev_napi_rx_poll    = ixx_pas_ib_fd_xxx_napi_rx_poll,
};

/*
 * describes the CAN-IB710/PMC FD adapter
 */
struct ixx_pci_adapter can_ib_710 = {
                .name = "CAN-IB710/PMC FD",
                .device_id = CAN_IB710_DEVICE_ID,
                .clock = { .freq = IFIFD_CANFD_CLOCK_HZ, },
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

                /* size of device private data */
                .sizeof_dev_private = sizeof(struct ixx_pci_priv),

                .ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES |
                                      CAN_CTRLMODE_ONE_SHOT |
                                      CAN_CTRLMODE_LISTENONLY |
                                      CAN_CTRLMODE_LOOPBACK |
                                      CAN_CTRLMODE_BERR_REPORTING |
                                      CAN_CTRLMODE_FD |
                                      CAN_CTRLMODE_FD_NON_ISO,

                /* device callbacks */
                .intf_probe          = ixx_pas_ib_fd_xxx_probe,
                .intf_int_ena        = ixx_pas_ib_fd_xxx_int_ena_req,
                .intf_get_caps       = ixx_pas_ib_fd_xxx_get_intf_caps,
                .dev_init            = ixx_pas_ib_fd_xxx_init,
                .dev_exit            = ixx_pas_ib_fd_xxx_exit,
                .dev_free            = ixx_pas_ib_fd_xxx_free,
                .dev_set_bittiming   = ixx_pas_ib_fd_xxx_set_bittiming,
                .dev_set_datatiming  = ixx_pas_ib_fd_xxx_set_datatiming,
                .intf_get_info       = ixx_pas_ib_fd_xxx_get_dev_info,
                .dev_start           = ixx_pas_ib_fd_xxx_start,
                .dev_stop            = ixx_pas_ib_fd_xxx_stop,
                .dev_restart         = ixx_pas_ib_fd_xxx_restart,
                .dev_start_xmit      = ixx_pas_ib_fd_xxx_start_xmit,
                .dev_napi_rx_poll    = ixx_pas_ib_fd_xxx_napi_rx_poll,
};

#endif //CANFD_CAPABLE
