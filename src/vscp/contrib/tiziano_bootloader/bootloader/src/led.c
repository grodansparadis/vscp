// *****************************************************************************
// FILENAME:                                                    
//                                                                           
// VERSION:                                                              
//                                                                          
// DESCRIPTION:                                   
//                                                                           
// TOOLS:                             		  
//                                                                           
// REVISION HISTORY                                                          
// Version  Author           Date        Remarks							  
//                                      
// ****************************************************************************
                  
#include <stdio.h>
#include <LPC23xx.H>  
#include "led.h"

typedef struct 
{
	unsigned LEDImg;
	unsigned LEDState;
	unsigned LEDBlink;
} LED;

static unsigned led_prescaler;
static unsigned ledblink_prescaler;
static unsigned ledblink_state;
static unsigned led_data[N_LED];
static LED Led[N_LED];

static void led_out(void);

///////////////////////////////////////////////////////////////////////////////

// Function that initializes LEDs                                             
void init_led(void) 
{
	FIO2DIR |= (PIN_LED1 | PIN_LED3 | PIN_LED4 | PIN_LED5); // pin defined as Outputs
	FIO3DIR |= (PIN_LED2);                					// pin defined as Outputs
	FIO1DIR |= (PIN_LED6 | PIN_LED7);      					// pin defined as Outputs
}

// Function that initializes LEDs                                             
void open_led(void) 
{
  	int i;

  	init_led();

	led_prescaler = LED_PRESCALER;
	ledblink_prescaler = BLINK_PRESCALER;
	ledblink_state = 0;

	for(i=0; i < N_LED; i++)
	{
		led_data[i] = 0;	 
		Led[i].LEDImg = 0;	 
		Led[i].LEDState = 0;	 
		Led[i].LEDBlink=0;	 
	}

	led_out();
	
	//test_led();
}

// Function that turns on requested LED                                       
void led_on (unsigned int num) 
{
	Led[num].LEDImg = 1;
	Led[num].LEDState = 1;
}

// Function that turns off requested LED                                      
void led_off (unsigned int num) 
{
	Led[num].LEDImg = 0;
	Led[num].LEDState = 0;
}

// Function that outputs value to LEDs                                        
static void led_out(void) 
{
  	if(led_data[LED1])
  		FIO2SET = PIN_LED1;
  	else
  		FIO2CLR = PIN_LED1;
  	if(led_data[LED2])
  	 	FIO3SET = PIN_LED2;
  	else
		FIO3CLR = PIN_LED2;
  	if(led_data[LED3])
  	 	FIO2SET = PIN_LED3;
  	else
  		FIO2CLR = PIN_LED3;
  	if(led_data[LED4])
  	 	FIO2SET = PIN_LED4;
  	else
  		FIO2CLR = PIN_LED4;
  	if(led_data[LED5])
  	  	FIO2SET = PIN_LED5;
  	else
  		FIO2CLR = PIN_LED5;
  	if(led_data[LED6])
  	  	FIO1SET = PIN_LED6;
  	else
  		FIO1CLR = PIN_LED6;
  	if(led_data[LED7])
  	  	FIO1SET = PIN_LED7;
  	else
  		FIO1CLR = PIN_LED7;
}

void led_blinkon(unsigned code_led)
{
	Led[code_led].LEDBlink = 1;
}

void led_blinkoff(unsigned code_led)
{
	Led[code_led].LEDBlink = 0;
	Led[code_led].LEDImg = Led[code_led].LEDState;
}

void led_onall(void)
{
	int i;

	for(i=0; i < N_LED; i++)
	{
		Led[i].LEDImg = 1;
		Led[i].LEDState = 1;
	}
}

void led_offall(void)
{
	int i;

	for(i=0; i < N_LED; i++)
	{
		Led[i].LEDImg = 0;
		Led[i].LEDState = 0;
	}
}

void led_blinkoffall(void)
{
	int i;

	for(i=0; i < N_LED; i++)
	{
		Led[i].LEDBlink = 0;
		Led[i].LEDImg = Led[i].LEDState;
	}
}

void led_blinkonall(void)
{
	int i;

	for(i=0; i < N_LED; i++)
	{
		Led[i].LEDBlink = 1;
	}
}

// **************************************************************************
// FUNCTION:                               
//                                                                           
// DESCRIPTION:    
//                 
// GLOBAL VARIABLES USED:                                                    
//                                                                           
// PARAMETERS:                                                               
//                                                                           
// RETURN:                    
//                                
// **************************************************************************

void exec_led(void)
{
	int i;

	--led_prescaler;
	if(!led_prescaler)
	{
		led_prescaler = LED_PRESCALER;
		for (i=0; i < N_LED; i++) 
		{
			led_data[i] = Led[i].LEDImg;
		}
		led_out();
	}
	--ledblink_prescaler;
	if(!ledblink_prescaler)
	{
		ledblink_prescaler = BLINK_PRESCALER;
		if (ledblink_state & 1)
		{
		    for (i=0; i < N_LED; i++) 
		    {
		    	if(Led[i].LEDBlink) Led[i].LEDImg = 1;   
			}
		}
		else
		{
		    for (i=0; i < N_LED; i++) 
		    {
		    	if(Led[i].LEDBlink) Led[i].LEDImg = 0;
			}
		}
		ledblink_state++;
	}
}

// **************************************************************************
// FUNCTION:                               
//                                                                           
// DESCRIPTION:    
//                 
// GLOBAL VARIABLES USED:                                                    
//                                                                           
// PARAMETERS:                                                               
//                                                                           
// RETURN:                    
//                                
// **************************************************************************

void test_led(void)
{
	/*
	led_on (LED1);
	led_on (LED2);
	led_on (LED3);
	led_on (LED4);
	led_on (LED5);
	led_on (LED6);
	led_on (LED7);
	
	led_blinkon (LED1);
	led_blinkon (LED2);
	led_blinkon (LED3);
	led_blinkon (LED4);
	led_blinkon (LED5);
	led_blinkon (LED6);
	led_blinkon (LED7);
	*/
	
	led_onall();
	
	led_blinkonall();
}
