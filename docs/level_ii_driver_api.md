# Level II Driver API

Level II drivers are drivers that can be used to extend the VSCP server capabilities and be used as an alternative driver approach for devices. They can connect to different type of hardware. Level II drivers has two advantages over Level I drivers. 


*  They always use the Level II event format which means that the full GUID is used. 

*  They can interface the daemon through the full TCP/IP interface giving a lot of possibilities for the driver to interact with the daemon.

The ability to use the full GUID is good as there is no need for translation schema's between the actual GUID and GUID's used in interfaces. The node-ID is unique all over the world.

Letting the driver talk to the daemon over the TCP/IP interface is favorable in that it can do many things that previously has been impossible. The most exciting is that it can read and write variables (even create new ones if needed). This is the recommended way to use for configurations of a Level II driver. It means that configuration of all drivers can be made in one place (the daemon variable file), it gives a possibility to change run time values in real time etc.

The level II driver is, just as the Level I driver, a dynamic link library with a specific set of exported methods. The exported methods are four of the methods from the CANAL interface and uses identical calling parameters and return values. There are some differences however noted below. 

The configuration for a typical VSCP driver in the vscpd.conf file looks like this

```xml
<vscpdriver>  <!-- Information about VSCP Level II drivers -->
    <driver prefix="logger1" >
        <name>l2logger</name>
        <config>d:\vscplog.txt;1</config>
        <path>
            /us/local/lib/vscpl2logger.so
        </path>
        <guid>00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00</guid>
    </driver>
</vscpdriver>
```

The driver name is used when the driver is presented to the world such as in the tcp/ip interface. The name should be unique within the system. The prefix is added in front of variable names the driver fetch or write to. Typically variables have names like “_variable” and the actual variable fetches is thus “prefix_variable”. “prefix” is set to the driver name set in the VSCP daemon configuration file for this driver. If the same driver is used for several times this name should be different for each driver if different configuration data should be read in for the drivers. The prefix is used in front of configuration variables and it will be used on the form prefix_variablename when the driver reads in additional driver configuration data.

In the parameter a list with semicolon separated values can be stored. Each entry represent a specific configuration value. Even if they are present the driver will try fetch the corresponding value from the daemon and if it is found it will replace the value set in the parameters list. The path is the location where the driver dynamic link library is located. Guid is the guid used for the interface. If absent or set to all zeros the daemon assigned guid will be used for the interface. 



Events received by the daemon from a Level II driver use the GUID set in the event if not all nills. If it has all nills the device GUID will be used if it is not all nills and if not the interface GUID will be used.

A smaller variant of the driver (Limited Level II driver) is available which is constructed as an alternative for Level I drivers. This driver does not use the TCP/IP interface of the daemon.

# VSCPOpen

```c
long VSCPOpen( const char *pUsername,
                  const char *pPassword, 
                  const char *pHost, 
                  short port, 
                  const char *pPrefix, 
                  const char *pConfiguration, 
                  unsigned long flags );
```

Start the driver and give it some initial configuration data. Before the driver is started by the daemon one parameters is added in front of the configuration string by the daemon. pPrefix is a user defined string that is used as a prefix for variables read/write/construct. This can be an empty string but it is recommended to set a value so that the same driver can be used and configured for different things at the same time. Typically this string has the form “mydriver” or something like that. Variables used to configure this particular driver then need to use the same prefix. It is up to the driver to decide if the prefix should be used or not. 

pUsername and pPassword is either a random generated pair of credentials constructed by the daemon or set from the configuration file.

pHost and port is either set from the configuration file or pHost is set to “localhost” by the daemon and port is set to the VSCP port 9598.

pPrefix is set to the driver name fetched from the daemon configuration file. 

pConfiguration is the normally semicolon separated list of configuration values supplied by the user. 

The flags parameter is not used at the moment.

If the driver can initialize and is started it will return a handle for open physical interface or < = 0 on error. For an interface where there is only one channel the handle has no special meaning and can only be looked upon as a status return parameter. 

## Limited Level II Driver

For the limited type of driver pUsername, pPassword, pHost, port, pPrefix all is set to NULL. pConfigure holds the configuration of the driver along with flags.

# VSCPClose

```c
int VSCPClose( long handle );
```


