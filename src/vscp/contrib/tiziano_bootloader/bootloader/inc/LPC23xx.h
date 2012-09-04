// ***************************************************************
//
// DESCRIPTION: Header file for NXP LPC230x Family Microprocessors 
// 		   		The header file is the super set of all hardware definition 
//				of the peripherals for the LPC230x family microprocessor.
// AUTHOR:
//
// HISTORY:    
//
// *******************************************************************

#ifndef __LPC23xx_H
#define __LPC23xx_H

///////////////////////////////////////////////////////////////////////////////
#define PART_ID_LPC2364                      0x1600f902
#define PART_ID_LPC2366                      0x1600f923
#define PART_ID_LPC2368                      0x1600f925
#define PART_ID_LPC2378                      0x1600fd25

#define FLASH_START_ADDRESS              0                           // up to 512k
#define START_OF_FLASH                   FLASH_START_ADDRESS
#define RAM_START_ADDRESS                0x40000000                  // up to 32k
#define ETHERNET_RAM_START_ADDRESS       0x7fe00000                  // 16k
#define ETHERNET_RAM_SIZE                (16 * 1024)
#define USB_RAM_START                    0x7fd00000                  // 8k
#define USB_RAM_SIZE                     (8 * 1024)

#define FLASH_GRANULARITY_SMALL          (4*1024)                    // small blocks
#define FLASH_GRANULARITY_LARGE          (32*1024)                   // large blocks

#define FLASH_SECTOR_0_OFFSET            0
#define FLASH_SECTOR_SIZE_0              FLASH_GRANULARITY_SMALL     // initially small blocks
#define FLASH_SECTOR_1_OFFSET            0x1000
#define FLASH_SECTOR_SIZE_1              FLASH_GRANULARITY_SMALL
#define FLASH_SECTOR_2_OFFSET            0x2000
#define FLASH_SECTOR_SIZE_2              FLASH_GRANULARITY_SMALL
#define FLASH_SECTOR_3_OFFSET            0x3000
#define FLASH_SECTOR_SIZE_3              FLASH_GRANULARITY_SMALL
#define FLASH_SECTOR_4_OFFSET            0x4000
#define FLASH_SECTOR_SIZE_4              FLASH_GRANULARITY_SMALL
#define FLASH_SECTOR_5_OFFSET            0x5000
#define FLASH_SECTOR_SIZE_5              FLASH_GRANULARITY_SMALL
#define FLASH_SECTOR_6_OFFSET            0x6000
#define FLASH_SECTOR_SIZE_6              FLASH_GRANULARITY_SMALL
#define FLASH_SECTOR_7_OFFSET            0x7000
#define FLASH_SECTOR_SIZE_7              FLASH_GRANULARITY_SMALL
#define FLASH_SECTOR_8_OFFSET            0x8000                      // end large blocks

#define FLASH_SECTOR_SIZE_8              FLASH_GRANULARITY_LARGE     // last sector in 64k part
#define FLASH_SECTOR_9_OFFSET            0x10000
#define FLASH_SECTOR_SIZE_9              FLASH_GRANULARITY_LARGE
#define FLASH_SECTOR_10_OFFSET           0x18000
#define FLASH_SECTOR_SIZE_10             FLASH_GRANULARITY_LARGE     // last sector in 128k part
#define FLASH_SECTOR_11_OFFSET           0x20000
#define FLASH_SECTOR_SIZE_11             FLASH_GRANULARITY_LARGE
#define FLASH_SECTOR_12_OFFSET           0x28000
#define FLASH_SECTOR_SIZE_12             FLASH_GRANULARITY_LARGE
#define FLASH_SECTOR_13_OFFSET           0x30000
#define FLASH_SECTOR_SIZE_13             FLASH_GRANULARITY_LARGE
#define FLASH_SECTOR_14_OFFSET           0x38000
#define FLASH_SECTOR_SIZE_14             FLASH_GRANULARITY_LARGE     // last sector in 256k part
#define FLASH_SECTOR_15_OFFSET           0x40000
#define FLASH_SECTOR_SIZE_15             FLASH_GRANULARITY_LARGE
#define FLASH_SECTOR_16_OFFSET           0x48000
#define FLASH_SECTOR_SIZE_16             FLASH_GRANULARITY_LARGE
#define FLASH_SECTOR_17_OFFSET           0x50000
#define FLASH_SECTOR_SIZE_17             FLASH_GRANULARITY_LARGE
#define FLASH_SECTOR_18_OFFSET           0x58000
#define FLASH_SECTOR_SIZE_18             FLASH_GRANULARITY_LARGE
#define FLASH_SECTOR_19_OFFSET           0x60000
#define FLASH_SECTOR_SIZE_19             FLASH_GRANULARITY_LARGE
#define FLASH_SECTOR_20_OFFSET           0x68000
#define FLASH_SECTOR_SIZE_20             FLASH_GRANULARITY_LARGE
#define FLASH_SECTOR_21_OFFSET           0x70000
#define FLASH_SECTOR_SIZE_21             FLASH_GRANULARITY_LARGE

#define FLASH_SECTOR_22_OFFSET           0x78000                     // further small blocks
#define FLASH_SECTOR_SIZE_22             FLASH_GRANULARITY_SMALL
#define FLASH_SECTOR_23_OFFSET           0x79000
#define FLASH_SECTOR_SIZE_23             FLASH_GRANULARITY_SMALL
#define FLASH_SECTOR_24_OFFSET           0x7a000
#define FLASH_SECTOR_SIZE_24             FLASH_GRANULARITY_SMALL
#define FLASH_SECTOR_25_OFFSET           0x7b000
#define FLASH_SECTOR_SIZE_25             FLASH_GRANULARITY_SMALL
#define FLASH_SECTOR_26_OFFSET           0x7c000
#define FLASH_SECTOR_SIZE_26             FLASH_GRANULARITY_SMALL
#define FLASH_SECTOR_27_OFFSET           0x7d000
#define FLASH_SECTOR_SIZE_27             FLASH_GRANULARITY_SMALL     // last sector in 512k part

#define FLASH_LINE_SIZE                  16                              // lines made up of quad words (128 bits)
#define FLASH_ROW_SIZE                   256                             // FLASH writes are performed in rows of this size

///////////////////////////////////////////////////////////////////////////////

// Vectored Interrupt Controller (VIC) 
#define VIC_BASE_ADDR	0xFFFFF000
#define VICIRQStatus   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x000))
#define VICFIQStatus   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x004))
#define VICRawIntr     (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x008))
    #define INTERRUPT_WDT                0x00000001
    #define INTERRUPT_ARM_CORE_RX        0x00000004
    #define INTERRUPT_ARM_CORE_TX        0x00000008
    #define INTERRUPT_TIMER0             0x00000010
    #define INTERRUPT_TIMER1             0x00000020
    #define INTERRUPT_UART0              0x00000040
    #define INTERRUPT_UART1              0x00000080
    #define INTERRUPT_PWM1               0x00000100
    #define INTERRUPT_I2C0               0x00000200
    #define INTERRUPT_SSP0               0x00000400
    #define INTERRUPT_SSP1               0x00000800
    #define INTERRUPT_PLL                0x00001000
    #define INTERRUPT_RTC                0x00002000
    #define INTERRUPT_EINT0              0x00004000
    #define INTERRUPT_EINT1              0x00008000
    #define INTERRUPT_EINT2              0x00010000
    #define INTERRUPT_EINT3              0x00020000
    #define INTERRUPT_ADC0               0x00040000
    #define INTERRUPT_I2C1               0x00080000
    #define INTERRUPT_BOD                0x00100000
    #define INTERRUPT_ETHERNET           0x00200000
    #define INTERRUPT_USB                0x00400000
    #define INTERRUPT_CAN                0x00800000
    #define INTERRUPT_SD_MMC             0x01000000
    #define INTERRUPT_GP_DMA             0x02000000
    #define INTERRUPT_TIMER2             0x04000000
    #define INTERRUPT_TIMER3             0x08000000
    #define INTERRUPT_UART2              0x10000000
    #define INTERRUPT_UART3              0x20000000
    #define INTERRUPT_I2C2               0x40000000
    #define INTERRUPT_I2S                0x80000000
#define VICIntSelect   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x00C))
#define VICIntEnable   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x010))
#define VICIntEnClr    (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x014))
#define VICSoftInt     (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x018))
#define VICSoftIntClr  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x01C))
#define VICProtection  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x020))
#define VICSWPrioMask  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x024))

#define VICVectAddr0   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x100))
#define VICVectAddr1   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x104))
#define VICVectAddr2   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x108))
#define VICVectAddr3   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x10C))
#define VICVectAddr4   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x110))
#define VICVectAddr5   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x114))
#define VICVectAddr6   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x118))
#define VICVectAddr7   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x11C))
#define VICVectAddr8   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x120))
#define VICVectAddr9   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x124))
#define VICVectAddr10  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x128))
#define VICVectAddr11  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x12C))
#define VICVectAddr12  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x130))
#define VICVectAddr13  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x134))
#define VICVectAddr14  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x138))
#define VICVectAddr15  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x13C))
#define VICVectAddr16  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x140))
#define VICVectAddr17  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x144))
#define VICVectAddr18  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x148))
#define VICVectAddr19  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x14C))
#define VICVectAddr20  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x150))
#define VICVectAddr21  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x154))
#define VICVectAddr22  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x158))
#define VICVectAddr23  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x15C))
#define VICVectAddr24  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x160))
#define VICVectAddr25  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x164))
#define VICVectAddr26  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x168))
#define VICVectAddr27  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x16C))
#define VICVectAddr28  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x170))
#define VICVectAddr29  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x174))
#define VICVectAddr30  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x178))
#define VICVectAddr31  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x17C))

// The name convention below is from previous LPC2000 family MCUs, in LPC230x,
// these registers are known as "VICVectPriority(x)". 
#define VICVectCntl0   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x200))
#define VICVectCntl1   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x204))
#define VICVectCntl2   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x208))
#define VICVectCntl3   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x20C))
#define VICVectCntl4   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x210))
#define VICVectCntl5   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x214))
#define VICVectCntl6   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x218))
#define VICVectCntl7   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x21C))
#define VICVectCntl8   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x220))
#define VICVectCntl9   (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x224))
#define VICVectCntl10  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x228))
#define VICVectCntl11  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x22C))
#define VICVectCntl12  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x230))
#define VICVectCntl13  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x234))
#define VICVectCntl14  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x238))
#define VICVectCntl15  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x23C))
#define VICVectCntl16  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x240))
#define VICVectCntl17  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x244))
#define VICVectCntl18  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x248))
#define VICVectCntl19  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x24C))
#define VICVectCntl20  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x250))
#define VICVectCntl21  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x254))
#define VICVectCntl22  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x258))
#define VICVectCntl23  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x25C))
#define VICVectCntl24  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x260))
#define VICVectCntl25  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x264))
#define VICVectCntl26  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x268))
#define VICVectCntl27  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x26C))
#define VICVectCntl28  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x270))
#define VICVectCntl29  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x274))
#define VICVectCntl30  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x278))
#define VICVectCntl31  (*(volatile unsigned int *)(VIC_BASE_ADDR + 0x27C))

#define VICVectAddr    (*(volatile unsigned int *)(VIC_BASE_ADDR + 0xF00))

// Pin Connect Block 
#define PINSEL_BASE_ADDR	0xE002C000
#define PINSEL0        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x00))
  #define PINSEL0_P0_0_GPIO              0x00000000
  #define PINSEL0_P0_0_RD1               0x00000001
  #define PINSEL0_P0_0_TXD3              0x00000002
  #define PINSEL0_P0_0_SDA1              0x00000003
  #define PINSEL0_P0_0_RESET             0x00000003
  #define PINSEL0_P0_1_GPIO              0x00000000
  #define PINSEL0_P0_1_TD1               0x00000004
  #define PINSEL0_P0_1_RXD3              0x00000008
  #define PINSEL0_P0_1_SCL1              0x0000000c
  #define PINSEL0_P0_1_RESET             0x0000000c
  #define PINSEL0_P0_2_GPIO              0x00000000
  #define PINSEL0_P0_2_TXD0              0x00000010
  #define PINSEL0_P0_2_RESET             0x00000030
  #define PINSEL0_P0_3_GPIO              0x00000000
  #define PINSEL0_P0_3_RXD0              0x00000040
  #define PINSEL0_P0_3_RESET             0x000000c0
  #define PINSEL0_P0_4_GPIO              0x00000000
  #define PINSEL0_P0_4_I2SRX_CLK         0x00000100
  #define PINSEL0_P0_4_RD2               0x00000200
  #define PINSEL0_P0_4_CAP2_0            0x00000300
  #define PINSEL0_P0_4_RESET             0x00000300
  #define PINSEL0_P0_5_GPIO              0x00000000
  #define PINSEL0_P0_5_I2SRX_WS          0x00000400
  #define PINSEL0_P0_5_TD2               0x00000800
  #define PINSEL0_P0_5_CAP2_1            0x00000c00
  #define PINSEL0_P0_5_RESET             0x00000c00
  #define PINSEL0_P0_6_GPIO              0x00000000
  #define PINSEL0_P0_6_I2SRX_SDA         0x00001000
  #define PINSEL0_P0_6_SSEL1             0x00002000
  #define PINSEL0_P0_6_MAT2_1            0x00003000
  #define PINSEL0_P0_6_RESET             0x00003000
  #define PINSEL0_P0_7_GPIO              0x00000000
  #define PINSEL0_P0_7_I2STX_CLK         0x00004000
  #define PINSEL0_P0_7_SCK1              0x00008000
  #define PINSEL0_P0_7_MAT2_1            0x0000c000
  #define PINSEL0_P0_7_RESET             0x0000c000
  #define PINSEL0_P0_8_GPIO              0x00000000
  #define PINSEL0_P0_8_I2STX_WS          0x00010000
  #define PINSEL0_P0_8_MISO1             0x00020000
  #define PINSEL0_P0_8_MAT2_2            0x00030000
  #define PINSEL0_P0_8_RESET             0x00030000
  #define PINSEL0_P0_9_GPIO              0x00000000
  #define PINSEL0_P0_9_I2STX_SDA         0x00040000
  #define PINSEL0_P0_9_MOSI1             0x00080000
  #define PINSEL0_P0_9_MAT2_3            0x000c0000
  #define PINSEL0_P0_9_RESET             0x000c0000
  #define PINSEL0_P0_10_GPIO             0x00000000
  #define PINSEL0_P0_10_TXD2             0x00100000
  #define PINSEL0_P0_10_SDA2             0x00200000
  #define PINSEL0_P0_10_MAT3_0           0x00300000
  #define PINSEL0_P0_10_RESET            0x00300000
  #define PINSEL0_P0_11_GPIO             0x00000000
  #define PINSEL0_P0_11_RXD2             0x00400000
  #define PINSEL0_P0_11_SCL2             0x00800000
  #define PINSEL0_P0_11_MAT3_1           0x00c00000
  #define PINSEL0_P0_11_RESET            0x00c00000
  #define PINSEL0_P0_12_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL0_P0_12_MISO1            0x02000000                      // Only LPC2378
  #define PINSEL0_P0_12_AD0_6            0x03000000                      // Only LPC2378
  #define PINSEL0_P0_12_RESET            0x03000000                      // Only LPC2378
  #define PINSEL0_P0_13_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL0_P0_13_U2UP_LED         0x04000000                      // Only LPC2378
  #define PINSEL0_P0_13_MOSI1            0x08000000                      // Only LPC2378
  #define PINSEL0_P0_13_AD0_7            0x0c000000                      // Only LPC2378
  #define PINSEL0_P0_13_RESET            0x0c000000                      // Only LPC2378
  #define PINSEL0_P0_14_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL0_P0_14_U2CONNECT        0x10000000                      // Only LPC2378
  #define PINSEL0_P0_14_SSEL2            0x30000000                      // Only LPC2378
  #define PINSEL0_P0_14_RESET            0x30000000                      // Only LPC2378
  #define PINSEL0_P0_15_GPIO             0x00000000
  #define PINSEL0_P0_15_TXD1             0x40000000
  #define PINSEL0_P0_15_SCK0             0x80000000
  #define PINSEL0_P0_15_SCK              0xc0000000
  #define PINSEL0_P0_15_RESET            0xc0000000
