# VSCP Level II driver for HD44780 based LCD connected via I2C to Raspberry Pi

![VSCP Logo](https://www.vscp.org/images/logo_200.png)

This driver interface one HD44780 based LCD connected over I2C interface on a Raspberry Pi. If you need hardware it can be found on [Ebay](https://www.ebay.com/itm/1PCS-1602-16x2-HD44780-Character-LCD-IIC-I2C-Serial-Interface-Adapter-Module-N/182350804137?hash=item2a74f474a9:g:nDYAAOSwcUBYJZYm:rk:13:pf:0) or [AliExpress](https://www.aliexpress.com/item/1602-16x2-HD44780-Character-LCD-w-IIC-I2C-Serial-Interface-Adapter-Module/32546958584.html?spm=2114.search0204.3.2.51252fd87N4EXx&ws_ab_test=searchweb0_0,searchweb201602_1_10065_10068_318_319_10546_317_10548_10696_450_10084_10083_10618_452_535_534_533_10307_10820_532_10303_204_10059_10884_323_10887_100031_320_10103_448_449,searchweb201603_60,ppcSwitch_0_ppcChannel&algo_expid=25e44b6d-6e38-40fd-9f0b-679449600c85-0&algo_pvid=25e44b6d-6e38-40fd-9f0b-679449600c85&transAbTest=ae803_5) or similar for a low cost.


## Building the driver
If you build the VSCP package from source and want to build this driver you should add the switch **--enable-rpi** to configure as

  ./configure --enable-rpi

 that will build the Raspberry Pi specific components and drivers for VSCP & Friends

## Enable I2C on the Raspberry Pi
Use **raspi-config** to enable the I2C interface hardware drivers.

Sparcfun have a [good tutorial]((https://learn.sparkfun.com/tutorials/raspberry-pi-spi-and-i2c-tutorial/all)) on both SPI and I2C and where you should connect the tour device.

## Driver configuration

```xml
<driver enable="true" >
    <name>driver-name</name>
    <path>path to driver</path>
    <config>Optional BINHEX64 coded config</config>
    <guid>00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00</guid>
</driver>
```

## XML Configuration

The configuration can be fetched from the driver configuration as BASE64 encoded XML data or from the server starting the driver from a XML variable **name_setup** where **name** is the driver-name used in the driver configuration above.

The XML configuration have the following format

```xml
<?xml version = "1.0" encoding = "UTF-8" ?>
<setup>
    <interface enable="true|false"
                bus="0|1"
                rows="2"
                width="16"
                display="true|flase"
                backlight="true|false"
                cursor-type="block|line"
                cursor="true|false"
                start-text="line0\n\line1\n..."
                auto="true|false >

        <format id="n" line="0" pos="0" template=""/>

        <page id="n"
                clear="true|false"
                format="n"
                line="n"
                row="n"
                wait="seconds" />

    </interface>
</setup>
```
There can be more than one <interface> tag but they must use different bus id's (0/1) and address combinations.

  * **enable** - Interface row is valid. Interface will not be used if disabled. "true"/"false"
  * **bus** - I2C channel to use. (0/1)
  * **address** - I2C address to use (default is 0x27).
  * **rows** - Number of rows for display (default is 2)
  * **width** - Display width (default is 16).
  * **display** - The display will be turned on/off from start. ("true"/"false")
  * **backlight** - Backlight will be on or off from start. ("true"/"false")
  * **cursor-type** - Set the cursor type. "line" or "block" cursor.
  * **cursor** - Cursor will be "on" or "off" from start.
  * **auto** - The auto mode show a number of pages in a round robin fashion. Pages (at least one) must be defined with the <page> tag.
  * **start-text** - Initial text om the LCD. This text can be given BASE64 encoded if preceded with BASE64:

### <format>

 * **id** Unique id or this format definition.
 * **line** Line to put cursor at. Empty for current pos.
 * **pos** Horizontal pos to put cursor at. Empty for current pos.
 * **template**  printf format template for data. Time and date

### <page>

The auto mode show a number of pages in a round robin fashion.

 * **id** - This is the sort order for the page.
 * **clear** - Clear display before printing.
 * **format** - This is the format id used. Leave out format or set to -1 for no format.
 * **row** - This is the row where the formated string is put.
 * **pos** - This is the pos where the formated string is put.
 * **wait** - Time in seconds that the information is shown. Can be zero. Typically used for update of several rows as a "page". In this case the wait is in the last update.

## Decision Matrix (DM)

The work for the driver is mostly done in the decision matrix. That is received event data is displayed when the data arrive or events trigger other actions.

### Actions

Actions are validated with CLASS1.INFORMATION, Type=32 (VSCP_TYPE_INFORMATION_ACTION_TRIGGER) event.

 * **noop** - No operation.
 * **measurement** - Print measurement value. Argument is template id. Measurements are always expressed as a double precision floating point number so the template should be designed according to that.
 * **text** - Output text. Argument text;line;pos  If line/pos is not give outputs at cursor.
 * **data** - Print event data. Argument is template id;data;....
 * **time** - Print time. Argument is template-id;line;pos
 * **date** - Print date. Argument is template-id;line;pos
 * **clear** - Clear screen.
 * **cursblkon** - Block cursor on.
 * **curscblkoff** - Block cursor off.
 * **curslineon** - Line cursor on.
 * **curslineoff** - Line cursor off.
 * **moveto** - Move to new location. Argument is *line;pos*
 * **shiftr** - Shift right. Argument is number of steps. Default is one step.
 * **shiftl** - Shift left. Argument is number of steps. Default is one step.
 * **putc** - Put character at position. Argument is *char;line;pos*
 * **blank** - Blank display.
 * **restore** - Restore display.
 * **blon** - Backlight on.
 * **bloff** - Backlight off.
 * **nextpg** - Go to next auto page.
 * **prevpg** - go to previous auto page.
 * **addpg** - Add an auto display page. Argument is page XML data *\<page\>* coded in BINHEX64-
 * **delpg** - Delete an auto display page. Arguent is page id.
 * **gopg** - Go to page n in an auto sequency. Argument is page.

### Print event data
Any event data can be printed.

 * **%int8_t[pos]** - Signed byte at *pos* on event data.
 * **%int16_t[pos]** - Signed integer starting at *pos* on event data. Big endian.
 * **%int32_t[pos]** - Signed long starting at *pos* on event data. Big endian.
 * **%uint8_t[pos]** -
 * **%uint16_t[pos]** - Unsigned integer starting at *pos* on event data. Big endian.
 * **%uint32_t[pos]** - Unsigned long starting at *pos* on event data. Big endian.
 * **%double[pos]** - Double starting at *pos* on event data. Big endian.
 * **float[pos]** - Float starting at *pos* on event data. Big endian.
 * **%bool[pos:bit]** - Boolean starting at *pos* on event data. A bit (0-7) can optionally be set. Outputs "true"/"false".
 * **%string[pos;length]** - String which starts and *pos* and have length characters. If length is not give the string is assumed to be zero termiantes.
 * **%char[pos]** - Character at *pos*.



### Example templates

"Time: 23:47     " --> "Time: HH:MM"
"Temp is 23.4C   " --> "Temp is %.2f"

The first template is handled by a time action and the second by a measurement.



---

MIT license

Copyright 2019 [Ake Hedman, Grodans Paradis AB](akhe@grodansparadis.com)