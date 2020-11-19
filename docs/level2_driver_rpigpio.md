# Raspberry Pi Level II GPIO driver

This is a driver that add support for GPIO control to the Raspberry Pi system. It can be used to get events sent when GPIO pins change states or have pin set/reset when specified VSCP events is received.

The excellent [pigpio library](http://abyz.me.uk/rpi/pigpio) is used for direct hardware control on the Raspberry Pi.

## Driver Linux
 
 vscpl2drv-rpigpio.so which use the [pigpio c library](http://abyz.me.uk/rpi/pigpio/cif.html).
 
and

 vscpl2drv-rpigpio-if2.so which use the [pigpio daemon](http://abyz.me.uk/rpi/pigpio/pigpiod.html).

 The advantage with the pigpiod usage is that several processes can use the the GPOIO library, meaning that you for example can have Python code controlling I/O alongside this driver.

## Prerequisites

 To use the drivers the pigpio package must be installed on the system. Install using

   sudo apt-get install pigpio

To use *vscpl2drv-rpigpio-if2.so* the **pigpiod** daemon must be started. See instructions in the [pigpio documentation](http://abyz.me.uk/rpi/pigpio).

## Configuration

### Configuration string
The configuration string have only one optinal item 

  [config-xml-base64]

This an XML file of format as described below which is BASE64 encoded. If this configuration item is not available the XML configuration will be fetched from the specified server as a VSCP remote variable.

### XML Configuration

If not specified in the configuration string the driver will look for a VSCP remote variable on the local server that is named **prefix_setup** where **prefix** is the prefix set in the driver setup. For example

```xml
<driver enable="true" >
    <name>gpiodrv1</name>
    <path>/srv/vscp/drivers/level2/vscpl2drv-rpigpio-if2.so</path>
    <config></config>
    <guid>FF:FF:FF:FF:FF:FF:FF:FC:01:02:03:04:05:06:07:08</guid>
</driver>
```

The prefix here is **gpiodrv1** so in this case the driver will look for a remote variable **gpiodrv1_setup** on the same server that started the driver to find XML configuration. The variable can be either a string or a XML remote variable. 

The other option is to put a BASE64 encoded XML configuration string between the **\<setup\>** tag. If both ar available the VSCP remote variable will be used. 

#### Format

```xml
<?xml version = "1.0" encoding = "UTF-8" ?>

<setup 	pigpiod-host="host where pigpiod is (default:"localhost")"
    pigpiod-port="port on host where pigpiod is (default:"8888")"
    sample_rate="5"
    primary_dma_channel="14"
    secondary_dma_channel="6"
    filter="Incoming filter on string form"
    mask="Incoming mask on string form"
    guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00" 
    index="0"
    zone="0"
    subzone="0" >

... pins and dm defined here

</setup>
```

**pigpiod-host** and **pigpiod-port** is only for the vscpl2drv-rpigpio-if2.so driver.

**sample-rate**, **primary_dma_channel** and **secondary_dma_channel** is only for the driver usage and is selldom changed from default values and can be left out. For the pigpiod version they are specified in the daemon setup. See [pigpiod documentation](http://abyz.me.uk/rpi/pigpio/pigpiod.html).

**filter** and **mask** specify a filter for events received by the driver.

##### mask
Standard VSCP mask in string form.
```
0,0x0000,0x0000,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
```	
as *priority,class,type,GUID*
Used to filter what events that is received from 
the socketcan interface. If not give all events 
are received.

##### filter
Standard VSCP filter in string form. 
```
1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
```
as *priority,class,type,GUID*
Used to filter what events that is received from 
the socketcan interface. If not give all events 
are received.

**index**, **zone** and **subzone** is default values for events sent by the driver.

##### Inputs

Defines input pins and there functionality. Inputs can either be monitored in the DM or or by callbacks. Events are sent as programmed. (CLASS1_INFORMATION, TYPE=3/4 ON/OFF)  
Any CLASS1_INFORMATION event

Periodic reads are also possible. Se **reports** below.

```xml
<input pin="n"
	pullup="off|up|down"
	watchdog="ms"
	noise_filter_steady="ms"		
	noise_filter_active="ms"		
	glitch_filter="ms" />
```
###### pin (mandatory)
Input (BCM) pin. Valid values are 0-31.

###### pullup (optional)

This sets the pull-up or pull-down resistor mode on the given pin, which should be set as an input. The parameter should be;

* OFF, (no pull up/down)
* DOWN (pull to ground) 
* UP (pull to 3.3v) 

The internal pull up/down resistors have a value of approximately 50KOhm on the Raspberry Pi.

###### watchdog (optional)
0-60000. Until cancelled a timeout will be reported every timeout milliseconds after the last GPIO activity.  Cancel watchdog by setting it to zero. This affects only monitor setups.

###### noice_filter_steady (optional)
0-300000. Level changes on the GPIO are ignored until a level which has been stable for steady microseconds is detected. Level changes on the GPIO are then reported for active microseconds after which the process repeats. 

###### noice_filter_active (optional)
0-1000000. Level changes on the GPIO are ignored until a level which has been stable for steady microseconds is detected. Level changes on the GPIO are then reported for active microseconds after which the process repeats. 

###### glitch_filter (optional)
0-300000. Level changes on the GPIO are not reported unless the level has been stable for at least steady microseconds. The level is then reported. Level changes of less than steady microseconds are ignored. 


##### Outputs

Specify an output pin. Events that make things 
happen on the pin is defined in the decision matrix.

```xml
<output pin="n"
	initial_state="on|1|off|0|x" />
```

###### pin (mandatory)

This is the pin that should be set as output. Valid values is 0-31.

###### initial_state (optional)
Set the initial state of the "pin's" output after initialization. If not specified or set to *x* no initial state is set.


##### PWM

Outputs pwm signal on pin.

pwmFrequency in Hz = 19.2e6 Hz / pwmClock / pwmRange.

Note that only pin 1 (BMC_GPIO 18, Phys 12) supports PWM output.


```xml
<pwm pin="n" 
		hardware="true|false"
		range="255"
		frequency="n"
		dutycycle="40" />
```

##### Clock

Only pin 7 (BCM_GPIO 4) supports CLOCK output.

```xml
<gpioclock pin="7" frequency="5000000" />
```


##### Input monitoring

Send event on trigger (falling|rising|both|setup).
CLASS1.INFORMATION TYPE=3/4 ON/OFF

```xml
<monitor pin="27"
		edge="falling|rising|both"

		falling_class="20"
		falling_type="79"
		falling_index="0"
		falling_zone="11"
		falling_subzone="22"
		falling_data="0,0,0"

		rising_class="20"
		rising_type="78"
		rising_index="0"
		rising_zone="11"
		rising_subzone="22"	 
		rising_data="0,0,0"
			
		watchdog_class="1"
		watchdog_type="12"
		watchdog_index="0"
		watchdog_zone="11"
		watchdog_subzone="22"	 
		watchdog_data="0,0,0" />
```

##### Input reporting

Report input pin status every "period" milliseconds
CLASS1.INFORMATION TYPE=3/4 ON/OFF
Can be set to id = 0..9


```xml
<setup>

<input pin="27"
	pullup="up"  />

<report id="0"
    pin="27"
    period="1000"

    high_class="20"
    high_type="3"
    high_index="27"
    high_zone="11"
    high_subzone="22"
    high_data="27,0,0"

    low_class="20"
    low_type="4"
    low_index="27"
    low_zone="11"
    low_subzone="22"
    low_data="27,0,0" />

</setup>
```
By default index is used for the pin.

##### Driver decision matrix (DM)

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
		action-parameter="1,8,5"
	/>
</dm>
```

### Actions

#### ON
Set output pin(s)

CLASS1.INFORMATION, Type=3, ON   index = pin
CLASS1.INFORMATION, Type=4, OFF	 index = pin

**action-parameter** is a comma separated list with pins to turn on. Pins must have been defined as an output.

#### OFF
Clear output pin(s).

CLASS1.INFORMATION, Type=3, ON   index = pin
CLASS1.INFORMATION, Type=4, OFF	 index = pin

**action-parameter** is comma separated list with pins to turn off. Pin must have been defined as nb output.

#### PWM
Generate hard/soft PWM signal on output pin. Default range of 255

CLASS1.INFORMATION, Type=3, ON   index = pin
CLASS1.INFORMATION, Type=4, OFF	 index = pin

##### Confirm
CLASS1.INFORMATION, Type=51 (0x33) - Big level changed 

**action parameter** is pin, duty-cycle (0-range)

#### Frequency (sound)
Generate a tone on an output pin
action parameter is pin, frequency, duration

##### Confirm
CLASS1.INFORMATION, Type=51 (0x33) - Big level changed

#### Servo
**action parameter** is pin, pulse-width( 0-255)

##### Confirm
CLASS1.INFORMATION, Type=51 (0x33) - Big level changed

#### Waveform

#### Sample

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
CLASS1.INFORMATION, Type=3, ON   index = pin
CLASS1.INFORMATION, Type=4, OFF	 index = pin
action parameter: pin, zone, subzone



```

[filename](./bottom_copyright.md ':include')