#define PINSEL1        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x04))
  #define PINSEL1_P0_16_GPIO             0x00000000
  #define PINSEL1_P0_16_RXD1             0x00000001
  #define PINSEL1_P0_16_SSEL0            0x00000002
  #define PINSEL1_P0_16_SSEL             0x00000003
  #define PINSEL1_P0_16_RESET            0x00000003
  #define PINSEL1_P0_17_GPIO             0x00000000
  #define PINSEL1_P0_17_CTS1             0x00000004
  #define PINSEL1_P0_17_MISO0            0x00000008
  #define PINSEL1_P0_17_MISO             0x0000000c
  #define PINSEL1_P0_17_RESET            0x0000000c
  #define PINSEL1_P0_18_GPIO             0x00000000
  #define PINSEL1_P0_18_DCD1             0x00000010
  #define PINSEL1_P0_18_MOSI0            0x00000020
  #define PINSEL1_P0_18_MOSI             0x00000030
  #define PINSEL1_P0_18_RESET            0x00000030
  #define PINSEL1_P0_19_GPIO             0x00000000
  #define PINSEL1_P0_19_DSR1             0x00000040
  #define PINSEL1_P0_19_MCICLK           0x00000080
  #define PINSEL1_P0_19_SDA1             0x000000c0
  #define PINSEL1_P0_19_RESET            0x000000c0
  #define PINSEL1_P0_20_GPIO             0x00000000
  #define PINSEL1_P0_20_DTR1             0x00000100
  #define PINSEL1_P0_20_MCIMD            0x00000200
  #define PINSEL1_P0_20_SCL1             0x00000300
  #define PINSEL1_P0_20_RESET            0x00000300
  #define PINSEL1_P0_21_GPIO             0x00000000
  #define PINSEL1_P0_21_RI1              0x00000400
  #define PINSEL1_P0_21_MCIPWR           0x00000800
  #define PINSEL1_P0_21_RD1              0x00000c00
  #define PINSEL1_P0_21_RESET            0x00000c00
  #define PINSEL1_P0_22_GPIO             0x00000000
  #define PINSEL1_P0_22_RTS1             0x00001000
  #define PINSEL1_P0_22_MCIDATA0         0x00002000
  #define PINSEL1_P0_22_TD1              0x00003000
  #define PINSEL1_P0_22_RESET            0x00003000
  #define PINSEL1_P0_23_GPIO             0x00000000
  #define PINSEL1_P0_23_AD0_0            0x00004000
  #define PINSEL1_P0_23_I2SRX_CLK        0x00008000
  #define PINSEL1_P0_23_CAP3_0           0x0000c000
  #define PINSEL1_P0_23_RESET            0x0000c000
  #define PINSEL1_P0_24_GPIO             0x00000000
  #define PINSEL1_P0_24_AD0_1            0x00010000
  #define PINSEL1_P0_24_I2SRX_WS         0x00020000
  #define PINSEL1_P0_24_CAP3_1           0x00030000
  #define PINSEL1_P0_24_RESET            0x00030000
  #define PINSEL1_P0_25_GPIO             0x00000000
  #define PINSEL1_P0_25_AD0_2            0x00040000
  #define PINSEL1_P0_25_I2SRX_SDA        0x00080000
  #define PINSEL1_P0_25_TXD3             0x000c0000
  #define PINSEL1_P0_25_RESET            0x000c0000
  #define PINSEL1_P0_26_GPIO             0x00000000
  #define PINSEL1_P0_26_AD0_3            0x00100000
  #define PINSEL1_P0_26_AOUT             0x00200000
  #define PINSEL1_P0_26_RXD3             0x00300000
  #define PINSEL1_P0_26_RESET            0x00300000
  #define PINSEL1_P0_27_GPIO             0x00000000
  #define PINSEL1_P0_27_SDA0             0x00400000
  #define PINSEL1_P0_27_RESET            0x00c00000
  #define PINSEL1_P0_28_GPIO             0x00000000
  #define PINSEL1_P0_28_SCL0             0x01000000
  #define PINSEL1_P0_28_RESET            0x03000000
  #define PINSEL1_P0_29_GPIO             0x00000000
  #define PINSEL1_P0_29_U1D_PLUS         0x04000000
  #define PINSEL1_P0_29_RESET            0x0c000000
  #define PINSEL1_P0_30_GPIO             0x00000000
  #define PINSEL1_P0_30_U1D_MINUS        0x10000000
  #define PINSEL1_P0_30_RESET            0x30000000
  #define PINSEL1_P0_31_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL1_P0_31_U2D_PLUS         0x40000000                      // Only LPC2378
  #define PINSEL1_P0_31_RESET            0xc0000000                      // Only LPC2378
#define PINSEL2        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x08))
  #define PINSEL2_P1_0_GPIO              0x00000000
  #define PINSEL2_P1_0_ENET_TXD0         0x00000001
  #define PINSEL2_P1_0_RESET             0x00000003
  #define PINSEL2_P1_1_GPIO              0x00000000
  #define PINSEL2_P1_1_ENET_TXD0         0x00000004
  #define PINSEL2_P1_1_RESET             0x0000000c
  #define PINSEL2_P1_4_GPIO              0x00000000
  #define PINSEL2_P1_4_ENET_TX_EN        0x00000100
  #define PINSEL2_P1_4_RESET             0x00000300
  #define PINSEL2_P1_5_GPIO              0x00000000
  #define PINSEL2_P1_6_GPIO              0x00000000
  #define PINSEL2_P1_6_ENET_TX_CLK       0x00001000
  #define PINSEL2_P1_8_GPIO              0x00000000
  #define PINSEL2_P1_8_ENET_CRS          0x00010000
  #define PINSEL2_P1_8_RESET             0x00030000
  #define PINSEL2_P1_9_GPIO              0x00000000
  #define PINSEL2_P1_9_ENET_RXD0         0x00040000
  #define PINSEL2_P1_9_RESET             0x000c0000
  #define PINSEL2_P1_10_GPIO             0x00000000
  #define PINSEL2_P1_10_ENET_RXD1        0x00100000
  #define PINSEL2_P1_10_RESET            0x00300000
  #define PINSEL2_P1_14_GPIO             0x00000000
  #define PINSEL2_P1_14_ENET_RX_ER       0x10000000
  #define PINSEL2_P1_14_RESET            0x30000000
  #define PINSEL2_P1_15_GPIO             0x00000000
  #define PINSEL2_P1_15_ENET_REF_CLK     0x40000000
  #define PINSEL2_P1_15_RESET            0xc0000000
#define PINSEL3        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x0C))
  #define PINSEL3_P1_16_GPIO             0x00000000
  #define PINSEL3_P1_16_ENET_MDC         0x00000001
  #define PINSEL3_P1_16_RESET            0x00000003
  #define PINSEL3_P1_17_GPIO             0x00000000
  #define PINSEL3_P1_17_ENET_MDIO        0x00000004
  #define PINSEL3_P1_17_RESET            0x0000000c
  #define PINSEL3_P1_18_GPIO             0x00000000
  #define PINSEL3_P1_18_U1UP_LED         0x00000010
  #define PINSEL3_P1_18_PWM1_1           0x00000020
  #define PINSEL3_P1_18_CAP1_0           0x00000030
  #define PINSEL3_P1_18_RESET            0x00000030
  #define PINSEL3_P1_19_GPIO             0x00000000
  #define PINSEL3_P1_19_CAP1_1           0x000000c0
  #define PINSEL3_P1_19_RESET            0x000000c0
  #define PINSEL3_P1_20_GPIO             0x00000000
  #define PINSEL3_P1_20_PWM1_2           0x00000200
  #define PINSEL3_P1_20_SCK0             0x00000300
  #define PINSEL3_P1_20_RESET            0x00000300
  #define PINSEL3_P1_21_GPIO             0x00000000
  #define PINSEL3_P1_21_PWM1_3           0x00000800
  #define PINSEL3_P1_21_SSEL0            0x00000c00
  #define PINSEL3_P1_21_RESET            0x00000c00
  #define PINSEL3_P1_22_GPIO             0x00000000
  #define PINSEL3_P1_22_MAT1_0           0x00003000
  #define PINSEL3_P1_22_RESET            0x00003000
  #define PINSEL3_P1_23_GPIO             0x00000000
  #define PINSEL3_P1_23_PWM1_4           0x00008000
  #define PINSEL3_P1_23_MISO0            0x0000c000
  #define PINSEL3_P1_23_RESET            0x0000c000
  #define PINSEL3_P1_24_GPIO             0x00000000
  #define PINSEL3_P1_24_PWM1_5           0x00020000
  #define PINSEL3_P1_24_MOSI0            0x00030000
  #define PINSEL3_P1_24_RESET            0x00030000
  #define PINSEL3_P1_25_GPIO             0x00000000
  #define PINSEL3_P1_25_MAT1_1           0x000c0000
  #define PINSEL3_P1_25_RESET            0x000c0000
  #define PINSEL3_P1_26_GPIO             0x00000000
  #define PINSEL3_P1_26_PWM1_6           0x00200000
  #define PINSEL3_P1_26_CAP0_0           0x00300000
  #define PINSEL3_P1_26_RESET            0x00300000
  #define PINSEL3_P1_27_GPIO             0x00000000
  #define PINSEL3_P1_27_CAP0_1           0x00c00000
  #define PINSEL3_P1_27_RESET            0x00c00000
  #define PINSEL3_P1_28_GPIO             0x00000000
  #define PINSEL3_P1_28_PCAP1_0          0x02000000
  #define PINSEL3_P1_28_MAT0_0           0x03000000
  #define PINSEL3_P1_28_RESET            0x03000000
  #define PINSEL3_P1_29_GPIO             0x00000000
  #define PINSEL3_P1_29_PCAP1_1          0x04000000
  #define PINSEL3_P1_29_MAT0_1           0x0c000000
  #define PINSEL3_P1_29_RESET            0x0c000000
  #define PINSEL3_P1_30_GPIO             0x00000000
  #define PINSEL3_P1_30_VBUS             0x20000000
  #define PINSEL3_P1_30_AD0_4            0x30000000
  #define PINSEL3_P1_30_RESET            0x30000000
  #define PINSEL3_P1_31_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL3_P1_31_SCK1             0x40000000                      // Only LPC2378
  #define PINSEL3_P1_31_AD0_5            0xc0000000                      // Only LPC2378
  #define PINSEL3_P1_31_RESET            0xc0000000                      // Only LPC2378
#define PINSEL4        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x10))
  #define PINSEL4_P2_0_GPIO              0x00000000
  #define PINSEL4_P2_0_PWM1_1            0x00000001
  #define PINSEL4_P2_0_TXD1              0x00000002
  #define PINSEL4_P2_0_TRACECLK          0x00000003
  #define PINSEL4_P2_0_RESET             0x00000003
  #define PINSEL4_P2_1_GPIO              0x00000000
  #define PINSEL4_P2_1_PWM1_2            0x00000004
  #define PINSEL4_P2_1_RXD1              0x00000008
  #define PINSEL4_P2_1_PIPESTAT0         0x0000000c
  #define PINSEL4_P2_1_RESET             0x0000000c
  #define PINSEL4_P2_2_GPIO              0x00000000
  #define PINSEL4_P2_2_PWM1_3            0x00000010
  #define PINSEL4_P2_2_CTS1              0x00000020
  #define PINSEL4_P2_2_PIPESTAT1         0x00000030
  #define PINSEL4_P2_2_RESET             0x00000030
  #define PINSEL4_P2_3_GPIO              0x00000000
  #define PINSEL4_P2_3_PWM1_4            0x00000040
  #define PINSEL4_P2_3_DCD1              0x00000080
  #define PINSEL4_P2_3_PIPESTAT2         0x000000c0
  #define PINSEL4_P2_3_RESET             0x000000c0
  #define PINSEL4_P2_4_GPIO              0x00000000
  #define PINSEL4_P2_4_PWM1_5            0x00000100
  #define PINSEL4_P2_4_DSR1              0x00000200
  #define PINSEL4_P2_4_TRACESYN          0x00000300
  #define PINSEL4_P2_4_RESET             0x00000300
  #define PINSEL4_P2_5_GPIO              0x00000000
  #define PINSEL4_P2_5_PWM1_6            0x00000400
  #define PINSEL4_P2_5_DTR1              0x00000800
  #define PINSEL4_P2_5_TRACEPKT0         0x00000c00
  #define PINSEL4_P2_5_RESET             0x00000c00
  #define PINSEL4_P2_6_GPIO              0x00000000
  #define PINSEL4_P2_6_PCAP1_0           0x00001000
  #define PINSEL4_P2_6_RI1               0x00002000
  #define PINSEL4_P2_6_TRACEPKT1         0x00003000
  #define PINSEL4_P2_6_RESET             0x00003000
  #define PINSEL4_P2_7_GPIO              0x00000000
  #define PINSEL4_P2_7_RD2               0x00004000
  #define PINSEL4_P2_7_RTS1              0x00008000
  #define PINSEL4_P2_7_TRACEPKT2         0x0000c000
  #define PINSEL4_P2_7_RESET             0x0000c000
  #define PINSEL4_P2_8_GPIO              0x00000000
  #define PINSEL4_P2_8_TD2               0x00010000
  #define PINSEL4_P2_8_TXD2              0x00020000
  #define PINSEL4_P2_8_TRACEPKT3         0x00030000
  #define PINSEL4_P2_8_RESET             0x00030000
  #define PINSEL4_P2_9_GPIO              0x00000000
  #define PINSEL4_P2_9_U1CONNECT         0x00040000
  #define PINSEL4_P2_9_RXD2              0x00080000
  #define PINSEL4_P2_9_EXTIN0            0x000c0000
  #define PINSEL4_P2_9_RESET             0x000c0000
  #define PINSEL4_P2_10_GPIO             0x00000000
  #define PINSEL4_P2_10_EINT0            0x00100000
  #define PINSEL4_P2_10_RESET            0x00300000
  #define PINSEL4_P2_11_GPIO             0x00000000
  #define PINSEL4_P2_11_EINT1            0x00400000
  #define PINSEL4_P2_11_MCIDAT1          0x00800000
  #define PINSEL4_P2_11_I2STX_CLK        0x00c00000
  #define PINSEL4_P2_11_RESET            0x00c00000
  #define PINSEL4_P2_12_GPIO             0x00000000
  #define PINSEL4_P2_12_EINT2            0x01000000
  #define PINSEL4_P2_12_MCIDAT2          0x02000000
  #define PINSEL4_P2_12_I2STX_WS         0x03000000
  #define PINSEL4_P2_12_RESET            0x03000000
  #define PINSEL4_P2_13_GPIO             0x00000000
  #define PINSEL4_P2_13_EINT3            0x04000000
  #define PINSEL4_P2_13_MCIDAT3          0x08000000
  #define PINSEL4_P2_13_I2STX_SDA        0x0c000000
  #define PINSEL4_P2_13_RESET            0x0c000000
  #define PINSEL4_P2_14_GPIO             0x00000000
  #define PINSEL4_P2_14_RESET            0x30000000
  #define PINSEL4_P2_15_GPIO             0x00000000
  #define PINSEL4_P2_15_RESET            0xc0000000
