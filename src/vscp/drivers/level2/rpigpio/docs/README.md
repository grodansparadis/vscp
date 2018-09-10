# vscp2drv_rpigpio.so

This is a Level II driver for the linux raspberry pi gpio interface

## Setup

### _SETUP_PINMODEn=mode


```xml
<setup pin="18" mode="input|output|pwm|clock" pullup="off|down|up" state="on/off>
```

state is initial state for output. Set after initialization

**json:** "pinmode18" : "input"

One for each pin. _SETUP_PINMODE18 is for pin 18

Values can be 
  
  * INPUT
  * OUTPUT
  * PWM_OUTPUT
  * GPIO_CLOCK. 
  
Note that only pin 1 (BCM_GPIO 18) supports PWM output and only pin 7 (BCM_GPIO 4) supports CLOCK output modes.


### _SETUP_PULLUPn=pud

**json:** "pullup1" : "up"

This sets the pull-up or pull-down resistor mode on the given pin, which should be set 
as an input. The parameter should be;

* OFF, (no pull up/down)
* DOWN (pull to ground) 
* UP (pull to 3.3v) 
 
The internal pull up/down resistors have a value of approximately 50KO 
on the Raspberry Pi.

### _SETUP_INITIALn=state

**json:** "init1" : "on"

Initial state for output.

* ON for a pin a high lever
* OFF for a low pin.

## Inputs

```xml
<input pin="1"
	pullup="off|up|down" >

	<!-- irq -->
	<monitor period="10" >
		<high class="20"
			type="3"
			index="0"
			zone="11"
			subzone="22" />
		<low class="20"
			type="4"
			index="0"
			zone="11"
			subzone="22" />	
	</monitor>

	<!-- Report pin status every x seconds CLASS1.INFORMATION TYPE=3/4 ON/OFF-->
	<report period="5"
			input="1"
			index="0"
			zone="11"
			zubzone="22"/>
		<high class="20"
			type="3"
			index="0"
			zone="11"
			subzone="22" />
		<low class="20"
			type="4"
			index="0"
			zone="11"
			subzone="22" />
	</report>	
</input>
```

<input>  	- input		<report> <monitor>
<output>	- output	<status>
<pwm>		- pwm
<clock>		- GPIO CLOCK
<tone>
<softpwm>
<shift>


Inputs can either be monitored in the DM or or by call backs. Events are sent as programmed. (CLASS1_INFORMATION, TYPE=3/4 ON/OFF)  Any CLASS1_INFORMATION event

Periodic reads are also possible.

## Outputs

```xml
<output pin="8"
	pullup="off" />

<!-- Sent event that set state-->
<trigger pin="8" state="on"
	class="20"
	type="3"
	index="0"
	zone="11"
	subzone="22" />
```

### Callbacks (interrupts)

**params:** pin, edgetype

#### Edgetypes

* FALLING
* RISING
* BOTH
* EDGE

## Outputs

Decision matrix is used to tell what event should be used to set or rest outputs.

_OUTPUT_MATRIXn=flags;prio;class;type;action;action-param;d0;d1;...

rows must be consecutive.

class type index zone subzone  => action

### flags

bit00 - Compare D0
bit01 - Compare D1
bit02 - Compare D2
bit03 - Compare D3 
bit04 - Compare D4
bit05 - Compare D5
bit06 - Compare D6
bit07 - Compare D7
bit08 - 
bit09 -
bit10 - 
bit11 -  
bit12 - 
bit13 -
bit14 - 
bit15 -  
bit16 - 
bit17 -
bit18 - 
bit19 -  
bit20 - 
bit21 -
bit22 - 
bit23 -  
bit24 - 
bit25 -
bit26 - 
bit27 -  
bit28 - 
bit29 -
bit30 - 
bit31 - Enable 

## DM

```xml
<dm>
	<row class-mask="0xffff"
		class-filter="30" 
		type-mask="0xffff"
		type-filter="5"
		guid-mask="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"
		guid-filter="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"
		index="0"
		zone="11"
		subzone="22"
		action="on"
		param="1,8,5"
	/>
</dm>
```

### Actions

#### ON
	action-parameter is comma separated list with pins to turn on.

#### OFF
	action-parameter is comma separated list with pins to turn off.

#### SET
    action-parameter is pinstate as binary
	bit-array (011110)

#### PWM
	action parameter is value

#### Frequency (sound)
	action parameter is frequency, duration

#### Shift out
    action-parameter is data byte to shift out
	
#### Shift out from data
    action-parameter is offset to data byte to shift out

	


