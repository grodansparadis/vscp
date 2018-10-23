# VSCP Level II driver for MAX6675 interfaces thermocouple

![VSCP Logo](https://www.vscp.org/images/logo_200.png)

This driver interface one or more K type thermocouples connected to MAX6675 SPI interface connected to one of the SPI interfaces on a Raspberry Pi. If you need hardware it can be found on [Ebay](https://www.ebay.com/itm/NEW-MAX6675-Module-K-Type-Thermocouple-Temperature-Sensor-Arduino-/272321510687) or [AliExpress](https://www.aliexpress.com/item/MAX6675-Module-K-Type-Thermocouple-Thermocouple-Senso-Temperature-Degrees-Module-for-arduino/32841448771.html?spm=2114.search0204.3.1.371980b8u1Rknu&ws_ab_test=searchweb0_0,searchweb201602_1_10065_10068_318_319_10546_317_10548_10696_450_10084_10083_10618_452_535_534_533_10307_10820_532_10303_204_10059_10884_323_10887_100031_320_10103_448_449,searchweb201603_60,ppcSwitch_0_ppcChannel&algo_expid=cf22e2e5-120a-4654-8619-09600ef8612e-0&algo_pvid=cf22e2e5-120a-4654-8619-09600ef8612e&transAbTest=ae803_5) or similar for a low cost. 

It is possible to measure temperatures with 12-bit resolution in the range 0-1023 degrees (resolution: 0.25 degrees Celsius) with a higher accuracy up to 700 degrees.

## Building the driver
If you build the VSCP package from source and want to build this driver you should add the switch **--enable-rpi** to configure as

  ./configure --enable-rpi

 that will build the Raspberry Pi specific components and drivers for VSCP & Friends 

## Enable SPI on the Raspberry Pi
Use **raspi-config** to enable the SPI interface hardware drivers.

Sparcfun have a [good tutorial]((https://learn.sparkfun.com/tutorials/raspberry-pi-spi-and-i2c-tutorial/all)) on both SPI and I2C and where you should connect the tour device.

## Driver configuration

```xml
<driver enable="true" >
    <name>driver-name</name>
    <path>path to driver</path>
    <config>Optional BINHEX64 coded XML config</config>
    <guid>00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00</guid>
</driver>
```

## XML Configuration

The configuration can be fetched from the driver configuration as BASE64 encoded XML data or from the server starting the driver from a XML varible **name_setup** where **name** is the driver-name used in the driver configuration above.

The XML configuration have the following format

```xml
<?xml version = "1.0" encoding = "UTF-8" ?>
<setup filter=""
        mask=""
        guid=""
        index="" 
        zone="" 
        subzone="" >
    <max6675 enable="true|false" 
                spichannel="0|1" 
                unit="0|1|2|C|K|F" 
                detect-open="true|false"
                interval="seconds" 
                vscpclass="10" 
                index="" 
                zone="" 
                subzone="" />
</setup>
```

There can be more than one <interface> tag but they must use different bus id's (0/1).

 * **enable** - Enable the interface. Item disregarded if disabled. 
 * **bus** - SPI bus. Can currently be 0 or 1.
 * **unit** - This is the unit for the temperature measurement. Can me 0/1/2 or "K"/"C"/"F" representing units Kelvin, degrees Celsius and degrees Fahrenheit.
 * **detect-open** - If true and open thermocouple will be detected.
 * **interval** - Interval in seconds between temperature events.
 * **vcpclass** - Measurement class that should be used of temperature events. See generated events below.
 * **index** - Index to use if event of selected VSCP class require it.
 * **zone** - Zone to use if event of selected VSCP class require it.
 * **Subzone** - Subzone to use if event of selected VSCP class require it.

## Generated events

Event can be sent from the driver as

 * CLASS1.MEASUREMENT = 10, Type=6, VSCP_TYPE_MEASUREMENT_TEMPERATURE 
 * CLASS1.MEASUREMENT64 = 60, Type=6, VSCP_TYPE_MEASUREMENT_TEMPERATURE
 * CLASS1.MEASUREZONE = 65, Type=6, VSCP_TYPE_MEASUREMENT_TEMPERATURE
 * CLASS1.MEASUREMENT32 = 70, Type=6, VSCP_TYPE_MEASUREMENT_TEMPERATURE
 * CLASS1.SETVALUEZONE = 85, Type=6, VSCP_TYPE_MEASUREMENT_TEMPERATURE
 * CLASS2.MEASUREMENT_STR = 1040, Type=6, VSCP_TYPE_MEASUREMENT_TEMPERATURE
 * CLASS2.MEASUREMENT_FLOAT = 1060, Type=6, VSCP_TYPE_MEASUREMENT_TEMPERATURE

---

## Test 

use variable "max6675_setup"

```xml
<?xml version = "1.0" encoding = "UTF-8" ?>
<setup  guid="00:01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F"
        index="11" 
        zone="22" 
        subzone="33" >
    <max6675 enable="true" 
                spichannel="0" 
                unit="C" 
                detect-open="true"
                interval="1000" 
                vscpclass="10" 
                index="0" 
                zone="1" 
                subzone="2" />
</setup>
```

---

MIT license

Copyright 2018 [Ake Hedman, Grodans Paradis AB](akhe@grodansparadis.com)