#define PINSEL5        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x14))
#define PINSEL6        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x18))
  #define PINSEL6_P3_0_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL6_P3_0_D0                0x00000001                      // Only LPC2378
  #define PINSEL6_P3_0_RESET             0x00000003                      // Only LPC2378
  #define PINSEL6_P3_1_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL6_P3_1_D1                0x00000004                      // Only LPC2378
  #define PINSEL6_P3_1_RESET             0x0000000c                      // Only LPC2378
  #define PINSEL6_P3_2_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL6_P3_2_D2                0x00000010                      // Only LPC2378
  #define PINSEL6_P3_2_RESET             0x00000030                      // Only LPC2378
  #define PINSEL6_P3_3_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL6_P3_3_D3                0x00000040                      // Only LPC2378
  #define PINSEL6_P3_3_RESET             0x000000c0                      // Only LPC2378
  #define PINSEL6_P3_4_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL6_P3_4_D4                0x00000100                      // Only LPC2378
  #define PINSEL6_P3_4_RESET             0x00000300                      // Only LPC2378
  #define PINSEL6_P3_5_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL6_P3_5_D5                0x00000400                      // Only LPC2378
  #define PINSEL6_P3_5_RESET             0x00000c00                      // Only LPC2378
  #define PINSEL6_P3_6_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL6_P3_6_D6                0x00001000                      // Only LPC2378
  #define PINSEL6_P3_6_RESET             0x00003000                      // Only LPC2378
  #define PINSEL6_P3_7_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL6_P3_7_D7                0x00004000                      // Only LPC2378
  #define PINSEL6_P3_7_RESET             0x0000c000                      // Only LPC2378
#define PINSEL7        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x1C))
    #define PINSEL7_P3_23_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL7_P3_23_CAP0_0           0x00008000                      // Only LPC2378
  #define PINSEL7_P3_23_PCAP1_0          0x0000c000                      // Only LPC2378
  #define PINSEL7_P3_23_RESET            0x0000c000                      // Only LPC2378
  #define PINSEL7_P3_24_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL7_P3_24_CAP0_1           0x00020000                      // Only LPC2378
  #define PINSEL7_P3_24_PCAP1_1          0x00030000                      // Only LPC2378
  #define PINSEL7_P3_24_RESET            0x00030000                      // Only LPC2378
  #define PINSEL7_P3_25_GPIO             0x00000000
  #define PINSEL7_P3_25_MAT0_1           0x00080000
  #define PINSEL7_P3_25_PWM1_2           0x000c0000
  #define PINSEL7_P3_25_RESET            0x000c0000
  #define PINSEL7_P3_26_GPIO             0x00000000
  #define PINSEL7_P3_26_MAT0_1           0x00200000
  #define PINSEL7_P3_26_PWM1_3           0x00300000
  #define PINSEL7_P3_26_RESET            0x00300000
#define PINSEL8        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x20))
    #define PINSEL7_P4_0_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL7_P4_0_A0                0x00000001                      // Only LPC2378
  #define PINSEL7_P4_0_RESET             0x00000003                      // Only LPC2378
  #define PINSEL7_P4_1_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL7_P4_1_A1                0x00000004                      // Only LPC2378
  #define PINSEL7_P4_1_RESET             0x0000000c                      // Only LPC2378
  #define PINSEL7_P4_2_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL7_P4_2_A2                0x00000010                      // Only LPC2378
  #define PINSEL7_P4_2_RESET             0x00000030                      // Only LPC2378
  #define PINSEL7_P4_3_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL7_P4_3_A3                0x00000040                      // Only LPC2378
  #define PINSEL7_P4_3_RESET             0x000000c0                      // Only LPC2378
  #define PINSEL7_P4_4_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL7_P4_4_A4                0x00000100                      // Only LPC2378
  #define PINSEL7_P4_4_RESET             0x00000300                      // Only LPC2378
  #define PINSEL7_P4_5_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL7_P4_5_A5                0x00000400                      // Only LPC2378
  #define PINSEL7_P4_5_RESET             0x00000c00                      // Only LPC2378
  #define PINSEL7_P4_6_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL7_P4_6_A6                0x00001000                      // Only LPC2378
  #define PINSEL7_P4_6_RESET             0x00003000                      // Only LPC2378
  #define PINSEL7_P4_7_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL7_P4_7_A7                0x00004000                      // Only LPC2378
  #define PINSEL7_P4_7_RESET             0x0000c000                      // Only LPC2378
  #define PINSEL7_P4_8_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL7_P4_8_A8                0x00010000                      // Only LPC2378
  #define PINSEL7_P4_8_RESET             0x00030000                      // Only LPC2378
  #define PINSEL7_P4_9_GPIO              0x00000000                      // Only LPC2378
  #define PINSEL7_P4_9_A9                0x00040000                      // Only LPC2378
  #define PINSEL7_P4_9_RESET             0x000c0000                      // Only LPC2378
  #define PINSEL7_P4_10_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL7_P4_10_A10              0x00100000                      // Only LPC2378
  #define PINSEL7_P4_10_RESET            0x00300000                      // Only LPC2378
  #define PINSEL7_P4_11_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL7_P4_11_A11              0x00400000                      // Only LPC2378
  #define PINSEL7_P4_11_RESET            0x00c00000                      // Only LPC2378
  #define PINSEL7_P4_12_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL7_P4_12_A12              0x01000000                      // Only LPC2378
  #define PINSEL7_P4_12_RESET            0x03000000                      // Only LPC2378
  #define PINSEL7_P4_13_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL7_P4_13_A13              0x04000000                      // Only LPC2378
  #define PINSEL7_P4_13_RESET            0x0c000000                      // Only LPC2378
  #define PINSEL7_P4_14_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL7_P4_14_A14              0x10000000                      // Only LPC2378
  #define PINSEL7_P4_14_RESET            0x30000000                      // Only LPC2378
  #define PINSEL7_P4_15_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL7_P4_15_A15              0x40000000                      // Only LPC2378
  #define PINSEL7_P4_15_RESET            0xc0000000                      // Only LPC2378
#define PINSEL9        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x24))
    #define PINSEL9_P4_24_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL9_P4_24_A24              0x00010000                      // Only LPC2378
  #define PINSEL9_P4_24_OE               0x00020000                      // Only LPC2378
  #define PINSEL9_P4_24_RESET            0x00030000                      // Only LPC2378
  #define PINSEL9_P4_25_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL9_P4_25_A25              0x00040000                      // Only LPC2378
  #define PINSEL9_P4_25_WE               0x00080000                      // Only LPC2378
  #define PINSEL9_P4_25_RESET            0x000c0000                      // Only LPC2378
  #define PINSEL9_P4_30_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL9_P4_30_CS0              0x10000000                      // Only LPC2378
  #define PINSEL9_P4_30_RESET            0x30000000                      // Only LPC2378
  #define PINSEL9_P4_31_GPIO             0x00000000                      // Only LPC2378
  #define PINSEL9_P4_31_CS1              0x40000000                      // Only LPC2378
  #define PINSEL9_P4_31_RESET            0xc0000000                      // Only LPC2378
#define PINSEL10       (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x28))
  #define GPIO_TRACE                     0x00000008                      // Set if RTCK is '0' at reset - ETM signals are then available regardless of PINSEL4 content

#define PINMODE0        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x40))
#define PINMODE1        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x44))
#define PINMODE2        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x48))
#define PINMODE3        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x4C))
#define PINMODE4        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x50))
#define PINMODE5        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x54))
#define PINMODE6        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x58))
#define PINMODE7        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x5C))
#define PINMODE8        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x60))
#define PINMODE9        (*(volatile unsigned int *)(PINSEL_BASE_ADDR + 0x64))

// General Purpose Input/Output (GPIO) 
#define GPIO_BASE_ADDR		0xE0028000
#define IOPIN0         (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x00))
#define IOSET0         (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x04))
#define IODIR0         (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x08))
#define IOCLR0         (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x0C))
#define IOPIN1         (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x10))
#define IOSET1         (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x14))
#define IODIR1         (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x18))
#define IOCLR1         (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x1C))

// GPIO Interrupt Registers 
#define IO0_INT_EN_R    (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x90)) 
#define IO0_INT_EN_F    (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x94))
#define IO0_INT_STAT_R  (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x84))
#define IO0_INT_STAT_F  (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x88))
#define IO0_INT_CLR     (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x8C))

#define IO2_INT_EN_R    (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0xB0)) 
#define IO2_INT_EN_F    (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0xB4))
#define IO2_INT_STAT_R  (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0xA4))
#define IO2_INT_STAT_F  (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0xA8))
#define IO2_INT_CLR     (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0xAC))

#define IO_INT_STAT     (*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x80))

#define PARTCFG_BASE_ADDR		0x3FFF8000
#define PARTCFG        (*(volatile unsigned int *)(PARTCFG_BASE_ADDR + 0x00)) 

// Fast I/O setup 
#define FIO_BASE_ADDR		0x3FFFC000
#define FIO0DIR        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x00)) 
#define FIO0MASK       (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x10))
#define FIO0PIN        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x14))
#define FIO0SET        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x18))
#define FIO0CLR        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x1C))

#define FIO1DIR        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x20)) 
#define FIO1MASK       (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x30))
#define FIO1PIN        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x34))
#define FIO1SET        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x38))
#define FIO1CLR        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x3C))

#define FIO2DIR        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x40)) 
#define FIO2MASK       (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x50))
#define FIO2PIN        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x54))
#define FIO2SET        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x58))
#define FIO2CLR        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x5C))

#define FIO3DIR        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x60)) 
#define FIO3MASK       (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x70))
#define FIO3PIN        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x74))
#define FIO3SET        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x78))
#define FIO3CLR        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x7C))

#define FIO4DIR        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x80)) 
#define FIO4MASK       (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x90))
#define FIO4PIN        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x94))
#define FIO4SET        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x98))
#define FIO4CLR        (*(volatile unsigned int *)(FIO_BASE_ADDR + 0x9C))

// FIOs can be accessed through WORD, HALF-WORD or BYTE. 
#define FIO0DIR0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x01)) 
#define FIO1DIR0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x21)) 
#define FIO2DIR0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x41)) 
#define FIO3DIR0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x61)) 
#define FIO4DIR0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x81)) 

#define FIO0DIR1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x02)) 
#define FIO1DIR1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x22)) 
#define FIO2DIR1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x42)) 
#define FIO3DIR1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x62)) 
#define FIO4DIR1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x82)) 

#define FIO0DIR2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x03)) 
#define FIO1DIR2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x23)) 
#define FIO2DIR2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x43)) 
#define FIO3DIR2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x63)) 
#define FIO4DIR2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x83)) 

#define FIO0DIR3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x04)) 
#define FIO1DIR3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x24)) 
#define FIO2DIR3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x44)) 
#define FIO3DIR3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x64)) 
#define FIO4DIR3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x84)) 

#define FIO0DIRL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x00)) 
#define FIO1DIRL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x20)) 
#define FIO2DIRL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x40)) 
#define FIO3DIRL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x60)) 
#define FIO4DIRL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x80)) 

#define FIO0DIRU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x02)) 
#define FIO1DIRU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x22)) 
#define FIO2DIRU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x42)) 
#define FIO3DIRU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x62)) 
#define FIO4DIRU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x82)) 

#define FIO0MASK0      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x10)) 
#define FIO1MASK0      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x30)) 
#define FIO2MASK0      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x50)) 
#define FIO3MASK0      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x70)) 
#define FIO4MASK0      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x90)) 

#define FIO0MASK1      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x11)) 
#define FIO1MASK1      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x21)) 
#define FIO2MASK1      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x51)) 
#define FIO3MASK1      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x71)) 
#define FIO4MASK1      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x91)) 

#define FIO0MASK2      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x12)) 
#define FIO1MASK2      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x32)) 
#define FIO2MASK2      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x52)) 
#define FIO3MASK2      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x72)) 
#define FIO4MASK2      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x92)) 

#define FIO0MASK3      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x13)) 
#define FIO1MASK3      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x33)) 
#define FIO2MASK3      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x53)) 
#define FIO3MASK3      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x73)) 
#define FIO4MASK3      (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x93)) 

#define FIO0MASKL      (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x10)) 
#define FIO1MASKL      (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x30)) 
#define FIO2MASKL      (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x50)) 
#define FIO3MASKL      (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x70)) 
#define FIO4MASKL      (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x90)) 

#define FIO0MASKU      (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x12)) 
#define FIO1MASKU      (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x32)) 
#define FIO2MASKU      (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x52)) 
#define FIO3MASKU      (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x72)) 
#define FIO4MASKU      (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x92)) 

#define FIO0PIN0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x14)) 
#define FIO1PIN0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x34)) 
#define FIO2PIN0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x54)) 
#define FIO3PIN0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x74)) 
#define FIO4PIN0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x94)) 

#define FIO0PIN1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x15)) 
#define FIO1PIN1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x25)) 
#define FIO2PIN1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x55)) 
#define FIO3PIN1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x75)) 
#define FIO4PIN1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x95)) 

#define FIO0PIN2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x16)) 
#define FIO1PIN2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x36)) 
#define FIO2PIN2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x56)) 
#define FIO3PIN2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x76)) 
#define FIO4PIN2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x96)) 

#define FIO0PIN3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x17)) 
#define FIO1PIN3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x37)) 
#define FIO2PIN3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x57)) 
#define FIO3PIN3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x77)) 
#define FIO4PIN3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x97)) 

#define FIO0PINL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x14)) 
#define FIO1PINL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x34)) 
#define FIO2PINL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x54)) 
#define FIO3PINL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x74)) 
#define FIO4PINL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x94)) 

#define FIO0PINU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x16)) 
#define FIO1PINU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x36)) 
#define FIO2PINU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x56)) 
#define FIO3PINU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x76)) 
#define FIO4PINU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x96)) 

#define FIO0SET0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x18)) 
#define FIO1SET0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x38)) 
#define FIO2SET0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x58)) 
#define FIO3SET0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x78)) 
#define FIO4SET0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x98)) 

#define FIO0SET1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x19)) 
#define FIO1SET1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x29)) 
#define FIO2SET1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x59)) 
#define FIO3SET1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x79)) 
#define FIO4SET1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x99)) 

#define FIO0SET2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x1A)) 
#define FIO1SET2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x3A)) 
#define FIO2SET2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x5A)) 
#define FIO3SET2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x7A)) 
#define FIO4SET2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x9A)) 

#define FIO0SET3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x1B)) 
#define FIO1SET3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x3B)) 
#define FIO2SET3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x5B)) 
#define FIO3SET3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x7B)) 
#define FIO4SET3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x9B)) 

#define FIO0SETL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x18)) 
#define FIO1SETL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x38)) 
#define FIO2SETL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x58)) 
#define FIO3SETL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x78)) 
#define FIO4SETL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x98)) 

#define FIO0SETU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x1A)) 
#define FIO1SETU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x3A)) 
#define FIO2SETU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x5A)) 
#define FIO3SETU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x7A)) 
#define FIO4SETU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x9A)) 

#define FIO0CLR0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x1C)) 
#define FIO1CLR0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x3C)) 
#define FIO2CLR0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x5C)) 
#define FIO3CLR0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x7C)) 
#define FIO4CLR0       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x9C)) 

#define FIO0CLR1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x1D)) 
#define FIO1CLR1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x2D)) 
#define FIO2CLR1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x5D)) 
#define FIO3CLR1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x7D)) 
#define FIO4CLR1       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x9D)) 

#define FIO0CLR2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x1E)) 
#define FIO1CLR2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x3E)) 
#define FIO2CLR2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x5E)) 
#define FIO3CLR2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x7E)) 
#define FIO4CLR2       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x9E)) 

#define FIO0CLR3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x1F)) 
#define FIO1CLR3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x3F)) 
#define FIO2CLR3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x5F)) 
#define FIO3CLR3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x7F)) 
#define FIO4CLR3       (*(volatile unsigned char  *)(FIO_BASE_ADDR + 0x9F)) 

#define FIO0CLRL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x1C)) 
#define FIO1CLRL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x3C)) 
#define FIO2CLRL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x5C)) 
#define FIO3CLRL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x7C)) 
#define FIO4CLRL       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x9C)) 