If the driver can close/stop the driver it will return TRUE (non zero) on success or FALSE on failure. 

# VSCPBlockingSend

```c
int VSCPBlockingSend( long handle, 
                        const vscpEvent *pEvent, 
                        unsigned long timeout );
```


Send an event while blocking for timeout (forever if timeout=0). 

# VSCPBlockingReceive

```c
int VSCPBlockingReceive( long handle, 
                           vscpEvent *pEvent, 
                           unsigned long timeout );
```


Blocking receive of an event. Blocks for timeout and forever if timeout=0. 

CANAL_ERROR_SUCCESS is received if an event is received and CANAL_ERROR_TIMEOUT on timeout.

# VSCPGetLevel

```c
unsigned long VSCPGetLevel( long handle ) 
```


Will return CANAL_LEVEL_USES_TCPIP for a full Level II driver and CANAL_LEVEL_NO_TCPIP for a limited Level II driver. The constants is defined in canal.h May return something else in the future if the driver is extended.

# VSCPGetWebPageTemplate

Only available for a full Level II driver, limited drivers just return from the call without any internal action.

```c
long VSCPGetWebPageTemplate( long handle, 
                                const char *url, 
                                char **ppage )
```


With this method it is possible for a driver to add internal web server functionality. Pages with an address 

    /vscp/drivername/url 

will be recognized as a driver page and therefore redirected to this method of the the driver expect a pointer to a webpage in return or NULL for a non recognized url. Important! If a pointer is returned it is the calling program that should de allocate the data returned. 

If you want to make it possible to get information from or configure a driver this is a way to do it. Pages that posts data will have there data scanned by the internal web-server of the daemon and found data values is written as strings to server variables (see [sub:VARIABLE]) which are constructed if they are not already defined.

Before a page is sent to a user escapes as defined in the table below are replaced with current values

 | Escape  | Description             | 
 | ------  | -----------             | 
 | %server | Address for the server. | 
 | %date   | ISO formated date.      | 
 | %time   | ISO formated time.      | 

The standard menu will be shown on top of each page.

# VSCPGetWebPageInfo

Only available for a full Level II driver, limited drivers just return from the call without any internal action.

```c
int VSCPGetWebPageInfo( long handle, 
                          const struct vscpextwebpageinfo *info )
```


This call can be used to get information about what pages the driver will recognize abd a description of what they do. This information is displayed in the web interface. The structure that should be returned is defined in vscp/src/common/vscpdlldef.h 

# VSCPWebPageupdate

Only available for a full Level II driver, limited drivers just return from the call without any internal action.

```c
int VSCPWebPageupdate( long handle, const char *url )
```


When a page is posted to a driver url variable data will be written to server variables by the server. If variables are undefined they will be created. After this has been done a call to this method will be carried out.

# VSCPGetDllVersion

```c
unsigned long VSCPGetDllVersion ( void )
```


Gets the version for the driver dll/dl.

# VSCPGetVendorString

```c
const char *VSCPGetVendorString ( void )
```


Get a string that identifies the creator of the driver.

# VSCPGetDriverInfo 

```c
const char * VSCPGetDriverInfo( void )
```


Get information about the interface. Either NULL if there is no information or a pointer to a string with XML information.

