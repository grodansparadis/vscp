# vscp2drv_rpigpio.so

This is a Level II driver for the linux raspberry pi gpio interface

## Setup

Variable x_setup is read on startup. This should be an xml variable. x part of variable name
is specified on driver startup configuration.

The xml data is packed in

<?xml version = "1.0" encoding = "UTF-8" ?>
<setup>
... pin defines and dm here
</setup>

### mask
Standard VSCP mask in string form.
```
1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
```	
as *priority,class,type,GUID*
Used to filter what events that is received from 
the socketcan interface. If not give all events 
are received. 

### filter
Standard VSCP filter in string form. 
```
1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
```
as *priority,class,type,GUID*
Used to filter what events that is received from 
the socketcan interface. If not give all events 
are received.

### Input

Defines input pins and there functionality.

Inputs can either be monitored in the DM or or by callbacks. Events are sent as programmed. (CLASS1_INFORMATION, TYPE=3/4 ON/OFF)  Any CLASS1_INFORMATION event

Periodic reads are also possible.

#### pullup

This sets the pull-up or pull-down resistor mode on the given pin, which should be set 
as an input. The parameter should be;

* OFF, (no pull up/down)
* DOWN (pull to ground) 
* UP (pull to 3.3v) 
 
The internal pull up/down resistors have a value of approximately 50KOhm 
on the Raspberry Pi.

```xml
<input pin="1"
	pullup="off|up|down"

	<!-- 
		Periodic monitoring of input pin
		Send event on trigger (falling|rising|both|setup).
		CLASS1.INFORMATION TYPE=3/4 ON/OFF
	-->
	monitor_edge="falling|rising|both|setup|disable" <!-- if not give monitor is ignored (disable) -->
	monitor_event_class="20"
	monitor_event_type="3"
	monitor_event_index="0" 	<!-- Written to data byte 0 -->
	monitor_event_zone="11"		<!-- Written to data byte 1 -->
	monitor_event_subzone="22"	<!-- Written to data byte 3 -->
	monitor_event_data="1,2,3,4,,," 

	<!-- 
		Report pin status every "period" milliseconds
		CLASS1.INFORMATION TYPE=3/4 ON/OFF
	-->
	report_period="1000"

	report_event_high_class="20"
	report_event_high_type="3"
	report_event_high_index="0"		<!-- Written to data byte 0 -->
	report_event_high_zone="11"		<!-- Written to data byte 1 -->
	report_event_high_subzone="22" 	<!-- Written to data byte 2 -->
	report_event_high_data="1,2,3,4,,,"
	
	report_event_low_class="20"
	report_event_low_type="4"
	report_event_low_index="0"		<!-- Written to data byte 0 -->
	report_event_low_zone="11"		<!-- Written to data byte 1 -->
	report_event_low_subzone="22"		<!-- Written to data byte 2 -->
	report_event_low_data="1,2,3,4,,,"

</input>
```

### Output

Specify an output pin. Events that make things 
happen on the pin is defined in the decision matrix.

state is initial state of pin. set after initialization.


```xml
<output pin="n"
	initialstate="on|off" >
</output>		
```

### Pwm

Outputs pwm signal on pin.

pwmFrequency in Hz = 19.2e6 Hz / pwmClock / pwmRange.

Note that only pin 1 (BMC_GPIO 18, Phys 12) supports PWM output.

mask & space is default mode

<pwm pin="n" type="hard|soft" mode="balanced|markspace" range="1024" divisor="n" >
</pwm>

### GPIO clock

Only pin 7 (BCM_GPIO 4) supports CLOCK output.

<gpioclock pin="7" />

## DM

Decision matrix is used to tell what event should be used to set or clear outputs etc.

class type index zone subzone  => action

* If enable == "false" row is not read in. Default is "true"
* Compare data byte 0 of event with index if index is defined. Default is undefined.
* Compare data byte 1 of event with zone if zone is defined. Default is undefined.
* Compare data byte 2 of event with subzone if subzone is defined. Default is undefined.

## DM

```xml
<dm>
	<row enable="true|false"
		priority-mask="0xff"
		priotity-filter="0" 
		class-mask="0xffff"
		class-filter="30" 
		type-mask="0xffff"
		type-filter="5"
		guid-mask="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"
		guid-filter="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"
		index="0"
		zone="11"
		subzone="22"
		action="on"
		action-param="1,8,5"
	/>
</dm>
```

### Actions

#### ON
	action-parameter is comma separated list with pin to turn on. 
	pins must be defined as an output.

#### OFF
	Clear output pin.
	action-parameter is comma separated list with pins to turn off.
	pins must be defined as an output.

#### SET
	set output pin.
    action-parameter is pin state in binary for (01010101) first 8 GPIO pins
	bit-array of eight bits (01010101) in binary

#### PWM
	Generate hard/spoft pwm signal on output pin.
	action parameter is pin, pwm range value

#### Frequency (sound)
	Generate a tone on an output pin
	action parameter is pin, frequency, duration

#### Shiftout
	Shiftout action parameter data on output pin.
    action-parameter is, pin, data byte(s) to shift out
	
#### Shiftoutevent
	Shift out event data on output pin.
    action-parameter is: pin, offset to data byte(s) to shift out  CLASS1.CONTROL, Type=25

#### Shiftin 
	Shift in data on input pin.
	action-paramter: pin, # bytes (1-7), data coding  CLASS1.DATA

#### Status
	Return status for output pin. Can be used for sync events etc.
	action parameter: pin, event...


```xml
<?xml version = "1.0" encoding = "UTF-8" ?>
<setup>
    <input pin="1"
           pullup="up"

    monitor_edge="rising"
    monitor_event_class="20"
    monitor_event_type="3"
    monitor_event_index="0"
    monitor_event_zone="11"
    monitor_event_subzone="22"
    monitor_event_data="1,2,3,4,5,6"

    report_period="1000"

    report_event_high_class="20"
    report_event_high_type="3"
    report_event_high_index="0"
    report_event_high_zone="11"
    report_event_high_subzone="22"
    report_event_high_data="1,2,3,4,5,6,7,8"

    report_event_low_class="20"
    report_event_low_type="4"
    report_event_low_index="0"
    report_event_low_zone="11"
    report_event_low_subzone="22"
    report_event_low_data="1,2,3,4,5,6,7,8" />

</setup>
```


	