#define FIO0CLRU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x1E)) 
#define FIO1CLRU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x3E)) 
#define FIO2CLRU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x5E)) 
#define FIO3CLRU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x7E)) 
#define FIO4CLRU       (*(volatile unsigned short *)(FIO_BASE_ADDR + 0x9E))

  #define PORT0_BIT0                     0x00000001
  #define PORT0_BIT1                     0x00000002
  #define PORT0_BIT2                     0x00000004
  #define PORT0_BIT3                     0x00000008
  #define PORT0_BIT4                     0x00000010
  #define PORT0_BIT5                     0x00000020
  #define PORT0_BIT6                     0x00000040
  #define PORT0_BIT7                     0x00000080
  #define PORT0_BIT8                     0x00000100
  #define PORT0_BIT9                     0x00000200
  #define PORT0_BIT10                    0x00000400
  #define PORT0_BIT11                    0x00000800
  #define PORT0_BIT12                    0x00001000
  #define PORT0_BIT13                    0x00002000
  #define PORT0_BIT14                    0x00004000
  #define PORT0_BIT15                    0x00008000
  #define PORT0_BIT16                    0x00010000
  #define PORT0_BIT17                    0x00020000
  #define PORT0_BIT18                    0x00040000
  #define PORT0_BIT19                    0x00080000
  #define PORT0_BIT20                    0x00100000
  #define PORT0_BIT21                    0x00200000
  #define PORT0_BIT22                    0x00400000
  #define PORT0_BIT23                    0x00800000
  #define PORT0_BIT24                    0x01000000
  #define PORT0_BIT25                    0x02000000
  #define PORT0_BIT26                    0x04000000
  #define PORT0_BIT27                    0x08000000
  #define PORT0_BIT28                    0x10000000
  #define PORT0_BIT29                    0x20000000
  #define PORT0_BIT30                    0x40000000
  #define PORT0_BIT31                    0x80000000

  #define PORT1_BIT0                     0x00000001
  #define PORT1_BIT1                     0x00000002
  #define PORT1_BIT2                     0x00000004
  #define PORT1_BIT3                     0x00000008
  #define PORT1_BIT4                     0x00000010
  #define PORT1_BIT5                     0x00000020
  #define PORT1_BIT6                     0x00000040
  #define PORT1_BIT7                     0x00000080
  #define PORT1_BIT8                     0x00000100
  #define PORT1_BIT9                     0x00000200
  #define PORT1_BIT10                    0x00000400
  #define PORT1_BIT11                    0x00000800
  #define PORT1_BIT12                    0x00001000
  #define PORT1_BIT13                    0x00002000
  #define PORT1_BIT14                    0x00004000
  #define PORT1_BIT15                    0x00008000
  #define PORT1_BIT16                    0x00010000
  #define PORT1_BIT17                    0x00020000
  #define PORT1_BIT18                    0x00040000
  #define PORT1_BIT19                    0x00080000
  #define PORT1_BIT20                    0x00100000
  #define PORT1_BIT21                    0x00200000
  #define PORT1_BIT22                    0x00400000
  #define PORT1_BIT23                    0x00800000
  #define PORT1_BIT24                    0x01000000
  #define PORT1_BIT25                    0x02000000
  #define PORT1_BIT26                    0x04000000
  #define PORT1_BIT27                    0x08000000
  #define PORT1_BIT28                    0x10000000
  #define PORT1_BIT29                    0x20000000
  #define PORT1_BIT30                    0x40000000
  #define PORT1_BIT31                    0x80000000

  #define PORT2_BIT0                     0x00000001
  #define PORT2_BIT1                     0x00000002
  #define PORT2_BIT2                     0x00000004
  #define PORT2_BIT3                     0x00000008
  #define PORT2_BIT4                     0x00000010
  #define PORT2_BIT5                     0x00000020
  #define PORT2_BIT6                     0x00000040
  #define PORT2_BIT7                     0x00000080
  #define PORT2_BIT8                     0x00000100
  #define PORT2_BIT9                     0x00000200
  #define PORT2_BIT10                    0x00000400
  #define PORT2_BIT11                    0x00000800
  #define PORT2_BIT12                    0x00001000
  #define PORT2_BIT13                    0x00002000
  #define PORT2_BIT14                    0x00004000
  #define PORT2_BIT15                    0x00008000
  #define PORT2_BIT16                    0x00010000
  #define PORT2_BIT17                    0x00020000
  #define PORT2_BIT18                    0x00040000
  #define PORT2_BIT19                    0x00080000
  #define PORT2_BIT20                    0x00100000
  #define PORT2_BIT21                    0x00200000
  #define PORT2_BIT22                    0x00400000
  #define PORT2_BIT23                    0x00800000
  #define PORT2_BIT24                    0x01000000
  #define PORT2_BIT25                    0x02000000
  #define PORT2_BIT26                    0x04000000
  #define PORT2_BIT27                    0x08000000
  #define PORT2_BIT28                    0x10000000
  #define PORT2_BIT29                    0x20000000
  #define PORT2_BIT30                    0x40000000
  #define PORT2_BIT31                    0x80000000

  #define PORT3_BIT0                     0x00000001
  #define PORT3_BIT1                     0x00000002
  #define PORT3_BIT2                     0x00000004
  #define PORT3_BIT3                     0x00000008
  #define PORT3_BIT4                     0x00000010
  #define PORT3_BIT5                     0x00000020
  #define PORT3_BIT6                     0x00000040
  #define PORT3_BIT7                     0x00000080
  #define PORT3_BIT8                     0x00000100
  #define PORT3_BIT9                     0x00000200
  #define PORT3_BIT10                    0x00000400
  #define PORT3_BIT11                    0x00000800
  #define PORT3_BIT12                    0x00001000
  #define PORT3_BIT13                    0x00002000
  #define PORT3_BIT14                    0x00004000
  #define PORT3_BIT15                    0x00008000
  #define PORT3_BIT16                    0x00010000
  #define PORT3_BIT17                    0x00020000
  #define PORT3_BIT18                    0x00040000
  #define PORT3_BIT19                    0x00080000
  #define PORT3_BIT20                    0x00100000
  #define PORT3_BIT21                    0x00200000
  #define PORT3_BIT22                    0x00400000
  #define PORT3_BIT23                    0x00800000
  #define PORT3_BIT24                    0x01000000
  #define PORT3_BIT25                    0x02000000
  #define PORT3_BIT26                    0x04000000
  #define PORT3_BIT27                    0x08000000
  #define PORT3_BIT28                    0x10000000
  #define PORT3_BIT29                    0x20000000
  #define PORT3_BIT30                    0x40000000
  #define PORT3_BIT31                    0x80000000

  #define PORT4_BIT0                     0x00000001
  #define PORT4_BIT1                     0x00000002
  #define PORT4_BIT2                     0x00000004
  #define PORT4_BIT3                     0x00000008
  #define PORT4_BIT4                     0x00000010
  #define PORT4_BIT5                     0x00000020
  #define PORT4_BIT6                     0x00000040
  #define PORT4_BIT7                     0x00000080
  #define PORT4_BIT8                     0x00000100
  #define PORT4_BIT9                     0x00000200
  #define PORT4_BIT10                    0x00000400
  #define PORT4_BIT11                    0x00000800
  #define PORT4_BIT12                    0x00001000
  #define PORT4_BIT13                    0x00002000
  #define PORT4_BIT14                    0x00004000
  #define PORT4_BIT15                    0x00008000
  #define PORT4_BIT16                    0x00010000
  #define PORT4_BIT17                    0x00020000
  #define PORT4_BIT18                    0x00040000
  #define PORT4_BIT19                    0x00080000
  #define PORT4_BIT20                    0x00100000
  #define PORT4_BIT21                    0x00200000
  #define PORT4_BIT22                    0x00400000
  #define PORT4_BIT23                    0x00800000
  #define PORT4_BIT24                    0x01000000
  #define PORT4_BIT25                    0x02000000
  #define PORT4_BIT26                    0x04000000
  #define PORT4_BIT27                    0x08000000
  #define PORT4_BIT28                    0x10000000
  #define PORT4_BIT29                    0x20000000
  #define PORT4_BIT30                    0x40000000
  #define PORT4_BIT31                    0x80000000


// System Control Block(SCB) modules include Memory Accelerator Module,
// Phase Locked Loop, VPB divider, Power Control, External Interrupt, 
// Reset, and Code Security/Debugging 
#define SCB_BASE_ADDR	0xE01FC000

/* Memory Accelerator Module (MAM) */
#define MAMCR          (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x000))
  #define MAM_DISABLED                   0x00000000
  #define MAM_PARTIALLY_ENABLED          0x00000001
  #define MAM_FULLY_ENABLED              0x00000002
#define MAMTIM         (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x004))
  #define MAM_FETCH_CLOCKS_1             0x00000001
  #define MAM_FETCH_CLOCKS_2             0x00000002
  #define MAM_FETCH_CLOCKS_3             0x00000003
  #define MAM_FETCH_CLOCKS_4             0x00000004	
#define MEMMAP         (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x040))

// Phase Locked Loop (PLL) 
#define PLLCON         (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x080))
  #define PLLE                           0x00000001                                         // enable PLL
  #define PLLC                           0x00000002                                         // connect PLL
#define PLLCFG         (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x084))
#define PLLSTAT        (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x088))
  #define PLLENABLED                     0x01000000                                         // status of enable bit
  #define PLLCONNECTED                   0x02000000                                         // status of connected bit
  #define PLLLOCKED                      0x04000000                                         // PLL lock status status
#define PLLFEED        (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x08C))

// Power Control 
#define PCON           (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x0C0))
#define PCONP          (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x0C4))
  #define PCTIM0                         0x00000002
  #define PCTIM1                         0x00000004
  #define PCUART0                        0x00000008
  #define PCUART1                        0x00000010
  #define PCPWM0                         0x00000020
  #define PCPWM1                         0x00000040
  #define PCI2C0                         0x00000080
  #define PCSPI                          0x00000100
  #define PCRTC                          0x00000200
  #define PCSSP1                         0x00000400
  #define PCEMC                          0x00000800
  #define PCPCAD                         0x00001000
  #define PCPCAN1                        0x00002000
  #define PCPCAN2                        0x00004000
  #define PCI2C1                         0x00080000
  #define PCSSP0                         0x00200000
  #define PCTIM2                         0x00400000
  #define PCTIM3                         0x00800000
  #define PCUART2                        0x01000000
  #define PCUART3                        0x02000000
  #define PCI2C2                         0x04000000
  #define PCI2S                          0x08000000
  #define PCSDC                          0x10000000
  #define PCGPDMA                        0x20000000
  #define PCENET                         0x40000000
  #define PCUSB                          0x80000000

// Clock Divider 
// #define APBDIV         (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x100))
#define CCLKCFG        (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x104))
#define USBCLKCFG      (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x108))
#define CLKSRCSEL      (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x10C))
  #define PLL_SOURCE_4M_RC               0x00000000                                          // PLL source is 4MHz RC oscillator
  #define PLL_SOURCE_OSCILLATOR          0x00000001                                          // PLL source is main oscillator
  #define PLL_SOURCE_RTC                 0x00000002                                          // PLL source is RTC oscillator
#define PCLKSEL0       (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x1A8))
  #define PCLK_WDT_CCLK_4                0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_WDT_CCLK_1                0x00000001                                          // PCLK to this peripheral is CCLK
  #define PCLK_WDT_CCLK_2                0x00000002                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_WDT_CCLK_8                0x00000003                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_TIMER0_CCLK_4             0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_TIMER0_CCLK_1             0x00000004                                          // PCLK to this peripheral is CCLK
  #define PCLK_TIMER0_CCLK_2             0x00000008                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_TIMER0_CCLK_8             0x0000000c                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_TIMER1_CCLK_4             0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_TIMER1_CCLK_1             0x00000010                                          // PCLK to this peripheral is CCLK
  #define PCLK_TIMER1_CCLK_2             0x00000020                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_TIMER1_CCLK_8             0x00000030                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_UART0_CCLK_4              0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_UART0_CCLK_1              0x00000040                                          // PCLK to this peripheral is CCLK
  #define PCLK_UART0_CCLK_2              0x00000080                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_UART0_CCLK_8              0x000000c0                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_UART1_CCLK_4              0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_UART1_CCLK_1              0x00000100                                          // PCLK to this peripheral is CCLK
  #define PCLK_UART1_CCLK_2              0x00000200                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_UART1_CCLK_8              0x00000300                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_PWM0_CCLK_4               0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_PWM0_CCLK_1               0x00000400                                          // PCLK to this peripheral is CCLK
  #define PCLK_PWM0_CCLK_2               0x00000800                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_PWM0_CCLK_8               0x00000c00                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_PWM1_CCLK_4               0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_PWM1_CCLK_1               0x00001000                                          // PCLK to this peripheral is CCLK
  #define PCLK_PWM1_CCLK_2               0x00002000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_PWM1_CCLK_8               0x00003000                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_I2C0_CCLK_4               0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_I2C0_CCLK_1               0x00004000                                          // PCLK to this peripheral is CCLK
  #define PCLK_I2C0_CCLK_2               0x00008000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_I2C0_CCLK_8               0x0000c000                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_SPI_CCLK_4                0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_SPI_CCLK_1                0x00010000                                          // PCLK to this peripheral is CCLK
  #define PCLK_SPI_CCLK_2                0x00020000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_SPI_CCLK_8                0x00030000                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_RTC_CCLK_4                0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_RTC_CCLK_2                0x00080000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_RTC_CCLK_8                0x000c0000                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_SSP1_CCLK_4               0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_SSP1_CCLK_1               0x00100000                                          // PCLK to this peripheral is CCLK
  #define PCLK_SSP1_CCLK_2               0x00200000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_SSP1_CCLK_8               0x00300000                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_DAC_CCLK_4                0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_DAC_CCLK_1                0x00400000                                          // PCLK to this peripheral is CCLK
  #define PCLK_DAC_CCLK_2                0x00800000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_DAC_CCLK_8                0x00c00000                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_ADC_CCLK_4                0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_ADC_CCLK_1                0x01000000                                          // PCLK to this peripheral is CCLK
  #define PCLK_ADC_CCLK_2                0x02000000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_ADC_CCLK_8                0x03000000                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_CAN1_CCLK_4               0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_CAN1_CCLK_1               0x04000000                                          // PCLK to this peripheral is CCLK
  #define PCLK_CAN1_CCLK_2               0x08000000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_CAN1_CCLK_6               0x0c000000                                          // PCLK to this peripheral is CCLK/6
  #define PCLK_CAN2_CCLK_4               0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_CAN2_CCLK_1               0x10000000                                          // PCLK to this peripheral is CCLK
  #define PCLK_CAN2_CCLK_2               0x20000000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_CAN2_CCLK_6               0x30000000                                          // PCLK to this peripheral is CCLK/6
  #define PCLK_ACF_CCLK_4                0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_ACF_CCLK_1                0x40000000                                          // PCLK to this peripheral is CCLK
  #define PCLK_ACF_CCLK_2                0x80000000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_ACF_CCLK_6                0xc0000000                                          // PCLK to this peripheral is CCLK/6