```xml
<?xml version = "1.0" encoding = "UTF-8" ?>

<!-- Version 0.0.2     2009-11-17    
    "string"    Text string. 
    "bool"      1 bit number specified as true or false.
    "char"      8 bit number. Hexadecimal if it starts 
                with "0x" else decimal.
    "uchar"     Unsigned 8  bit number. Hexadecimal if it 
                starts with "0x" else decimal.
    "short"     16 bit signed number. Hexadecimal if it 
                starts with "0x" else decimal.
    "ushort"    16 bit unsigned number. Hexadecimal if it 
                starts with "0x" else decimal.
    "int"       32 bit signed number. Hexadecimal if it 
                starts with "0x" else decimal. 
    "uint"      32 bit unsigned number. Hexadecimal if it 
                starts with "0x" else decimal. 
    "long"      64 bit signed number. Hexadecimal if it 
                starts with "0x" 
				else decimal. 
    "ulong"     64 bit unsigned number. Hexadecimal if it 
                starts with "0x" 
			    else decimal.
    "decimal"   128 bit number. Hexadecimal if it starts 
                with "0x" else decimal.  
    "date"      Must be passed in the format dd-mmm-yyyy.
    "time"      Must be passed in the format hh:mm:ss 
                where hh is 24 hour clock.   
-->   

<vscpdriver>
    <drivername>aaaaaaaa</drivername>
    <arch>WIN32|WIN64|LINUX</arch>
    <model>bbbbb</model>
    <version>cccccc</version>
    <description lang ="en">
        yyyyyyyyyyyyyyyyyyyyyyyyyyyy
    </description>`   
	
    <!-- Site with info about the product -->
    <infourl>`http://www.somewhere.com`</infourl>
	
    <!-- Information about CANAL driver maker -->
    <maker>
        <name>tttttttttttttttttttt</name>
        <address>
            <street>ttttttttttttt</street>
            <town>llllllllll</town>
            <city>London</city>
            <postcode>HH1234</postcode>
            <!-- Use region or state -->
            <state>sssss</state>
            <region>rrrrr</region>      
            <country>ttttt</country>
        </address>
		
        <!-- One or many -->
        <telephone>     
            <number>123456789</number>
            <description lang="en" >
                Main Reception
            </description>
        </telephone>
		
        <!-- One or many -->
        <fax>
            <number>1234567879</number>
            <description lang="en">
                Main Fax Number
            </description>
        </fax>
		
        <!-- One or many -->
        <email>someone@somwhere.com</email>
		
        <!-- One or many -->   
	    <web>www.somewhere.com</web> 

    </maker>     
	
	<configstring>     

        <!-- Option example with selectable values -->
        <option pos="0" type="string">
            <name lang="en">`aaaaa`</name>
            <description lang="en">
                yyy
            </description>
            <valuelist>                    
                <item value="CANUSB" />
                    <name lang="en">
                        USB Adapter</name>
                    <description lang="en">
                        yyy
                    </description>
                </item>
                        
		        <item value="CANPCI" />         
                    <name lang="en">
                        PCI Adapter
                    </name>
                    <description lang="en">
                        yyy
                    </description>
                </item>
                          
                <item value="CAN232" />
                    <name lang="en">
                        Serial Adapter
                    </name>
                    <description lang="en">
                        yyy
                    </description>
                </item>
            </valuelist>
                
        </option>
		
        <!-- Option example with numerical value -->
        <option pos="1" 
                type="uchar" 
                min="0" 
                max="4">
            <name lang="en">aaaaa</name>
            <description lang="en">
                yyy
            </description>
        </option>
             
    </configstring>
		
    <!-- Flags part (32-bit value) of the device information -->
    <flags>
        <description lang="en">yyy</description>
                 
        <bit pos="0">
            <name lang="en">tttt</name>
            <description lang="en">
                yyy
            </description>
        </bit>
             
        <!-- example for bit groups, in this case 
             bit 1,2,3,4 
        -->
        <bit pos="1" width="4">
            <name lang="en">`tttt`</name>
            <description lang="en">
                yyy
            </description>`   
        </bit>
             
    </flags>
		
    <!-- Status return value (32-bit value) -->
    <status>
        <bit pos="0">
            <name lang="en">tttt</name>
                <description lang="en">
                    yyy
                </description>
        </bit>

        <bit pos="1" width="4">
            <name lang="en">tttt</name>
            <description lang="en">
                yyy
            </description>
        </bit>
            
    </status>

    <!-- Description of variable the driver reads in -->
    <variable>
        <name>ksdkjskjskdjksdjks</name>
        <type>bool|string|etc etc</type>
        <description>ksksjdksjkdjs</description>
    </variable>

</vscpdriver>
```


# VSCPGetStatus

```c
const char * VSCPGetStatus( unsigned long* pErrorCode )
```


Get status information from a device in real text or as a code. Set the pErrorCode variable to NULL if no error code is needed. The errocode CANAL_ERROR_SUCCESS and “OK” is returned if everything is OK and there is no error code waiting to be received. If another code is returned other codes may be queued up and can be received in successive reads. For severe errors and states, such as bus off on a can-bus, the state code should be returned until the state is cleared and the devices is working properly again.



[filename](./bottom_copyright.md ':include')

