/*****************************************************************************
 *
 * Project          : 
 * Subproject       : 
 * Name             : lpc2364_rom.ld
 * Function         : Linker script
 * Designer         : 
 * Creation date    : 
 * Compiler         : GNU ARM
 * Processor        : LPC2364
 * Last update      :
 * Last updated by  :
 * History          :
 *
 ********************************************************************
 *
 *  Flash
 *   startup
 *    .startup   Vector table and startup code
 *   prog
 *    .text
 *    .rodata
 *    .rodata*
 *    .glue_7
 *    .glue_7t
 *               __end_of_text_
 *     .         __data_beg_src__
 *     .data
 *
 *  Ram
 *   .data
 *     __data_beg__
 *     .data
 *     __data_end__
 *   .bss
 *     __bss_beg__
 *     .bss
 *     __bss_end__
 *     _bss_end__
 *     __end__
 *     _end
 *     end
 *
 *  Ramstack
 *   stacklayout
 *    __stack_scv_bottom__
 *    __stack_irq_bottom__  __stack_scv_top__  
 *    __stack_fiq_bottom__  __stack_irq_top__
 *    __stack_und_bottom__  __stack_fiq_top__
 *    __stack_iap_bottom__  __stack_und_top__
 *
 *  USBRam
 *   usbramlayout
 *    __usbram_start__
 *    __usbram_size__
 *
 *  EthRam
 *   ethramlayout
 *    __ethram_start__
 *    __ethram_size__
 *
 ********************************************************************
 *
 * ------------------------------------ 0xFFFF FFFF
 * |                                  |
 * |  AHB Peripherals                 |
 * |                                  |
 * |----------------------------------| 0xF000 0000
 * |                                  |
 * |  APB Peripherals                 |
 * |                                  |
 * |----------------------------------| 0xE000 0000
 * |  [Reserved]                      |
 * |----------------------------------| 0x8000 0000
 * |                                  |
 * |  Boot block                      |
 * |                                  |
 * |----------------------------------|
 * |  [Reserved]                      |
 * |----------------------------------| 0x7FE0 4000
 * |  Ethernet RAM                    |   16384 bytes (0x4000)
 * |----------------------------------| 0x7FE0 0000
 * |  [Reserved]                      |
 * |----------------------------------| 0x7FD0 2000
 * |  USB RAM                         |    8192 bytes (0x2000)
 * |----------------------------------| 0x7FD0 0000
 * |  [Reserved]                      |
 * |----------------------------------| 0x4000 8000
 * |            | Reserved for IAP    |     256 bytes (0x100)
 * |            |---------------------| 0x4000 7F00
 * |            | UND Stack           |     256 bytes (0x100)
 * |  Internal  |---------------------| 0x4000 7E00
 * |  RAM       | FIQ Stack           |     256 bytes (0x100)
 * |            |---------------------| 0x4000 7D00
 * |            | IRQ Stack           |     256 bytes (0x100)
 * |            |---------------------| 0x4000 7C00
 * |            | SVC Stack           |    1024 bytes (0x400)
 * |            |---------------------| 0x4000 7800
 * |            | Variables RW and ZI |   30720 bytes (0x7800)
 * |----------------------------------| 0x4000 0000
 * |  [Reserved]                      |
 * |----------------------------------| 0x0008 0000
 * |                                  |
 * | Flash                            |    512K
 * |                                  |
 * |                                  |
 * ------------------------------------ 0x00000000
 *  UND Stack is used by HaltUndef, HaltSWI, HaltPabort and HaldAabort
 *
 ********************************************************************/

/*
 * Defines memory layout
 */

/*
MEMORY
{
  flash    (rx) : ORIGIN = 0x00000000, LENGTH = 128K
  ram      (rw) : ORIGIN = 0x40000000, LENGTH = 8192
  ramstack (rw) : ORIGIN = 0x40007800, LENGTH = 2048
  usbram   (rw) : ORIGIN = 0x7FD00000, LENGTH = 8K
  ethram   (rw) : ORIGIN = 0x7FE00000, LENGTH = 16K
}
*/

/*
MEMORY
{
  flash    (rx) : ORIGIN = 0x00000000, LENGTH = 128K
  ram      (rw) : ORIGIN = 0x40000000, LENGTH = 4096
  ramstack (rw) : ORIGIN = 0x40001800, LENGTH = 2048
  usbram   (rw) : ORIGIN = 0x7FD00000, LENGTH = 8K
  ethram   (rw) : ORIGIN = 0x7FE00000, LENGTH = 16K
}
*/


MEMORY
{
  flash    (rx) : ORIGIN = 0x00000000, LENGTH = 128K
  ram      (rw) : ORIGIN = 0x40000000, LENGTH = 8192
  ramstack (rw) : ORIGIN = 0x7FE00000, LENGTH = 2048
  usbram   (rw) : ORIGIN = 0x7FD00000, LENGTH = 8K
}


/* 
 * Define entry point, found in startarm.s
 */
ENTRY(_reset_handler)

/*
 * Sections
 */
SECTIONS
{
  /* Place memory counter at address 0 */
  . = 0;

  /* Place startup code at start of flash */
  startup : 
  { 
    *(.startup)
  } > flash =0

  . = ALIGN(4);

  /* Place program code */
  prog :
  {
    *(.text)
    *(.rodata)
    *(.rodata*)
    *(glue_7)
    *(glue_7t)
  } > flash =0
  
  . = ALIGN(4);

  __end_of_text__  = .;
  __data_beg_src__ = .; 
  PROVIDE(etext = .);

  /* Place initialized data */
  .data : AT(__data_beg_src__)
  {
    __data_beg__     = .;
    *(.data)
  } > ram

  . = ALIGN(4);
  __data_end__     = .;
  PROVIDE(edata = .);

  /* Place zero initialized data */
  .bss :
  {
    __bss_beg__ = .;
    *(.bss)
  } >ram


  /* Align here to ensure that the .bss section occupies space up to
     _end. Align after .bss to ensure correct alignment even if the
     .bss section disappears because there are no input sections. */
  . = ALIGN(32 / 8);

  . = ALIGN(32 / 8);

  /* Provide end markers */
  _end        = .;
  _bss_end__  = .;
  __bss_end__ = .;
  __end__     = .;
  PROVIDE(end = .);

  stacklayout :
  {
    __stack_scv_bottom__ = .;
    . = . + 0x0400;
    __stack_scv_top__    = .;
    __stack_irq_bottom__ = .;
    . = . + 0x0100;
    __stack_irq_top__    = .;
    __stack_fiq_bottom__ = .;
    . = . + 0x0100;
    __stack_fiq_top__    = .;
    __stack_und_bottom__ = .;
    . = . + 0x0100;
    __stack_und_top__    = .;
    __stack_iap_bottom__ = .;
  } >ramstack

  usbramlayout :
  {
    __usbram_start__ = .;
    __usbram_end__   = 0x2000;
  } >usbram

/*
  ethramlayout :
  {
    __ethram_start__ = .;
    __ethram_end__   = 0x4000;
  } > ethram
*/

  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
}