#define PCLKSEL1       (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x1AC))
  #define PCLK_BAT_RAM_CCLK_4            0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_BAT_RAM_CCLK_1            0x00000001                                          // PCLK to this peripheral is CCLK
  #define PCLK_BAT_RAM_CCLK_2            0x00000002                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_BAT_RAM_CCLK_8            0x00000003                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_GPIO_CCLK_4               0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_GPIO_CCLK_1               0x00000004                                          // PCLK to this peripheral is CCLK
  #define PCLK_GPIO_CCLK_2               0x00000008                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_GPIO_CCLK_8               0x0000000c                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_PCB_CCLK_4                0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_PCB_CCLK_1                0x00000010                                          // PCLK to this peripheral is CCLK
  #define PCLK_PCB_CCLK_2                0x00000020                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_PCB_CCLK_8                0x00000030                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_I2C1_CCLK_4               0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_I2C1_CCLK_1               0x00000040                                          // PCLK to this peripheral is CCLK
  #define PCLK_I2C1_CCLK_2               0x00000080                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_I2C1_CCLK_8               0x000000c0                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_SSP0_CCLK_4               0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_SSP0_CCLK_1               0x00000400                                          // PCLK to this peripheral is CCLK
  #define PCLK_SSP0_CCLK_2               0x00000800                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_SSP0_CCLK_8               0x00000c00                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_TIMER2_CCLK_4             0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_TIMER2_CCLK_1             0x00001000                                          // PCLK to this peripheral is CCLK
  #define PCLK_TIMER2_CCLK_2             0x00002000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_TIMER2_CCLK_8             0x00003000                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_TIMER3_CCLK_4             0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_TIMER3_CCLK_1             0x00004000                                          // PCLK to this peripheral is CCLK
  #define PCLK_TIMER3_CCLK_2             0x00008000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_TIMER3_CCLK_8             0x0000c000                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_UART2_CCLK_4              0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_UART2_CCLK_1              0x00010000                                          // PCLK to this peripheral is CCLK
  #define PCLK_UART2_CCLK_2              0x00020000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_UART2_CCLK_8              0x00030000                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_UART3_CCLK_4              0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_UART3_CCLK_1              0x00040000                                          // PCLK to this peripheral is CCLK
  #define PCLK_UART3_CCLK_2              0x00080000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_UART3_CCLK_8              0x000c0000                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_I2C2_CCLK_4               0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_I2C2_CCLK_1               0x00100000                                          // PCLK to this peripheral is CCLK
  #define PCLK_I2C2_CCLK_2               0x00200000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_I2C2_CCLK_8               0x00300000                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_I2S_CCLK_4                0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_I2S_CCLK_1                0x00400000                                          // PCLK to this peripheral is CCLK
  #define PCLK_I2S_CCLK_2                0x00800000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_I2S_CCLK_8                0x00c00000                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_MCI_CCLK_4                0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_MCI_CCLK_1                0x01000000                                          // PCLK to this peripheral is CCLK
  #define PCLK_MCI_CCLK_2                0x02000000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_MCI_CCLK_8                0x03000000                                          // PCLK to this peripheral is CCLK/8
  #define PCLK_PCLK_SYSCON_CCLK_4        0x00000000                                          // PCLK to this peripheral is CCLK/4
  #define PCLK_PCLK_SYSCON_CCLK_1        0x10000000                                          // PCLK to this peripheral is CCLK
  #define PCLK_PCLK_SYSCON_CCLK_2        0x20000000                                          // PCLK to this peripheral is CCLK/2
  #define PCLK_PCLK_SYSCON_CCLK_8        0x30000000                                          // PCLK to this peripheral is CCLK/8
	
// External Interrupts 
#define EXTINT         (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x140))
#define INTWAKE        (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x144))
#define EXTMODE        (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x148))
#define EXTPOLAR       (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x14C))

// Reset, reset source identification 
#define RSIR           (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x180))
  #define POR                            0x00000001                                          // Power On reset
  #define EXTR                           0x00000002                                          // External reset
  #define WDTR                           0x00000004                                          // Watchdog Timer reset
  #define BODR                           0x00000008                                          // Brownout reset

// RSID, code security protection 
#define CSPR           (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x184))

// AHB configuration 
#define AHBCFG1        (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x188))
#define AHBCFG2        (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x18C))

// System Controls and Status 
#define SCS            (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x1A0))
  #define GPIOM                          0x00000001                                          // High speed GPIO mode enable
  #define EMC_RESET_DISABLE              0x00000002                                          // Preserve EMC state through warm reset
  #define MCIPWR                         0x00000008                                          // Active high level on this pin
  #define OSCRANGE                       0x00000010                                          // Main oscillator range set
  #define OSCEN                          0x00000020                                          // Enable external oscillator
  #define OSCSTAT                        0x00000040                                          // main oscillator status - ready for use (read only bit)

// MPMC(EMC) registers
#define STATIC_MEM0_BASE		0x80000000
#define STATIC_MEM1_BASE		0x81000000
#define STATIC_MEM2_BASE		0x82000000
#define STATIC_MEM3_BASE		0x83000000

#define DYNAMIC_MEM0_BASE		0xA0000000
#define DYNAMIC_MEM1_BASE		0xB0000000
#define DYNAMIC_MEM2_BASE		0xC0000000
#define DYNAMIC_MEM3_BASE		0xD0000000

/* External Memory Controller (EMC) */
#define EMC_BASE_ADDR		0xFFE08000
#define EMC_CTRL       (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x000))
#define EMC_STAT       (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x004))
#define EMC_CONFIG     (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x008))

/* Dynamic RAM access registers */
#define EMC_DYN_CTRL     (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x020))
#define EMC_DYN_RFSH     (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x024))
#define EMC_DYN_RD_CFG   (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x028))
#define EMC_DYN_RP       (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x030))
#define EMC_DYN_RAS      (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x034))
#define EMC_DYN_SREX     (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x038))
#define EMC_DYN_APR      (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x03C))
#define EMC_DYN_DAL      (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x040))
#define EMC_DYN_WR       (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x044))
#define EMC_DYN_RC       (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x048))
#define EMC_DYN_RFC      (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x04C))
#define EMC_DYN_XSR      (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x050))
#define EMC_DYN_RRD      (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x054))
#define EMC_DYN_MRD      (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x058))

#define EMC_DYN_CFG0     (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x100))
#define EMC_DYN_RASCAS0  (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x104))
#define EMC_DYN_CFG1     (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x140))
#define EMC_DYN_RASCAS1  (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x144))
#define EMC_DYN_CFG2     (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x160))
#define EMC_DYN_RASCAS2  (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x164))
#define EMC_DYN_CFG3     (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x180))
#define EMC_DYN_RASCAS3  (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x184))

/* static RAM access registers */
#define EMC_STA_CFG0      (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x200))
#define EMC_STA_WAITWEN0  (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x204))
#define EMC_STA_WAITOEN0  (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x208))
#define EMC_STA_WAITRD0   (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x20C))
#define EMC_STA_WAITPAGE0 (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x210))
#define EMC_STA_WAITWR0   (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x214))
#define EMC_STA_WAITTURN0 (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x218))

#define EMC_STA_CFG1      (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x220))
#define EMC_STA_WAITWEN1  (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x224))
#define EMC_STA_WAITOEN1  (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x228))
#define EMC_STA_WAITRD1   (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x22C))
#define EMC_STA_WAITPAGE1 (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x230))
#define EMC_STA_WAITWR1   (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x234))
#define EMC_STA_WAITTURN1 (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x238))

#define EMC_STA_CFG2      (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x240))
#define EMC_STA_WAITWEN2  (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x244))
#define EMC_STA_WAITOEN2  (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x248))
#define EMC_STA_WAITRD2   (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x24C))
#define EMC_STA_WAITPAGE2 (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x250))
#define EMC_STA_WAITWR2   (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x254))
#define EMC_STA_WAITTURN2 (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x258))

#define EMC_STA_CFG3      (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x260))
#define EMC_STA_WAITWEN3  (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x264))
#define EMC_STA_WAITOEN3  (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x268))
#define EMC_STA_WAITRD3   (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x26C))
#define EMC_STA_WAITPAGE3 (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x270))
#define EMC_STA_WAITWR3   (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x274))
#define EMC_STA_WAITTURN3 (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x278))

#define EMC_STA_EXT_WAIT  (*(volatile unsigned int *)(EMC_BASE_ADDR + 0x880))

	
// Timer 0..3 
#define TMR0_BASE_ADDR		0xE0004000
#define T0IR           (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x00))
  #define MR0_INTERRUPT                  0x00000001
  #define MR1_INTERRUPT                  0x00000002
  #define MR2_INTERRUPT                  0x00000004
  #define MR3_INTERRUPT                  0x00000008
  #define CR0_INTERRUPT                  0x00000010
  #define CR1_INTERRUPT                  0x00000020
  #define CR2_INTERRUPT                  0x00000040
  #define CR3_INTERRUPT                  0x00000080
#define T0TCR          (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x04))
  #define COUNTER_ENABLE                 0x00000001
  #define COUNTER_RESET                  0x00000002
#define T0TC           (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x08))
#define T0PR           (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x0C))
#define T0PC           (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x10))
#define T0MCR          (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x14))
  #define MR0I                           0x00000001                                         // Interrupt on MR0 match
  #define MR0R                           0x00000002                                         // Reset on MR0 match
  #define MR0S                           0x00000004                                         // Stop on MR0 match
  #define MR1I                           0x00000008                                         // Interrupt on MR1 match
  #define MR1R                           0x00000010                                         // Reset on MR1 match
  #define MR1S                           0x00000020                                         // Stop on MR1 match
  #define MR2I                           0x00000040                                         // Interrupt on MR2 match
  #define MR2R                           0x00000080                                         // Reset on MR2 match
  #define MR2S                           0x00000100                                         // Stop on MR2 match
  #define MR3I                           0x00000200                                         // Interrupt on MR3 match
  #define MR3R                           0x00000400                                         // Reset on MR3 match
  #define MR3S                           0x00000800                                         // Stop on MR3 match
#define T0MR0          (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x18))
#define T0MR1          (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x1C))
#define T0MR2          (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x20))
#define T0MR3          (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x24))
#define T0CCR          (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x28))
  #define CAP0RE                         0x00000001                                         // Capture on Rising edge CAPn.0
  #define CAP0FE                         0x00000002                                         // Capture on Falling edge CAPn.0
  #define CAP0I                          0x00000004                                         // Interrupt on capture event CAPn.0
  #define CAP1RE                         0x00000008                                         // Capture on Rising edge CAPn.1
  #define CAP1FE                         0x00000010                                         // Capture on Falling edge CAPn.1
  #define CAP1I                          0x00000020                                         // Interrupt on capture event CAPn.1
  #define CAP2RE                         0x00000040                                         // Capture on Rising edge CAPn.2
  #define CAP2FE                         0x00000080                                         // Capture on Falling edge CAPn.2
  #define CAP2I                          0x00000100                                         // Interrupt on capture event CAPn.2
  #define CAP3RE                         0x00000200                                         // Capture on Rising edge CAPn.3
  #define CAP3FE                         0x00000400                                         // Capture on Falling edge CAPn.3
  #define CAP3I                          0x00000800                                         // Interrupt on capture event CAPn.3
#define T0CR0          (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x2C))
#define T0CR1          (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x30))
#define T0CR2          (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x34))
#define T0CR3          (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x38))
#define T0EMR          (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x3C))
  #define EM0                            0x00000001                                         // External Match 0
  #define EM1                            0x00000002                                         // External Match 1
  #define EM2                            0x00000004                                         // External Match 2
  #define EM3                            0x00000008                                         // External Match 3
  #define EMC0_DO_NOTHING                0x00000000
  #define EMC0_CLR_ON_MATCH              0x00000010                                         // Clear MATn.0 pin on match
  #define EMC0_SET_ON_MATCH              0x00000020                                         // Set MATn.0 pin on match
  #define EMC0_TOGGLE_ON_MATCH           0x00000030                                         // Toggle MATn.0 pin on match
  #define EMC1_DO_NOTHING                0x00000000
  #define EMC1_CLR_ON_MATCH              0x00000040                                         // Clear MATn.1 pin on match
  #define EMC1_SET_ON_MATCH              0x00000080                                         // Set MATn.1 pin on match
  #define EMC1_TOGGLE_ON_MATCH           0x000000c0                                         // Toggle MATn.1 pin on match
  #define EMC2_DO_NOTHING                0x00000000
  #define EMC2_CLR_ON_MATCH              0x00000100                                         // Clear MATn.2 pin on match
  #define EMC2_SET_ON_MATCH              0x00000200                                         // Set MATn.2 pin on match
  #define EMC2_TOGGLE_ON_MATCH           0x00000300                                         // Toggle MATn.2 pin on match
  #define EMC3_DO_NOTHING                0x00000000
  #define EMC3_CLR_ON_MATCH              0x00000400                                         // Clear MATn.3 pin on match
  #define EMC3_SET_ON_MATCH              0x00000800                                         // Set MATn.3 pin on match
  #define EMC3_TOGGLE_ON_MATCH           0x00000c00                                         // Toggle MATn.3 pin on match
#define T0CTCR         (*(volatile unsigned int *)(TMR0_BASE_ADDR + 0x70))
  #define TIMER_MODE                     0x00000000                                         // count on every rising PCLK edge
  #define COUNTER_MODE_RISING_EDGE       0x00000001
  #define COUNTER_MODE_FALLING_EDGE      0x00000002
  #define COUNTER_MODE_BOTH_EDGES        0x00000003
  #define COUNTER_MODE_INPUT_CAP0        0x00000000
  #define COUNTER_MODE_INPUT_CAP1        0x00000004
  #define COUNTER_MODE_INPUT_CAP2        0x00000008
  #define COUNTER_MODE_INPUT_CAP3        0x0000000c

// Timer 1 
#define TMR1_BASE_ADDR		0xE0008000
#define T1IR           (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x00))
#define T1TCR          (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x04))
#define T1TC           (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x08))
#define T1PR           (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x0C))
#define T1PC           (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x10))
#define T1MCR          (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x14))
#define T1MR0          (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x18))
#define T1MR1          (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x1C))
#define T1MR2          (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x20))
#define T1MR3          (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x24))
#define T1CCR          (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x28))
#define T1CR0          (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x2C))
#define T1CR1          (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x30))
#define T1CR2          (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x34))
#define T1CR3          (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x38))
#define T1EMR          (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x3C))
#define T1CTCR         (*(volatile unsigned int *)(TMR1_BASE_ADDR + 0x70))

// Timer 2 
#define TMR2_BASE_ADDR		0xE0070000
#define T2IR           (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x00))
#define T2TCR          (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x04))
#define T2TC           (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x08))
#define T2PR           (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x0C))
#define T2PC           (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x10))
#define T2MCR          (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x14))
#define T2MR0          (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x18))
#define T2MR1          (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x1C))
#define T2MR2          (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x20))
#define T2MR3          (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x24))
#define T2CCR          (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x28))
#define T2CR0          (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x2C))
#define T2CR1          (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x30))
#define T2CR2          (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x34))
#define T2CR3          (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x38))
#define T2EMR          (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x3C))
#define T2CTCR         (*(volatile unsigned int *)(TMR2_BASE_ADDR + 0x70))

// Timer 3 
#define TMR3_BASE_ADDR		0xE0074000
#define T3IR           (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x00))
#define T3TCR          (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x04))
#define T3TC           (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x08))
#define T3PR           (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x0C))
#define T3PC           (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x10))
#define T3MCR          (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x14))
#define T3MR0          (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x18))
#define T3MR1          (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x1C))
#define T3MR2          (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x20))
#define T3MR3          (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x24))
#define T3CCR          (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x28))
#define T3CR0          (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x2C))
#define T3CR1          (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x30))
#define T3CR2          (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x34))
#define T3CR3          (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x38))
#define T3EMR          (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x3C))
#define T3CTCR         (*(volatile unsigned int *)(TMR3_BASE_ADDR + 0x70))


// Pulse Width Modulator (PWM) 
#define PWM0_BASE_ADDR		0xE0014000
#define PWM0IR          (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x00))
#define PWM0TCR         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x04))
#define PWM0TC          (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x08))
#define PWM0PR          (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x0C))
#define PWM0PC          (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x10))
#define PWM0MCR         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x14))
#define PWM0MR0         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x18))
#define PWM0MR1         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x1C))
#define PWM0MR2         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x20))
#define PWM0MR3         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x24))
#define PWM0CCR         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x28))
#define PWM0CR0         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x2C))
#define PWM0CR1         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x30))
#define PWM0CR2         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x34))
#define PWM0CR3         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x38))
#define PWM0EMR         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x3C))
#define PWM0MR4         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x40))
#define PWM0MR5         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x44))
#define PWM0MR6         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x48))
#define PWM0PCR         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x4C))
#define PWM0LER         (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x50))
#define PWM0CTCR        (*(volatile unsigned int *)(PWM0_BASE_ADDR + 0x70))

