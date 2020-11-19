# Bluetooth proximity Level II driver

**Available for:** Windows

The Bluetooth proximity driver can be used to detect Bluetooth devices. It can send out the ID of the device using [CLASS1.INFORMATION=20, Type=37](http://docs.vscp.org/spec/latest/#/./class2.level1.information1?id=type37-0x25-token-activity) when it enters/leave the detection zone and also a a detect event [CLASS1.INFOMATION=20, Type=49](http://docs.vscp.org/spec/latest/#/./class2.level1.information1?id=type49-0x31-detect) if configured to do so.

A call of method **VSCPBlockingSend** will always return **CANAL_ERROR_NOT_SUPPORTED** as this driver just receive events.

The driver is always using the GUID of the interface.

A demo is available here on [youtube](https://youtu.be/CrtfJhWrDIU). 

**Driver Windows**: *vscpl2_btproximitydrv.dll*

The **configuration string** is not used.

Variables fetched from the VSCP demon configuration file. Defaults will be used if the variable is not available.

 | Variable name | Type    | Default    | Description | 
 | ------------- | ----    | :-------:    | -----------  | 
 | _pausetime            | integer | One second | Pause in seconds between detection attempts.                                         | 
 | _zone                 | integer | 0          | Zone to use for token events.                                                        | 
 | _subzone              | integer | 0          | Sub-zone to use for token events.                                                    | 
 | _detectindex          | integer | 0          | Index to use for detect events.                                                      | 
 | _detectzone           | integer | 0          | Zone to use for detect events.                                                       | 
 | _detextsubzone        | integer | 0          | Sub-zone to use for detect events.                                                   | 
 | _send_token_activity  | boolean | true       | Set to true to enable token events.                                                  | 
 | _send_detect          | boolean | false      | Set to true to enable detect events.                                                 | 
 | _disable_radio_detect | boolean | false      | Normally also the radio unit is detected. Set this variable to true to disable this. | 

The full variable name is built from the name you give the driver (prefix before _variablename) in vscpd.conf. So in the examples below the driver have the name **btprox1** and the full variable name for the **_paustime* will thus be

    btproxy1_pausetime

If you have another diver and name it  **btproxy2** it will therefore instead request variable **btproxy1_paustime**

If your driver name contains spaces, for example "name of driver" it will get a prefix that is "name_of_driver". Leading and trailing spaces will be removed.


##### Example of vscpd.conf entry for the driver.

```xml
<driver enable="true" >
    <name>btprox1</name>
    <path>c:\vscpl2_btproximitydrv.dll</path>     
    <config></config>
    <guid>`FF:FF:FF:FF:FF:FF:FF:F7:01:00:00:00:00:00:00:00</guid>
</driver> 
```


##### Example of variable declarations to configure driver

```xml
<!-- 
This node is a Bluetooth proximity sensor driver that send out the identification id when a
Bluetooth device is within range. The id is the Bluetooth id of the device.
-->
<variable type="integer">
    <name>btprox1_pausetime</name>
    <note>Pause in seconds between detection attempts.</note>
    <value>1</value>
</variable>

<variable type="integer">
    <name>btprox1_zone</name>
    <note>Zone to use for token events.</note>
    <value>2</value>
</variable>

<variable type="integer">
    <name>btprox1_subzone</name>
    <note>Sub-zone to use for token events.</note>
    <value>1</value>
</variable>
    
<variable type="integer">
    <name>btprox1_detectindex</name>
    <note>Index to use for detect events.</note>
    <value>0</value>
</variable>
    
<variable type="integer">
    <name>btprox1_detectzone</name>
    <note>Zone to use for detect events.</note>
    <value>3</value>
</variable>

<variable type="integer">
    <name>btprox1_detectsubzone</name>
    <note>Sub-zone to use for detect events.</note>
    <value>1</value>
</variable>
    
<variable type="boolean">
    <name>btprox1_send_token_activity</name>
    <note>Set to true to enable token events.</note>
    <value>true</value>
</variable>
    
<variable type="boolean">
    <name>btprox1_send_detect</name>
    <note>Set to true to enable detect events.</note>
    <value>true</value>
</variable>
    
<variable type="boolean">
    <name>btprox1_disable_radio_detect</name>
    <note>Normally also the radio unit is detected. Set this variable to true to disable this.</note>
    <value>false</value>
</variable>

<variable type="guid">
    <name>btprox1_guid</name>
    <note>GUID for the driver.</note>
    <value>FF:FF:FF:FF:FF:FF:FF:F7:01:00:00:00:00:00:00:00</value>
</variable>
```


[filename](./bottom_copyright.md ':include')
