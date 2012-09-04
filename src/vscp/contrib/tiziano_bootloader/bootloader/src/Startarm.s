/*****************************************************************************
 *
 * Project          : profirm22
 * Subproject       : 
 * Name             : Startarm.s
 * Function         : Initialisation and vector table
 * Designer         : 
 * Creation date    : 
 * Compiler         : GNU ARM
 * Processor        : LPC2364
 * Last update      :
 * Last updated by  :
 * History          :
 *
 *****************************************************************************
 *
 * See linker script lpc2364_rom.ld for memory map
 *
 ****************************************************************************/


/*****************************************************************************
 *
 * External references
 *
 ****************************************************************************/

/*
 * Located in linker script lpc2364_rom.ld
 */
.extern        __data_beg_src__
.extern        __data_beg__
.extern        __data_end__

.extern        __bss_beg__
.extern        __bss_end__

.extern        __stack_scv_top__
.extern        __stack_irq_top__
.extern        __stack_fiq_top__
.extern        __stack_und_top__

/*
 * Located in handlers.c
 */
.extern        Undef_Handler
.extern        SWI_Handler
.extern        PAbt_Handler
.extern        DAbt_Handler
.extern        FIQ_Handler

/*
 * Located in init_cpu.c
 */
.extern        init_wait
.extern        init_clocks
.extern        init_mam
.extern        init_pins
.extern        init_cpu
.extern        init_modules
.extern        exit_main

/* 
 * Located in main.c
 */
.extern        main

/*****************************************************************************
 *
 * Definitions
 *
 ****************************************************************************/

/*
 * Standard definitions for Mode and interrupt bits in PSRs
 */
.equ           Mode_USR,        0x10
.equ           Mode_FIQ,        0x11
.equ           Mode_IRQ,        0x12
.equ           Mode_SVC,        0x13
.equ           Mode_ABT,        0x17
.equ           Mode_UND,        0x1B
.equ           Mode_SYS,        0x1F

.equ           I_Bit,           0x80
.equ           F_Bit,           0x40

/*
 * Definitions for IO registers
 */
.equ           IOPIN0,          0xE0028000
.equ           IODIR0,          0xE0028008

.equ           IOPIN1,          0xE0028010
.equ           IODIR1,          0xE0028018

.equ           IOPIN2,          0x3FFFC054
.equ           IODIR2,          0x3FFFC040

.equ           IOPIN3,          0x3FFFC074
.equ           IODIR3,          0x3FFFC060

.equ           IOPIN4,          0x3FFFC094
.equ           IODIR4,          0x3FFFC080

/*
 * PORT setup values
 */
.equ           IOPIN0_VALUE,    0x00000000
.equ           IODIR0_VALUE,    0xF0000000

.equ           IOPIN1_VALUE,    0x00000000
.equ           IODIR1_VALUE,    0x00000000

.equ           IOPIN2_VALUE,    0x00000000
.equ           IODIR2_VALUE,    0x00000000

.equ           IOPIN3_VALUE,    0x00000000
.equ           IODIR3_VALUE,    0x00000000

.equ           IOPIN4_VALUE,    0x00000000
.equ           IODIR4_VALUE,    0x00000000

/*****************************************************************************
 *
 * Vector table
 *
 ****************************************************************************/

.section .startup,"ax"
.arm
.align 0

_vectors:
              B     _reset_handler
              LDR   PC,Undef_Addr
              LDR   PC,SWI_Addr
              LDR   PC,PAbt_Addr
              LDR   PC,DAbt_Addr
              NOP                             
              LDR   PC,[PC, #-0x0120] 
              LDR   PC,FIQ_Addr

Undef_Addr:   .word   Undef_Handler
SWI_Addr:     .word   SWI_Handler
PAbt_Addr:    .word   PAbt_Handler
DAbt_Addr:    .word   DAbt_Handler
FIQ_Addr:     .word   FIQ_Handler
			  /*.=.+0x1C0*/

/*			  
.org 0x01FC, 0xFF

PROTECTION_CODE:
              .word   0xFFFFFFFF

.size _vectors, . - vectors			  
*/

.global _reset_handler

_reset_handler:
			  /*LDR    R0,=0xE01FC040
			  MOV    R1, #2
			  STR    R1, [R0]*/	
/* 
 * Setup basic stack for initialize code
 */
              LDR    R0,=__stack_scv_top__
              MSR    CPSR_c, #Mode_SVC|I_Bit|F_Bit
              MOV    SP,R0
				
/* 
 * Wait a moment so that ULink can interrupt use
 *  This is done before the PLL is running.
 */
              /* BL     init_wait SE C'E' NON PARTE IL DEBUGGER */

/* 
 * Initalize basic modules
 */
              BL     init_clocks
              BL     init_mam
              BL     init_pins

/* 
 * Setup stacks
 */

/*         Undefined instruction -> Undefined mode, uses Undefined stack    */
              LDR   R0,=__stack_und_top__
              MSR   CPSR_c, #Mode_UND|I_Bit|F_Bit
              MOV   SP,R0

/*         Abort (prefetch+data) -> Abort mode, uses Undefined stack        */
              MSR    CPSR_c, #Mode_ABT|I_Bit|F_Bit
              MOV    SP,R0

/*         FIQ -> FIQ mode, use FIQ stack                                   */
              LDR    R0,=__stack_fiq_top__
              MSR    CPSR_c, #Mode_FIQ|I_Bit|F_Bit
              MOV    SP,R0

/*         IRQ -> IRQ mode, use IRQ stack                                   */
              LDR    R0,=__stack_irq_top__
              MSR    CPSR_c, #Mode_IRQ|I_Bit|F_Bit
              MOV    SP,R0

/*         Supervisor mode, use supervisor stack                            */
              LDR    R0,=__stack_scv_top__
              MSR    CPSR_c, #Mode_SVC|I_Bit|F_Bit
              MOV    SP,R0

/*         We keep running in supervisor mode                               */

/*
 * Initialize .data : Copy from Flash to RAM
 */
              LDR    R1,=__data_beg_src__
              LDR    R2,=__data_beg__
              LDR    R3,=__data_end__
LoopRel:      CMP    R2, R3
              LDRLO  R0, [R1], #4
              STRLO  R0, [R2], #4
              BLO    LoopRel

/*
 * Initialize .bss : Make zero
 */
              MOV    R0, #0
              LDR    R1,=__bss_beg__
              LDR    R2,=__bss_end__
LoopZI:       CMP    R1, R2
              STRLO  R0, [R1], #4
              BLO    LoopZI


/*
 * Initialize CPU
 */
              BL     init_cpu
              BL     init_modules

/*
 * Enable interrupts
 */
              MSR    CPSR_c, #Mode_SVC

/*
 * Jump to main
 */
              BL    main

/* 
 * Jump to Exit_Main
 */
              BL    exit_main

/****************************************************************************
 *
 * Endless loop, normaly never executed
 *
 ****************************************************************************/

endless_loop:
              B     endless_loop

.size _reset_handler, . - _reset_handler


/****************************************************************************
 *
 * End marker
 *
 ****************************************************************************/

.end