#define PWM1_BASE_ADDR		0xE0018000
#define PWM1IR          (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x00))
#define PWM1TCR         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x04))
#define PWM1TC          (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x08))
#define PWM1PR          (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x0C))
#define PWM1PC          (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x10))
#define PWM1MCR         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x14))
#define PWM1MR0         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x18))
#define PWM1MR1         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x1C))
#define PWM1MR2         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x20))
#define PWM1MR3         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x24))
#define PWM1CCR         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x28))
#define PWM1CR0         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x2C))
#define PWM1CR1         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x30))
#define PWM1CR2         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x34))
#define PWM1CR3         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x38))
#define PWM1EMR         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x3C))
#define PWM1MR4         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x40))
#define PWM1MR5         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x44))
#define PWM1MR6         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x48))
#define PWM1PCR         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x4C))
#define PWM1LER         (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x50))
#define PWM1CTCR        (*(volatile unsigned int *)(PWM1_BASE_ADDR + 0x70))


// Universal Asynchronous Receiver Transmitter 0 (UART0) 
#define UART0_BASE_ADDR		0xE000C000
#define U0RBR          (*(volatile unsigned int *)(UART0_BASE_ADDR + 0x00))
#define U0THR          (*(volatile unsigned int *)(UART0_BASE_ADDR + 0x00))
#define U0DLL          (*(volatile unsigned int *)(UART0_BASE_ADDR + 0x00))
#define U0DLM          (*(volatile unsigned int *)(UART0_BASE_ADDR + 0x04))
#define U0IER          (*(volatile unsigned int *)(UART0_BASE_ADDR + 0x04))
  #define UART_RDA_INT                   0x00000001                                      // enable receive data available interrupt
  #define UART_THRE_INT                  0x00000002                                      // enable THRE interrupt
  #define UART_RXLINE_STATUS_INT         0x00000004                                      // enable Rx line status interrupt
  #define UART_ABTOIntEn                 0x00000100                                      // enable Auto-Baud timeout interrupt
  #define UART_ABEOIntEn                 0x00000200                                      // enable End of Auto-Baud interrupt
#define U0IIR          (*(volatile unsigned int *)(UART0_BASE_ADDR + 0x08))
  #define UART_NO_INT_PENDING            0x00000001                                      // no interrupts presently pending
  #define UART_INT_MASK                  0x0000000e                                      // mask for interrupt source
  #define UART_THRE_INT_PENDING          0x00000002                                      // THRE interrupt is pending
  #define UART_RDA_INT_PENDING           0x00000004                                      // Receive Data Available interrupt is pending
  #define UART_RLS_INT_PENDING           0x00000006                                      // Receive Line Status interrupt is pending
  #define UART_CTI_INT_PENDING           0x0000000c                                      // Character Time-out Indicator interrupt is pending
  #define UART_FIFO_ENABLED              0x000000c0                                      // FIFOs are enabled
  #define UART_ABEO_INT_PENDING          0x00000100                                      // Auto-baud interrupt is pending
  #define UART_ABTO_INT_PENDING          0x00000200                                      // Auto-bacu Timeout interrupt is pending
#define U0FCR          (*(volatile unsigned int *)(UART0_BASE_ADDR + 0x08))
  #define FIFO_ENABLE                    0x01                                            // enable FIFO (changes clear FIFOs)
  #define RX_FIFO_RESET                  0x02                                            // Reset Rx FIFO and clear pointer logic (self-clearing bit)
  #define TX_FIFO_RESET                  0x04                                            // Reset Tx FIFO and clear pointer logic (self-clearing bit)
  #define RX_TRIGGER_1_CHAR              0x00                                            // Interrupt for every received character
  #define RX_TRIGGER_4_CHARS             0x40                                            // Interrupt for every 4 received characters
  #define RX_TRIGGER_8_CHARS             0x80                                            // Interrupt for every 8 received characters
  #define RX_TRIGGER_14_CHARS            0xc0                                            // Interrupt for every 14 received characters
#define U0LCR          (*(volatile unsigned int *)(UART0_BASE_ADDR + 0x0C))
  #define UART_5_BIT_CHAR                0x00                                            // set 5 bit characters
  #define UART_6_BIT_CHAR                0x01                                            // set 6 bit characters
  #define UART_7_BIT_CHAR                0x02                                            // set 7 bit characters
  #define UART_8_BIT_CHAR                0x03                                            // set 8 bit characters
  #define UART_1_STOP_BITS               0x00                                            // set 1 stop bits
  #define UART_2_STOP_BITS               0x04                                            // set 2 stop bits (1.5 in 5 bit character mode)
  #define UART_PARITY_DISABLED           0x00                                            // disable parity
  #define UART_PARITY_ENABLE             0x08                                            // enable parity
  #define UART_PARITY_ODD                0x00                                            // set odd parity mode
  #define UART_PARITY_EVEN               0x10                                            // set even parity mode
  #define UART_PARITY_ONE                0x20                                            // set forced '1' stick parity mode
  #define UART_PARITY_ZERO               0x30                                            // set forced '0' stick parity mode
  #define BREAK_CONTROL                  0x40                                            // force break on TXD line
  #define DLAB                           0x80                                            // enable access to divisor latches
#define U0LSR          (*(volatile unsigned int *)(UART0_BASE_ADDR + 0x14))
  #define UART_RDR                       0x01                                            // receiver data ready
  #define UART_OE                        0x02                                            // overrun error
  #define UART_PE                        0x04                                            // parity error
  #define UART_FE                        0x08                                            // framing error
  #define UART_BI                        0x10                                            // break interrupt
  #define UART_THRE                      0x20                                            // transmit holding register empty
  #define UART_TEMT                      0x40                                            // transmitter empty
  #define UART_RXFE                      0x80                                            // error in receive FIFO
#define U0SCR          (*(volatile unsigned int *)(UART0_BASE_ADDR + 0x1C))
#define U0ACR          (*(volatile unsigned int *)(UART0_BASE_ADDR + 0x20))
  #define UART_AUTOBAUD_START            0x00000001                                      // Auto-baud start / running
  #define UART_AUTOBAUD_MODE_0           0x00000000                                      // Auto-baud mode 0
  #define UART_AUTOBAUD_MODE_1           0x00000002                                      // Auto-baud mode 1
  #define UART_AUTOBAUD_AUTO_RESTART     0x00000004                                      // Auto-baud auto-restart in case of timeout
  #define UART_AUTOBAUD_ABEOIntClr       0x00000100                                      // Auto-baud interrupt clear
  #define UART_AUTOBAUD_ABTOIntClr       0x00000200                                      // Auto-baud time-out interrupt clear
#define U0ICR          (*(volatile unsigned int *)(UART0_BASE_ADDR + 0x24))
#define U0FDR          (*(volatile unsigned int *)(UART0_BASE_ADDR + 0x28))
#define U0TER          (*(volatile unsigned int *)(UART0_BASE_ADDR + 0x30))

// Universal Asynchronous Receiver Transmitter 1 (UART1) 
#define UART1_BASE_ADDR		0xE0010000
#define U1RBR          (*(volatile unsigned int *)(UART1_BASE_ADDR + 0x00))
#define U1THR          (*(volatile unsigned int *)(UART1_BASE_ADDR + 0x00))
#define U1DLL          (*(volatile unsigned int *)(UART1_BASE_ADDR + 0x00))
#define U1DLM          (*(volatile unsigned int *)(UART1_BASE_ADDR + 0x04))
#define U1IER          (*(volatile unsigned int *)(UART1_BASE_ADDR + 0x04))
#define U1IIR          (*(volatile unsigned int *)(UART1_BASE_ADDR + 0x08))
#define U1FCR          (*(volatile unsigned int *)(UART1_BASE_ADDR + 0x08))
#define U1LCR          (*(volatile unsigned int *)(UART1_BASE_ADDR + 0x0C))
#define U1MCR          (*(volatile unsigned int *)(UART1_BASE_ADDR + 0x10))
#define U1LSR          (*(volatile unsigned int *)(UART1_BASE_ADDR + 0x14))
#define U1MSR          (*(volatile unsigned int *)(UART1_BASE_ADDR + 0x18))
#define U1SCR          (*(volatile unsigned int *)(UART1_BASE_ADDR + 0x1C))
#define U1ACR          (*(volatile unsigned int *)(UART1_BASE_ADDR + 0x20))
#define U1FDR          (*(volatile unsigned int *)(UART1_BASE_ADDR + 0x28))
#define U1TER          (*(volatile unsigned int *)(UART1_BASE_ADDR + 0x30))

// Universal Asynchronous Receiver Transmitter 2 (UART2) 
#define UART2_BASE_ADDR		0xE0078000
#define U2RBR          (*(volatile unsigned int *)(UART2_BASE_ADDR + 0x00))
#define U2THR          (*(volatile unsigned int *)(UART2_BASE_ADDR + 0x00))
#define U2DLL          (*(volatile unsigned int *)(UART2_BASE_ADDR + 0x00))
#define U2DLM          (*(volatile unsigned int *)(UART2_BASE_ADDR + 0x04))
#define U2IER          (*(volatile unsigned int *)(UART2_BASE_ADDR + 0x04))
#define U2IIR          (*(volatile unsigned int *)(UART2_BASE_ADDR + 0x08))
#define U2FCR          (*(volatile unsigned int *)(UART2_BASE_ADDR + 0x08))
#define U2LCR          (*(volatile unsigned int *)(UART2_BASE_ADDR + 0x0C))
#define U2LSR          (*(volatile unsigned int *)(UART2_BASE_ADDR + 0x14))
#define U2SCR          (*(volatile unsigned int *)(UART2_BASE_ADDR + 0x1C))
#define U2ACR          (*(volatile unsigned int *)(UART2_BASE_ADDR + 0x20))
#define U2ICR          (*(volatile unsigned int *)(UART2_BASE_ADDR + 0x24))
#define U2FDR          (*(volatile unsigned int *)(UART2_BASE_ADDR + 0x28))
#define U2TER          (*(volatile unsigned int *)(UART2_BASE_ADDR + 0x30))

// Universal Asynchronous Receiver Transmitter 3 (UART3) 
#define UART3_BASE_ADDR		0xE007C000
#define U3RBR          (*(volatile unsigned int *)(UART3_BASE_ADDR + 0x00))
#define U3THR          (*(volatile unsigned int *)(UART3_BASE_ADDR + 0x00))
#define U3DLL          (*(volatile unsigned int *)(UART3_BASE_ADDR + 0x00))
#define U3DLM          (*(volatile unsigned int *)(UART3_BASE_ADDR + 0x04))
#define U3IER          (*(volatile unsigned int *)(UART3_BASE_ADDR + 0x04))
#define U3IIR          (*(volatile unsigned int *)(UART3_BASE_ADDR + 0x08))
#define U3FCR          (*(volatile unsigned int *)(UART3_BASE_ADDR + 0x08))
#define U3LCR          (*(volatile unsigned int *)(UART3_BASE_ADDR + 0x0C))
#define U3LSR          (*(volatile unsigned int *)(UART3_BASE_ADDR + 0x14))
#define U3SCR          (*(volatile unsigned int *)(UART3_BASE_ADDR + 0x1C))
#define U3ACR          (*(volatile unsigned int *)(UART3_BASE_ADDR + 0x20))
#define U3ICR          (*(volatile unsigned int *)(UART3_BASE_ADDR + 0x24))
  #define UART_IrDAEN                    0x01                                            // IrDA Mode enable
  #define UART_IrDAINV                   0x02                                            // IrDA serial input inverted
  #define UART_FIX_PULSE_ENABLE          0x04                                            // IrDA Fixed Pulse Width Mode enable
  #define UART_PULSE_WIDTH_2_TPCLK       0x00                                            // IrDA pulse width fixed at 2 x Tpclk
  #define UART_PULSE_WIDTH_4_TPCLK       0x08                                            // IrDA pulse width fixed at 4 x Tpclk
  #define UART_PULSE_WIDTH_8_TPCLK       0x10                                            // IrDA pulse width fixed at 8 x Tpclk
  #define UART_PULSE_WIDTH_16_TPCLK      0x18                                            // IrDA pulse width fixed at 16 x Tpclk
  #define UART_PULSE_WIDTH_32_TPCLK      0x20                                            // IrDA pulse width fixed at 32 x Tpclk
  #define UART_PULSE_WIDTH_64_TPCLK      0x28                                            // IrDA pulse width fixed at 64 x Tpclk
  #define UART_PULSE_WIDTH_128_TPCLK     0x30                                            // IrDA pulse width fixed at 128 x Tpclk
  #define UART_PULSE_WIDTH_256_TPCLK     0x38                                            // IrDA pulse width fixed at 256 x Tpclk
#define U3FDR          (*(volatile unsigned int *)(UART3_BASE_ADDR + 0x28))
#define U3TER          (*(volatile unsigned int *)(UART3_BASE_ADDR + 0x30))

// I2C Interface 0 
#define I2C0_BASE_ADDR		0xE001C000
#define I20CONSET      (*(volatile unsigned int *)(I2C0_BASE_ADDR + 0x00))
#define I20STAT        (*(volatile unsigned int *)(I2C0_BASE_ADDR + 0x04))
#define I20DAT         (*(volatile unsigned int *)(I2C0_BASE_ADDR + 0x08))
#define I20ADR         (*(volatile unsigned int *)(I2C0_BASE_ADDR + 0x0C))
#define I20SCLH        (*(volatile unsigned int *)(I2C0_BASE_ADDR + 0x10))
#define I20SCLL        (*(volatile unsigned int *)(I2C0_BASE_ADDR + 0x14))
#define I20CONCLR      (*(volatile unsigned int *)(I2C0_BASE_ADDR + 0x18))

// I2C Interface 1 
#define I2C1_BASE_ADDR		0xE005C000
#define I21CONSET      (*(volatile unsigned int *)(I2C1_BASE_ADDR + 0x00))
#define I21STAT        (*(volatile unsigned int *)(I2C1_BASE_ADDR + 0x04))
#define I21DAT         (*(volatile unsigned int *)(I2C1_BASE_ADDR + 0x08))
#define I21ADR         (*(volatile unsigned int *)(I2C1_BASE_ADDR + 0x0C))
#define I21SCLH        (*(volatile unsigned int *)(I2C1_BASE_ADDR + 0x10))
#define I21SCLL        (*(volatile unsigned int *)(I2C1_BASE_ADDR + 0x14))
#define I21CONCLR      (*(volatile unsigned int *)(I2C1_BASE_ADDR + 0x18))

// I2C Interface 2 
#define I2C2_BASE_ADDR		0xE0080000
#define I22CONSET      (*(volatile unsigned int *)(I2C2_BASE_ADDR + 0x00))
#define I22STAT        (*(volatile unsigned int *)(I2C2_BASE_ADDR + 0x04))
#define I22DAT         (*(volatile unsigned int *)(I2C2_BASE_ADDR + 0x08))
#define I22ADR         (*(volatile unsigned int *)(I2C2_BASE_ADDR + 0x0C))
#define I22SCLH        (*(volatile unsigned int *)(I2C2_BASE_ADDR + 0x10))
#define I22SCLL        (*(volatile unsigned int *)(I2C2_BASE_ADDR + 0x14))
#define I22CONCLR      (*(volatile unsigned int *)(I2C2_BASE_ADDR + 0x18))

// SPI0 (Serial Peripheral Interface 0) 
#define SPI0_BASE_ADDR		0xE0020000
#define S0SPCR         (*(volatile unsigned int *)(SPI0_BASE_ADDR + 0x00))
#define S0SPSR         (*(volatile unsigned int *)(SPI0_BASE_ADDR + 0x04))
#define S0SPDR         (*(volatile unsigned int *)(SPI0_BASE_ADDR + 0x08))
#define S0SPCCR        (*(volatile unsigned int *)(SPI0_BASE_ADDR + 0x0C))
#define S0SPINT        (*(volatile unsigned int *)(SPI0_BASE_ADDR + 0x1C))

