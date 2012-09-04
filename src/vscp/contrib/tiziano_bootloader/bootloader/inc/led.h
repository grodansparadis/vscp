// ****************************************************************************
// led.h                                          
// *****************************************************************************
//  
//  
//  
//  
//  
// ****************************************************************************
#ifndef __LED_H 
#define __LED_H

//-----------------------------------------------
// Constant Definitions
//-----------------------------------------------

#define	PIN_LED1 PORT2_BIT10
#define	PIN_LED2 PORT3_BIT25
#define	PIN_LED3 PORT2_BIT1
#define	PIN_LED4 PORT2_BIT3
#define	PIN_LED5 PORT2_BIT8
#define	PIN_LED6 PORT1_BIT22
#define	PIN_LED7 PORT1_BIT25

typedef enum LEDS_tag
{
	LED1,
	LED2,
	LED3,
	LED4,
	LED5,
	LED6,
	LED7,
	N_LED
} enLED;

#define LED_PRESCALER		20	// prescaler per output sui led = 20 ms 
#define BLINK_PRESCALER		100	// prescaler per il blink = 100 ms
///////////////////////////////////////////////////////////////////////////////
extern void init_led (void);
extern void led_on (unsigned int num);
extern void led_off (unsigned int num);
extern void open_led(void);
extern void exec_led(void);
extern void led_blinkon(unsigned);
extern void led_blinkoff(unsigned);
extern void led_blinkonall(void);
extern void led_blinkoffall(void);
extern void led_onall(void);
extern void led_offall(void);
extern void test_led(void);
///////////////////////////////////////////////////////////////////////////////

#endif
