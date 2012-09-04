This is a CANAL driver for the tellstick USB adapter (http://www.telldus.se) 

Authors:
	Ake Hedman <akhe@grodansparadis.com>
	Tord Andersson <tord.o.andersson@gmail.com>
	Micke Prag <micke.prag@telldus.se>
	Gudmund Berggren

The notes below comes from rfcmd.c (Copyright(C) Tord Andersson 2007) which lays 
the ground for this driver

=================================================================================

 Modifications from rfcmd.c ver 0.2 done by Gudmund
	Added support for IKEA
	Note:
		1. System code
		2. Level 0 == Off (For dimmers and non dimmers)
			Level 1== 10%. (for dimmers)
			....
			Level 9 == 90 % (for dimmers)
			Level 10 == 100 %  (or ON for non dimmers)
		3. Command line syntax:
			/usr/local/bin/rfcmd  /dev/ttyUSB0  IKEA 0 1 10 1"
			Arg 1: device 
			Arg 2: Protocol (IKEA)
			Arg 3: System code (0 = )
			Arg 4: Device code ( 1 = )
			Arg 5: Level (0=off, 1 = 10%, 2=20%,...9=90%, 10=100%)
			Arg 6: DimStyle (0=instant change, 1=gradually change)
 
=================================================================================

Configuration for the module is done with a XML file

<tellstick>
	<event type="on | off | dim | all" 
			zone="n"
			subzone="n">
		<device
			protocol="NEXA |SARTANO | WAVEMAN | IKEA"
			systemcode="1-16" <!-- Needed for IKEA -->
			devicecode="1-10" <!-- Needed for IKEA -->
			dimlevel="0-10"	<!-- If missing and dim event level/10 is used -->
			dimstyle="0 | 1"
			housecode="A-P" <!-- Needed for NEXA, WAVEMAN -->
			channel="1-16 [SARTANO 0-255] " <!-- Needed for NEXA, WAVEMAN and SARTANO -->
			state="0 | 1" <!-- 0 = off, 1 = on if absent event is used. Needed for NEXA, WAVEMAN, SARTANO -->
			
		/>	<!-- Many devies can be given for one event -->	
	</event>
</tellstick>

The driver understands four events

"all"
Class=30 (CLASS1.CONTROL ), Type=2 ALL LAMPS ON/OFF
	Reply: Class=30 (CLASS1.INFORMATION ), Type=3 for ON or Type=4 for OFF
		for each effected device.

"on"
Class=30 (CLASS1.CONTROL ), Type=5 TURN ON
	Reply: Class=30 (CLASS1.INFORMATION ), Type=3 for ON
		for each effected device.
	
"off"		
Class=30 (CLASS1.CONTROL ), Type=6 TURN OFF
	Reply: Class=30 (CLASS1.INFORMATION ), Type=4 for OFF
		for each effected device.
	
"dim"		
Class=30 (CLASS1.CONTROL ), Type=20 DIM
	Reply: Class=30 (CLASS1.INFORMATION ), Type=40 for LEVEL CHANGED
		for each effected device.	
		
Replies from the module are not really replies from the controlled nodes themselves 
as they don't issue a confirm. VSCP devices should ALWAYS reply back there state after 
a request to change it so here the driver take this responsability.


=================================================================================

Configuation data for the CANAL driver is as follows

device-id;path-to-config

device-id
=========
This is the id for the actual tellstick adapter. If not present the firts found 
adapter will be used.

path-to-config
==============
This is the path to the XML config file. If absent the driver will search for the 
file tellstick.xml in vscp folder the current users application data folder.

		