// SSP0 Controller 
#define SSP0_BASE_ADDR		0xE0068000
#define SSP0CR0        (*(volatile unsigned int *)(SSP0_BASE_ADDR + 0x00))
#define SSP0CR1        (*(volatile unsigned int *)(SSP0_BASE_ADDR + 0x04))
#define SSP0DR         (*(volatile unsigned int *)(SSP0_BASE_ADDR + 0x08))
#define SSP0SR         (*(volatile unsigned int *)(SSP0_BASE_ADDR + 0x0C))
#define SSP0CPSR       (*(volatile unsigned int *)(SSP0_BASE_ADDR + 0x10))
#define SSP0IMSC       (*(volatile unsigned int *)(SSP0_BASE_ADDR + 0x14))
#define SSP0RIS        (*(volatile unsigned int *)(SSP0_BASE_ADDR + 0x18))
#define SSP0MIS        (*(volatile unsigned int *)(SSP0_BASE_ADDR + 0x1C))
#define SSP0ICR        (*(volatile unsigned int *)(SSP0_BASE_ADDR + 0x20))
#define SSP0DMACR      (*(volatile unsigned int *)(SSP0_BASE_ADDR + 0x24))

// SSP1 Controller 
#define SSP1_BASE_ADDR		0xE0030000
#define SSP1CR0        (*(volatile unsigned int *)(SSP1_BASE_ADDR + 0x00))
#define SSP1CR1        (*(volatile unsigned int *)(SSP1_BASE_ADDR + 0x04))
#define SSP1DR         (*(volatile unsigned int *)(SSP1_BASE_ADDR + 0x08))
#define SSP1SR         (*(volatile unsigned int *)(SSP1_BASE_ADDR + 0x0C))
#define SSP1CPSR       (*(volatile unsigned int *)(SSP1_BASE_ADDR + 0x10))
#define SSP1IMSC       (*(volatile unsigned int *)(SSP1_BASE_ADDR + 0x14))
#define SSP1RIS        (*(volatile unsigned int *)(SSP1_BASE_ADDR + 0x18))
#define SSP1MIS        (*(volatile unsigned int *)(SSP1_BASE_ADDR + 0x1C))
#define SSP1ICR        (*(volatile unsigned int *)(SSP1_BASE_ADDR + 0x20))
#define SSP1DMACR      (*(volatile unsigned int *)(SSP1_BASE_ADDR + 0x24))


// Real Time Clock 
#define RTC_BASE_ADDR		0xE0024000
#define RTC_ILR         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x00))
#define RTC_CTC         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x04))
#define RTC_CCR         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x08))
#define RTC_CIIR        (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x0C))
#define RTC_AMR         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x10))
#define RTC_CTIME0      (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x14))
#define RTC_CTIME1      (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x18))
#define RTC_CTIME2      (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x1C))
#define RTC_SEC         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x20))
#define RTC_MIN         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x24))
#define RTC_HOUR        (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x28))
#define RTC_DOM         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x2C))
#define RTC_DOW         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x30))
#define RTC_DOY         (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x34))
#define RTC_MONTH       (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x38))
#define RTC_YEAR        (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x3C))
#define RTC_CISS        (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x40))
#define RTC_ALSEC       (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x60))
#define RTC_ALMIN       (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x64))
#define RTC_ALHOUR      (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x68))
#define RTC_ALDOM       (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x6C))
#define RTC_ALDOW       (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x70))
#define RTC_ALDOY       (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x74))
#define RTC_ALMON       (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x78))
#define RTC_ALYEAR      (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x7C))
#define RTC_PREINT      (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x80))
#define RTC_PREFRAC     (*(volatile unsigned int *)(RTC_BASE_ADDR + 0x84))


// A/D Converter 0 (AD0) 
#define AD0_BASE_ADDR		0xE0034000
#define AD0CR          (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x00))
#define AD0GDR         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x04))
#define AD0INTEN       (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x0C))
#define AD0DR0         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x10))
#define AD0DR1         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x14))
#define AD0DR2         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x18))
#define AD0DR3         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x1C))
#define AD0DR4         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x20))
#define AD0DR5         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x24))
#define AD0DR6         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x28))
#define AD0DR7         (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x2C))
#define AD0STAT        (*(volatile unsigned int *)(AD0_BASE_ADDR + 0x30))


// D/A Converter 
#define DAC_BASE_ADDR		0xE006C000
#define DACR           (*(volatile unsigned int *)(DAC_BASE_ADDR + 0x00))


// Watchdog 
#define WDG_BASE_ADDR		0xE0000000
#define WDMOD          (*(volatile unsigned int *)(WDG_BASE_ADDR + 0x00))
  #define WDEN                           0x00000001                               // Watchdog enable
  #define WDRESET                        0x00000002                               // Watchdog reset enable
  #define WDTOF                          0x00000004                               // Watchdog timeout flag
  #define WDINT                          0x00000008                               // Watchdog Interrupt flag (read only)
#define WDTC           (*(volatile unsigned int *)(WDG_BASE_ADDR + 0x04))
#define WDFEED         (*(volatile unsigned int *)(WDG_BASE_ADDR + 0x08))
#define WDTV           (*(volatile unsigned int *)(WDG_BASE_ADDR + 0x0C))
#define WDCLKSEL       (*(volatile unsigned int *)(WDG_BASE_ADDR + 0x10))
  #define WD_CLOCK_INT_RC                0x00000000                               // clock watchdog from internal RC clock
  #define WD_CLOCK_PCLK                  0x00000001                               // clock watchdog from peripheral clock
  #define WD_CLOCK_RTC                   0x00000002                               // clock watchdog from RTC clock

// CAN CONTROLLERS AND ACCEPTANCE FILTER 
#define CAN_ACCEPT_BASE_ADDR		0xE003C000
#define CAN_AFMR		(*(volatile unsigned int *)(CAN_ACCEPT_BASE_ADDR + 0x00))  	
#define CAN_SFF_SA 		(*(volatile unsigned int *)(CAN_ACCEPT_BASE_ADDR + 0x04))  	
#define CAN_SFF_GRP_SA 	(*(volatile unsigned int *)(CAN_ACCEPT_BASE_ADDR + 0x08))
#define CAN_EFF_SA 		(*(volatile unsigned int *)(CAN_ACCEPT_BASE_ADDR + 0x0C))
#define CAN_EFF_GRP_SA 	(*(volatile unsigned int *)(CAN_ACCEPT_BASE_ADDR + 0x10))  	
#define CAN_EOT 		(*(volatile unsigned int *)(CAN_ACCEPT_BASE_ADDR + 0x14))
#define CAN_LUT_ERR_ADR (*(volatile unsigned int *)(CAN_ACCEPT_BASE_ADDR + 0x18))  	
#define CAN_LUT_ERR 	(*(volatile unsigned int *)(CAN_ACCEPT_BASE_ADDR + 0x1C))

#define CAN_CENTRAL_BASE_ADDR		0xE0040000  	
#define CAN_TX_SR 	(*(volatile unsigned int *)(CAN_CENTRAL_BASE_ADDR + 0x00))  	
#define CAN_RX_SR 	(*(volatile unsigned int *)(CAN_CENTRAL_BASE_ADDR + 0x04))  	
#define CAN_MSR 	(*(volatile unsigned int *)(CAN_CENTRAL_BASE_ADDR + 0x08))

#define CAN1_BASE_ADDR		0xE0044000
#define CAN1MOD 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x00))  	
#define CAN1CMR 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x04))  	
#define CAN1GSR 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x08))  	
#define CAN1ICR 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x0C))  	
#define CAN1IER 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x10))
#define CAN1BTR 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x14))  	
#define CAN1EWL 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x18))  	
#define CAN1SR 		(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x1C))  	
#define CAN1RFS 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x20))  	
#define CAN1RID 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x24))
#define CAN1RDA 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x28))  	
#define CAN1RDB 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x2C))
  	
#define CAN1TFI1 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x30))  	
#define CAN1TID1 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x34))  	
#define CAN1TDA1 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x38))
#define CAN1TDB1 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x3C))  	
#define CAN1TFI2 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x40))  	
#define CAN1TID2 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x44))  	
#define CAN1TDA2 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x48))  	
#define CAN1TDB2 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x4C))
#define CAN1TFI3 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x50))  	
#define CAN1TID3 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x54))  	
#define CAN1TDA3 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x58))  	
#define CAN1TDB3 	(*(volatile unsigned int *)(CAN1_BASE_ADDR + 0x5C))

#define CAN2_BASE_ADDR		0xE0048000
#define CAN2MOD 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x00))  	
#define CAN2CMR 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x04))  	
#define CAN2GSR 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x08))  	
#define CAN2ICR 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x0C))  	
#define CAN2IER 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x10))
#define CAN2BTR 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x14))  	
#define CAN2EWL 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x18))  	
#define CAN2SR 		(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x1C))  	
#define CAN2RFS 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x20))  	
#define CAN2RID 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x24))
#define CAN2RDA 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x28))  	
#define CAN2RDB 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x2C))
  	
#define CAN2TFI1 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x30))  	
#define CAN2TID1 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x34))  	
#define CAN2TDA1 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x38))
#define CAN2TDB1 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x3C))  	
#define CAN2TFI2 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x40))  	
#define CAN2TID2 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x44))  	
#define CAN2TDA2 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x48))  	
#define CAN2TDB2 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x4C))
#define CAN2TFI3 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x50))  	
#define CAN2TID3 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x54))  	
#define CAN2TDA3 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x58))  	
#define CAN2TDB3 	(*(volatile unsigned int *)(CAN2_BASE_ADDR + 0x5C))


/* MultiMedia Card Interface(MCI) Controller */
#define MCI_BASE_ADDR		0xE008C000
#define MCI_POWER      (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x00))
#define MCI_CLOCK      (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x04))
#define MCI_ARGUMENT   (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x08))
#define MCI_COMMAND    (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x0C))
#define MCI_RESP_CMD   (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x10))
#define MCI_RESP0      (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x14))
#define MCI_RESP1      (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x18))
#define MCI_RESP2      (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x1C))
#define MCI_RESP3      (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x20))
#define MCI_DATA_TMR   (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x24))
#define MCI_DATA_LEN   (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x28))
#define MCI_DATA_CTRL  (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x2C))
#define MCI_DATA_CNT   (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x30))
#define MCI_STATUS     (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x34))
#define MCI_CLEAR      (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x38))
#define MCI_MASK0      (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x3C))
#define MCI_MASK1      (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x40))
#define MCI_FIFO_CNT   (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x48))
#define MCI_FIFO       (*(volatile unsigned int *)(MCI_BASE_ADDR + 0x80))


/* I2S Interface Controller (I2S) */
#define I2S_BASE_ADDR		0xE0088000
#define I2S_DAO        (*(volatile unsigned int *)(I2S_BASE_ADDR + 0x00))
#define I2S_DAI        (*(volatile unsigned int *)(I2S_BASE_ADDR + 0x04))
#define I2S_TX_FIFO    (*(volatile unsigned int *)(I2S_BASE_ADDR + 0x08))
#define I2S_RX_FIFO    (*(volatile unsigned int *)(I2S_BASE_ADDR + 0x0C))
#define I2S_STATE      (*(volatile unsigned int *)(I2S_BASE_ADDR + 0x10))
#define I2S_DMA1       (*(volatile unsigned int *)(I2S_BASE_ADDR + 0x14))
#define I2S_DMA2       (*(volatile unsigned int *)(I2S_BASE_ADDR + 0x18))
#define I2S_IRQ        (*(volatile unsigned int *)(I2S_BASE_ADDR + 0x1C))
#define I2S_TXRATE     (*(volatile unsigned int *)(I2S_BASE_ADDR + 0x20))
#define I2S_RXRATE     (*(volatile unsigned int *)(I2S_BASE_ADDR + 0x24))


/* General-purpose DMA Controller */
#define DMA_BASE_ADDR		0xFFE04000
#define GPDMA_INT_STAT         (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x000))
#define GPDMA_INT_TCSTAT       (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x004))
#define GPDMA_INT_TCCLR        (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x008))
#define GPDMA_INT_ERR_STAT     (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x00C))
#define GPDMA_INT_ERR_CLR      (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x010))
#define GPDMA_RAW_INT_TCSTAT   (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x014))
#define GPDMA_RAW_INT_ERR_STAT (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x018))
#define GPDMA_ENABLED_CHNS     (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x01C))
#define GPDMA_SOFT_BREQ        (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x020))
#define GPDMA_SOFT_SREQ        (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x024))
#define GPDMA_SOFT_LBREQ       (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x028))
#define GPDMA_SOFT_LSREQ       (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x02C))
#define GPDMA_CONFIG           (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x030))
#define GPDMA_SYNC             (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x034))

/* DMA channel 0 registers */
#define GPDMA_CH0_SRC      (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x100))
#define GPDMA_CH0_DEST     (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x104))
#define GPDMA_CH0_LLI      (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x108))
#define GPDMA_CH0_CTRL     (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x10C))
#define GPDMA_CH0_CFG      (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x110))

/* DMA channel 1 registers */
#define GPDMA_CH1_SRC      (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x120))
#define GPDMA_CH1_DEST     (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x124))
#define GPDMA_CH1_LLI      (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x128))
#define GPDMA_CH1_CTRL     (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x12C))
#define GPDMA_CH1_CFG      (*(volatile unsigned int *)(DMA_BASE_ADDR + 0x130))


/* USB Controller */
#define USB_INT_BASE_ADDR	0xE01FC1C0
#define USB_BASE_ADDR		0xFFE0C200		/* USB Base Address */

#define USB_INT_STAT    (*(volatile unsigned int *)(USB_INT_BASE_ADDR + 0x00))

/* USB Device Interrupt Registers */
#define DEV_INT_STAT    (*(volatile unsigned int *)(USB_BASE_ADDR + 0x00))
#define DEV_INT_EN      (*(volatile unsigned int *)(USB_BASE_ADDR + 0x04))
#define DEV_INT_CLR     (*(volatile unsigned int *)(USB_BASE_ADDR + 0x08))
#define DEV_INT_SET     (*(volatile unsigned int *)(USB_BASE_ADDR + 0x0C))
#define DEV_INT_PRIO    (*(volatile unsigned int *)(USB_BASE_ADDR + 0x2C))

/* USB Device Endpoint Interrupt Registers */
#define EP_INT_STAT     (*(volatile unsigned int *)(USB_BASE_ADDR + 0x30))
#define EP_INT_EN       (*(volatile unsigned int *)(USB_BASE_ADDR + 0x34))
#define EP_INT_CLR      (*(volatile unsigned int *)(USB_BASE_ADDR + 0x38))
#define EP_INT_SET      (*(volatile unsigned int *)(USB_BASE_ADDR + 0x3C))
#define EP_INT_PRIO     (*(volatile unsigned int *)(USB_BASE_ADDR + 0x40))

/* USB Device Endpoint Realization Registers */
#define REALIZE_EP      (*(volatile unsigned int *)(USB_BASE_ADDR + 0x44))
#define EP_INDEX        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x48))
#define MAXPACKET_SIZE  (*(volatile unsigned int *)(USB_BASE_ADDR + 0x4C))

/* USB Device Command Reagisters */
#define CMD_CODE        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x10))
#define CMD_DATA        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x14))

/* USB Device Data Transfer Registers */
#define RX_DATA         (*(volatile unsigned int *)(USB_BASE_ADDR + 0x18))
#define TX_DATA         (*(volatile unsigned int *)(USB_BASE_ADDR + 0x1C))
#define RX_PLENGTH      (*(volatile unsigned int *)(USB_BASE_ADDR + 0x20))
#define TX_PLENGTH      (*(volatile unsigned int *)(USB_BASE_ADDR + 0x24))
#define USB_CTRL        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x28))

