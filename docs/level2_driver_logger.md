# Logger level II driver

**Available for**: Linux, Windows

The logger driver can be used to log events to a file. Two formats of the log file is supported. Either standard log file with a standard text string for each event on each line or in XML file format which can be read by VSCP works and further analyzed there.

**Driver Linux**: vscpl2_loggerdrv.so
**Driver Windows**: vscpl2_loggerdrv.dll

The **configuration string** have the following format

    path;rewrite;vscpworksfmt;filter;mask

See the table below for a description. Variables fetched from the daemon is used if they are available even if the configuration string specify a value.

##### Variables fetched from the VSCP demon configuration file.

 | Variable name | Type   | Description  | 
 | ------------- | ----   | -----------  | 
 | _path         | string | Path to the logfile.                                                                                                                                                                                               | 
 | _rewrite      | bool   | Set to “true” to rewrite the file each time the driver is started. Set to “false” to append to file.                                                                                                       | 
 | _vscpworksfmt | bool   | If “true” VSCP works XML format will be used for the log file. This means that the file will be possible to read and further analyzed by VSCP Works. If “false” a standard text based format will be used. | 
 | _filter       | string | Standard VSCP filter in string from. *1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00* as *priority,class,type,GUID*                                                                           | 
 | _mask         | string | Standard VSCP mask in string form. *1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00* as *priority,class,type,GUID*                                                                             | 

The full variable name is built from the name you give the driver (prefix before _variablename) in vscpd.conf. So in the examples below the driver have the name **log1** and the full variable name for the **_path** variable will therefore be

    log1_path 

If you have another diver and name it  **log2** it will therefore instead request variable **log1_path**. 

If your driver name contains spaces, for example "name of driver" it will get a prefix that is "name_of_driver". Leading and trailing spaces will be removed.

##### Example of vscpd.conf entry for the logger driver.

```xml
<driver enable="true" >
    <name>log1</name>
    <path>/usr/local/lib/vscpl2_loggerdrv.so</path>  
    <config>/tmp/vscp_level2.log</config>           
    <guid>00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00</guid>
</driver>
```

[filename](./bottom_copyright.md ':include')
