// ***************************************************************
//
// DESCRIPTION: program loop 
// 		   
// AUTHOR:
//
// HISTORY:    
//
// *******************************************************************
//
//  Called from startarm.s
//
// *******************************************************************

#include <string.h>
#include "LPC23xx.H"
#include "config.h"
#include "vic.h"
#include "irq.h"
#include "utils.h"
#include "tmr.h"
#include "buff.h"
#include "uart.h"
#include "upload.h"
#include "led.h"
#include "i2c.h"
#include "flash.h"
#include "config.h"
#include <inttypes.h>
#include "vscp.h"
#include "vscp_main.h"

extern unsigned long measurement_clock;
//---------------------------------------------
//* Functions
//---------------------------------------------
void open_tasks(void);
void __attribute__ ((interrupt("IRQ"))) IRQ_Timer1(void);

//---------------------------------------------
//* Constants Definitions
//---------------------------------------------

#define	FOREVER	1

enum NTASK 
{	
	N_Task1,
	N_Task2,
	N_Task3,
	N_Task4,
	OS_MAX_TASKS
};

#define	exec_task1	exec_tmr
#define PRESCALER_TASK1	2

#define	exec_task2	exec_io
#define PRESCALER_TASK2 1

#define	exec_task3	exec_rdwr
#define PRESCALER_TASK3 1

#define	exec_task4	exec_vscp
#define PRESCALER_TASK4 1

//---------------------------------------------
//* Global Variable Definitions
//---------------------------------------------
static UINT32 task_prescaler[OS_MAX_TASKS];
static UINT32 task_mod[OS_MAX_TASKS];
static UINT32 task_cnt[OS_MAX_TASKS];
static int counter = 0;

// int pippo[1]; test flash

int main (void)  
{
	int i;
		
	delay(20);
	
	// strobe
	STROBE_INIT;
	STROBE_OFF;
	
	if(user_code_present())
		execute_user_code();
	
	open_tasks();
		
	print_welcomemessage();
	
	// test flash
	/*
	i = flashInit();
	PrintString((UINT8 const *)"\n\rflashInit ret=");
	PrintInt((UINT32)i);
	i = flashErase(7, 10);
	PrintString((UINT8 const *)"\n\rflashErase ret=");
	PrintInt((UINT32)i);
	pippo[0]= 0x55667788;
	i = flashWrite((int *)(INT_FLASH_SIGNATURE_ADDRESS), (int *)pippo, 4);
	PrintString((UINT8 const *)"\n\rflashWrite ret=");
	PrintInt((UINT32)i);
	i = *((int *)INT_FLASH_SIGNATURE_ADDRESS);
	PrintString((UINT8 const *)"\n\rread flash=");
	PrintInt((UINT32)i);
	*/	
	// fine test flash
	
	do 
    {
		for(i=0; i < OS_MAX_TASKS; i++) 
		{
			if(!task_prescaler[i])
			{
				(*((void (*)(void))task_mod[i]))();
			    task_prescaler[i]=task_cnt[i];	
			}
		}
	}								  
	while (FOREVER);
}

///////////////////////////////////////////////////////////////////////////////

void __attribute__ ((interrupt("IRQ"))) IRQ_Timer1(void)
{  
	int i;
	
	//STROBE_ON;
	
	vscp_timer++;
	measurement_clock++;
	counter++;

	for(i=0 ; i < OS_MAX_TASKS ;i++) 
	{
		if(task_prescaler[i]) 
		{	
			--task_prescaler[i];
		}
	}
	
	//STROBE_OFF;
	
	T1IR = 0x01;	    		// CLR Timer1 interrupt flag
	VICVectAddr = 0x00;			// Inform VIC interrupt is end
}

///////////////////////////////////////////////////////////////////////////////
void open_timer1(void)
{
	//-----------------------------------------------
	// Config Timer1
	//-----------------------------------------------  
	T1PR = 0;			// Disable prescaler
	T1MCR = 0x03;		// Using T1MR0 match T1TC,Enable interrupt,reset T1TC
	T1MR0 = Fpclk/1000;	// 1ms each Timer interrupt
	T1TCR = 0x02;		// Reset T1TC, by set TCR:2
	T1TCR = 0x01;		// Must CLR TCR:2, then can Start T1TC
	install_irq( TIMER1_INT, IRQ_Timer1, TIMER1_INT );
}


void open_tasks(void)
{
  	task_mod[N_Task1] = (UINT32)&exec_task1;
  	task_cnt[N_Task1] = PRESCALER_TASK1;
	task_prescaler[N_Task1] = task_cnt[N_Task1]; 
	
  	task_mod[N_Task2] = (UINT32)&exec_task2;
  	task_cnt[N_Task2] = PRESCALER_TASK2;
	task_prescaler[N_Task2] = task_cnt[N_Task2];

	task_mod[N_Task3] = (UINT32)&exec_task3;
  	task_cnt[N_Task3] = PRESCALER_TASK3;
	task_prescaler[N_Task3] = task_cnt[N_Task3];
	
	task_mod[N_Task4] = (UINT32)&exec_task4;
	task_cnt[N_Task4] = PRESCALER_TASK4;
	task_prescaler[N_Task4] = task_cnt[N_Task4];

	//-----------------------------------------------
	// Config VIC, 
	// In <Starup.s> enable IRQ by CLR I bit in CPSR
	// Vectored interrupt Timer1 by slot-0
	//-----------------------------------------------
	init_vic();
	
	//-----------------------------------------------
	// Config UART0
	//-----------------------------------------------  
	open_uart();
	
	//-----------------------------------------------
	// init led, key
	//-----------------------------------------------
	open_io();
	
	//-----------------------------------------------
	// init i2c-eeprom
	//-----------------------------------------------
	open_i2c();

	//-----------------------------------------------
	// init event buffer
	//-----------------------------------------------
	open_rdwr();
	
	//-----------------------------------------------
	// timer manager
	//-----------------------------------------------  
	open_tmr();
	
	//-----------------------------------------------
	// Config spi
	//-----------------------------------------------  
	open_upload();
	
	//-----------------------------------------------
	// Config CAN
	//-----------------------------------------------  
	open_vscp();
	
	//-----------------------------------------------
	// Config Timer1
	//-----------------------------------------------  
	open_timer1();
	 
}

int GetFreeRunning(void)
{
	return counter;
}