/* USB Device DMA Registers */
#define DMA_REQ_STAT        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x50))
#define DMA_REQ_CLR         (*(volatile unsigned int *)(USB_BASE_ADDR + 0x54))
#define DMA_REQ_SET         (*(volatile unsigned int *)(USB_BASE_ADDR + 0x58))
#define UDCA_HEAD           (*(volatile unsigned int *)(USB_BASE_ADDR + 0x80))
#define EP_DMA_STAT         (*(volatile unsigned int *)(USB_BASE_ADDR + 0x84))
#define EP_DMA_EN           (*(volatile unsigned int *)(USB_BASE_ADDR + 0x88))
#define EP_DMA_DIS          (*(volatile unsigned int *)(USB_BASE_ADDR + 0x8C))
#define DMA_INT_STAT        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x90))
#define DMA_INT_EN          (*(volatile unsigned int *)(USB_BASE_ADDR + 0x94))
#define EOT_INT_STAT        (*(volatile unsigned int *)(USB_BASE_ADDR + 0xA0))
#define EOT_INT_CLR         (*(volatile unsigned int *)(USB_BASE_ADDR + 0xA4))
#define EOT_INT_SET         (*(volatile unsigned int *)(USB_BASE_ADDR + 0xA8))
#define NDD_REQ_INT_STAT    (*(volatile unsigned int *)(USB_BASE_ADDR + 0xAC))
#define NDD_REQ_INT_CLR     (*(volatile unsigned int *)(USB_BASE_ADDR + 0xB0))
#define NDD_REQ_INT_SET     (*(volatile unsigned int *)(USB_BASE_ADDR + 0xB4))
#define SYS_ERR_INT_STAT    (*(volatile unsigned int *)(USB_BASE_ADDR + 0xB8))
#define SYS_ERR_INT_CLR     (*(volatile unsigned int *)(USB_BASE_ADDR + 0xBC))
#define SYS_ERR_INT_SET     (*(volatile unsigned int *)(USB_BASE_ADDR + 0xC0))


/* USB Host Controller */
#define USBHC_BASE_ADDR		0xFFE0C000
#define HC_REVISION         (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x00))
#define HC_CONTROL          (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x04))
#define HC_CMD_STAT         (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x08))
#define HC_INT_STAT         (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x0C))
#define HC_INT_EN           (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x10))
#define HC_INT_DIS          (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x14))
#define HC_HCCA             (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x18))
#define HC_PERIOD_CUR_ED    (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x1C))
#define HC_CTRL_HEAD_ED     (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x20))
#define HC_CTRL_CUR_ED      (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x24))
#define HC_BULK_HEAD_ED     (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x28))
#define HC_BULK_CUR_ED      (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x2C))
#define HC_DONE_HEAD        (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x30))
#define HC_FM_INTERVAL      (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x34))
#define HC_FM_REMAINING     (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x38))
#define HC_FM_NUMBER        (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x3C))
#define HC_PERIOD_START     (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x40))
#define HC_LS_THRHLD        (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x44))
#define HC_RH_DESCA         (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x48))
#define HC_RH_DESCB         (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x4C))
#define HC_RH_STAT          (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x50))
#define HC_RH_PORT_STAT1    (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x54))
#define HC_RH_PORT_STAT2    (*(volatile unsigned int *)(USBHC_BASE_ADDR + 0x58))

/* USB OTG Controller */
#define USBOTG_BASE_ADDR	0xFFE0C100
#define OTG_INT_STAT        (*(volatile unsigned int *)(USBOTG_BASE_ADDR + 0x00))
#define OTG_INT_EN          (*(volatile unsigned int *)(USBOTG_BASE_ADDR + 0x04))
#define OTG_INT_SET         (*(volatile unsigned int *)(USBOTG_BASE_ADDR + 0x08))
#define OTG_INT_CLR         (*(volatile unsigned int *)(USBOTG_BASE_ADDR + 0x0C))
#define OTG_STAT_CTRL       (*(volatile unsigned int *)(USBOTG_BASE_ADDR + 0x10))
#define OTG_TIMER           (*(volatile unsigned int *)(USBOTG_BASE_ADDR + 0x14))

#define USBOTG_I2C_BASE_ADDR	0xFFE0C300
#define OTG_I2C_RX          (*(volatile unsigned int *)(USBOTG_I2C_BASE_ADDR + 0x00))
#define OTG_I2C_TX          (*(volatile unsigned int *)(USBOTG_I2C_BASE_ADDR + 0x00))
#define OTG_I2C_STS         (*(volatile unsigned int *)(USBOTG_I2C_BASE_ADDR + 0x04))
#define OTG_I2C_CTL         (*(volatile unsigned int *)(USBOTG_I2C_BASE_ADDR + 0x08))
#define OTG_I2C_CLKHI       (*(volatile unsigned int *)(USBOTG_I2C_BASE_ADDR + 0x0C))
#define OTG_I2C_CLKLO       (*(volatile unsigned int *)(USBOTG_I2C_BASE_ADDR + 0x10))

#define USBOTG_CLK_BASE_ADDR	0xFFE0CFF0
#define OTG_CLK_CTRL        (*(volatile unsigned int *)(USBOTG_CLK_BASE_ADDR + 0x04))
#define OTG_CLK_STAT        (*(volatile unsigned int *)(USBOTG_CLK_BASE_ADDR + 0x08))


// Ethernet MAC (32 bit data bus) -- all registers are RW unless indicated in parentheses 
#define MAC_BASE_ADDR		0xFFE00000 // AHB Peripheral # 0 
#define MAC_MAC1            (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x000)) // MAC config reg 1 
  #define RECEIVE_ENABLE                 0x00000001
  #define PASS_ALL_RECEIVE_FRAMES        0x00000002
  #define MAC1_RX_FLOW_CONTROL           0x00000004
  #define TX_FLOW_CONTROL                0x00000008
  #define MAC1_LOOPBACK                  0x00000010
  #define RESET_TX                       0x00000100
  #define RESET_MCS_TX                   0x00000200
  #define RESET_RX                       0x00000400
  #define RESET_MCS_RX                   0x00000800
  #define SIMULATION_RESET               0x00002000
  #define SOFT_RESET                     0x00008000
#define MAC_MAC2            (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x004)) // MAC config reg 2 
  #define MAC2_FULL_DUPLEX               0x00000001
  #define FRAME_LENGTH_CHECKING          0x00000002
  #define HUGE_FRAME_ENABLED             0x00000004
  #define DELAYED_CRC                    0x00000008
  #define CRC_ENABLE                     0x00000010
  #define PAD_CRC_ENABLE                 0x00000020
  #define VLAN_PAD_ENABLE                0x00000040
  #define AUTO_DETECT_PAD_ENABLE         0x00000080
  #define PURE_PREAMBLE_ENFORCEMENT      0x00000100
  #define LONG_PREAMBLE_ENFORCEMENT      0x00000200
  #define NO_BACKOFF                     0x00001000
  #define BACK_PRESSURE_NO_BACKOFF       0x00002000
  #define EXCESS_DEFER                   0x00004000
#define MAC_IPGT            (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x008)) /* b2b InterPacketGap reg */
  #define HALF_DUPLEX_IPG_TIME           0x00000012
  #define FULL_DUPLEX_IPG_TIME           0x00000015
#define MAC_IPGR            (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x00C)) /* non b2b InterPacketGap reg */
  #define IPGR_TIME                      0x00000012
#define MAC_CLRT            (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x010)) /* CoLlision window/ReTry reg */
  #define DEFAULT_CLRT                   0x370f                                                  // recommended default
#define MAC_MAXF            (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x014)) /* MAXimum Frame reg */
#define MAC_SUPP            (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x018)) /* PHY SUPPort reg */
  #define SUPP_SPEED                     0x00000100
  #define RESET_RMII                     0x00000800
#define MAC_TEST            (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x01C)) /* TEST reg */
#define MAC_MCFG            (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x020)) /* MII Mgmt ConFiG reg */
  #define SCAN_INCREMENT                 0x00000001
  #define SUPPRESS_PREAMBLE              0x00000002
  #define HOST_CLK_DIV_4                 0x00000000
  #define HOST_CLK_DIV_6                 0x00000008
  #define HOST_CLK_DIV_8                 0x0000000c
  #define HOST_CLK_DIV_10                0x00000010
  #define HOST_CLK_DIV_14                0x00000014
  #define HOST_CLK_DIV_20                0x00000018
  #define HOST_CLK_DIV_28                0x0000001c
  #define RESET_MMI_MGMT                 0x00008000
#define MAC_MCMD            (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x024)) /* MII Mgmt CoMmanD reg */
  #define MCMD_READ                      0x00000001
  #define MCMD_SCAN                      0x00000002
#define MAC_MADR            (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x028)) /* MII Mgmt ADdRess reg */
#define MAC_MWTD            (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x02C)) /* MII Mgmt WriTe Data reg (WO) */
#define MAC_MRDD            (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x030)) /* MII Mgmt ReaD Data reg (RO) */
#define MAC_MIND            (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x034)) /* MII Mgmt INDicators reg (RO) */
  #define MIND_BUSY                      0x00000001
  #define MIND_SCANNING                  0x00000002
  #define MIND_NOT_VALID                 0x00000004
  #define MIND_MII_LINK_FAIL             0x00000008

#define MAC_SA0             (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x040)) /* Station Address 0 reg */
#define MAC_SA1             (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x044)) /* Station Address 1 reg */
#define MAC_SA2             (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x048)) /* Station Address 2 reg */

#define MAC_COMMAND         (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x100)) /* Command reg */
  #define EMAC_RX_ENABLE                 0x00000001
  #define EMAC_TX_ENABLE                 0x00000002
  #define EMAC_REG_RESET                 0x00000008
  #define EMAC_TX_RESET                  0x00000010                      // set only (reads back 0)
  #define EMAC_RX_RESET                  0x00000020                      // set only (reads back 0)
  #define EMAC_PASS_RUNT_FRAME           0x00000040
  #define EMAC_PASS_RC_FILTER            0x00000080
  #define EMAC_TX_FLOW_CONTROL           0x00000100
  #define EMAC_RMII                      0x00000200
  #define EMAC_FULL_DUPLEX               0x00000400
#define MAC_STATUS          (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x104)) /* Status reg (RO) */
  #define EMAC_RX_STATUS                 0x00000001
  #define EMAC_TX_STATUS                 0x00000002
#define MAC_RXDESCRIPTOR    (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x108)) /* Rx descriptor base address reg */
#define MAC_RXSTATUS        (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x10C)) /* Rx status base address reg */
#define MAC_RXDESCRIPTORNUM (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x110)) /* Rx number of descriptors reg */
#define MAC_RXPRODUCEINDEX  (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x114)) /* Rx produce index reg (RO) */
#define MAC_RXCONSUMEINDEX  (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x118)) /* Rx consume index reg */
#define MAC_TXDESCRIPTOR    (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x11C)) /* Tx descriptor base address reg */
#define MAC_TXSTATUS        (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x120)) /* Tx status base address reg */
#define MAC_TXDESCRIPTORNUM (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x124)) /* Tx number of descriptors reg */
#define MAC_TXPRODUCEINDEX  (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x128)) /* Tx produce index reg */
#define MAC_TXCONSUMEINDEX  (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x12C)) /* Tx consume index reg (RO) */

#define MAC_TSV0            (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x158)) /* Tx status vector 0 reg (RO) */
#define MAC_TSV1            (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x15C)) /* Tx status vector 1 reg (RO) */
#define MAC_RSV             (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x160)) /* Rx status vector reg (RO) */

#define MAC_FLOWCONTROLCNT  (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x170)) /* Flow control counter reg */
#define MAC_FLOWCONTROLSTS  (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x174)) /* Flow control status reg */

#define MAC_RXFILTERCTRL    (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x200)) /* Rx filter ctrl reg */
  #define ENABLE_ACCEPT_UNICAST          0x00000001
  #define ENABLE_ACCEPT_BROADCAST        0x00000002
  #define ENABLE_ACCEPT_MULTICAST        0x00000004
  #define ENABLE_ACCEPT_UNICAST_HASH     0x00000008
  #define ENABLE_ACCEPT_MULTICAST_HASH   0x00000010
  #define ENABLE_ACCEPT_PERFECT          0x00000020
  #define ENABLE_ACCEPT_MAGIC_WAKEUP     0x00001000
  #define ENABLE_ACCEPT_WAKEUP           0x00002000
#define MAC_RXFILTERWOLSTS  (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x204)) /* Rx filter WoL status reg (RO) */
#define MAC_RXFILTERWOLCLR  (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x208)) /* Rx filter WoL clear reg (WO) */

#define MAC_HASHFILTERL     (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x210)) /* Hash filter LSBs reg */
#define MAC_HASHFILTERH     (*(volatile unsigned int *)(MAC_BASE_ADDR + 0x214)) /* Hash filter MSBs reg */

#define MAC_INTSTATUS       (*(volatile unsigned int *)(MAC_BASE_ADDR + 0xFE0)) /* Interrupt status reg (RO) */
  #define EMAC_RxOverrunInt              0x00000001
  #define EMAC_RxErrorInt                0x00000002
  #define EMAC_RxFinishedInt             0x00000004
  #define EMAC_RxDoneInt                 0x00000008
  #define EMAC_TxUnderrunInt             0x00000010
  #define EMAC_TxErrorInt                0x00000020
  #define EMAC_TxFinishedInt             0x00000040
  #define EMAC_TxDoneInt                 0x00000080
  #define EMAC_SoftInt                   0x00000100
  #define EMAC_WakeupInt                 0x00000200
#define MAC_INTENABLE       (*(volatile unsigned int *)(MAC_BASE_ADDR + 0xFE4)) /* Interrupt enable reg  */
#define MAC_INTCLEAR        (*(volatile unsigned int *)(MAC_BASE_ADDR + 0xFE8)) /* Interrupt clear reg (WO) */
#define MAC_INTSET          (*(volatile unsigned int *)(MAC_BASE_ADDR + 0xFEC)) /* Interrupt set reg (WO) */

#define MAC_POWERDOWN       (*(volatile unsigned int *)(MAC_BASE_ADDR + 0xFF4)) /* Power-down reg */
#define MAC_MODULEID        (*(volatile unsigned int *)(MAC_BASE_ADDR + 0xFFC)) /* Module ID reg (RO) */
    #define OLD_MAC_MODULE_ID            ((0x3902 << 16) | 0x2000)                               // first version MAC ID

typedef struct stLPC23XX_RX_BD
{
volatile unsigned char   *ptrBlock;
volatile unsigned long    bd_details;
} LPC23XX_RX_BD;

#define RX_LAN_SIZE_MASK                  0x000007ff
#define RX_LAN_INTERRUPT_ON_DONE          0x80000000
#define TX_OVERRIDE                       0x04000000
#define TX_HUGE_FRAME                     0x08000000
#define TX_PAD_FRAME                      0x10000000
#define TX_APPEND_CRC                     0x20000000
#define TX_LAST_FRAG                      0x40000000
#define TX_LAN_INTERRUPT_ON_DONE          0x80000000

typedef struct stLPC23XX_RX_STATUS
{
volatile unsigned long    ulStatusInfo;
volatile unsigned long    ulStatusHASH_CRC;
} LPC23XX_RX_STATUS;

#define CONTROL_FRAME                     0x00040000
#define VLAN_FRAME                        0x00080000
#define FAIL_FILTER                       0x00100000
#define MULTICAST_FRAME                   0x00200000
#define BROADCAST_FRAME                   0x00400000
#define CRC_ERROR                         0x00800000
#define SYMBOL_ERROR                      0x01000000
#define LENGTH_ERROR                      0x02000000
#define RANGE_ERROR                       0x04000000
#define ALIGNMENT_ERROR                   0x08000000
#define RX_LAN_OVERRUN                    0x10000000
#define RX_NO_DESCRIPTOR                  0x20000000
#define LAST_FRAG                         0x40000000
#define RX_FRAME_ERROR                    0x80000000

typedef struct stLPC23XX_TX_STATUS
{
volatile unsigned long    ulStatusInfo;
} LPC23XX_TX_STATUS;

#define COLLISION_COUNT_SHIFT             21
#define COLLISION_COUNT_MASK              0x01e00000
#define DEFER                             0x02000000
#define EXCESSIVE_DEFER                   0x04000000
#define EXCESSIVE_COLLISION               0x08000000
#define LATE_COLLISION                    0x10000000
#define TX_LAN_OVERRUN                    0x20000000
#define TX_NO_DESCRIPTOR                  0x40000000
#define TX_FRAME_ERROR                    0x80000000

#endif  // __LPC23xx_H 

