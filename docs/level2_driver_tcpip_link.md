# TCP/IP link level II driver

**Available for**: Linux, Windows

The tcp driver can be used to connect to other daemons or connect to limited TCP/IP devices that export the required minimum subset of commands needed. The required subset is described here [sub:Link-Commands] and here [sub:Available-tcpip-commands].

The driver will try to hold a connection open even if the remote node disconnects. This makes it possible to replace a node or take it down for maintenance and still have the link online again as soon as the node is powered up. 

**Driver Linux**: vscpl2_tcpdrv.so   
**Driver Windows**: vscpl2_tcpdrv.dll

The configuration string have the following format

    host;port;user;password;rxfilter;rxmask;txfilter;txmask;response-timeout

*Host* and *port* tells the address to the remote server. *User* and *password* is the credentials to log in to the remote machine. Filter and mask can be used to filter incoming traffic from the remote host. All of the parameters in the configuration string can also be set with variables.
The configuration string have the following format

 | Variable name    | Type    | Description  | 
 | -------------    | ----    | -----------  | 
 | _host_remote     | string  | IP address or a DNS resolvable address to the remote host. Mandatory and must be declared either in the configuration string or in this variable. | 
 | _port_remote     | integer | The port to use on the remote host. Default is 9598. | 
 | _user_remote     | string  | Username used to log in on the remote sever. | 
 | _password_remote | string  | Password used to login on the remote server. | 
 | _rxfilter          | string  | Standard VSCP filter in string form. 1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00 as priority,class,type,GUID Used to filter what events that is received from the remote host. If not given all events are received. | 
 | _rxmask            | string  | Standard VSCP mask in string form. 1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00 as priority,class,type,GUID Used to filter what events that is received from the remote host. If not given all events are received.   | 
 | _txfilter          | string  | Standard VSCP filter in string form. 1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00 as priority,class,type,GUID Used to filter what events that is sent to the remote host. If not given all events are received. | 
 | _txmask            | string  | Standard VSCP mask in string form. 1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00 as priority,class,type,GUID Used to filter what events that is sent to the remote host. If not given all events are received.   | 
  | _responsetimeout   | long  | Time in milliseconds we should wait for a response from the remote host.    |

The full variable name is built from the name you give the driver (prefix before _variablename) in vscpd.conf. So in the examples below the driver have the name **tcpiplink1** and the full variable name for the **_host_remote** will thus be

    tcpiplink1_host_remote

If you have another diver and name it  **tcpiplink2** it will therefore instead request variable **tcpiplink2_host_remote**

If your driver name contains spaces, for example “name of driver” it will get a prefix that is “name_of_driver”. Leading and trailing spaces will be removed. 

*_txmask*, *_txmask*, *_responsetimeout* is added in version 13.1.2. Also previously to this version _mask was used or *_rxmask* and *_filter* for *_rxfilter*. The old variants are deprecated but still works.

## XML configuration format
Form version 3.1.2 XML configuration strings are possible to use. The XML variable should be named xxxxx_setup where xxxxx is the driver name as of above.

```xml
<setup host="localhost"
         port="9598"
         user="admin"
         password="secret"
         rxfilter=""
         rxmask="" 
         txfilter=""
         txmask=""
         responsetimeout="2000" /> 
```

The configuration  is read in the following order

 * Configuration string.
 * Configuration variables.
 * XML configuration variable.

##### vscpd.conf example

```xml                
<driver enable="true" >
    <name>tcpiplink1</name>
    <path>/usr/local/lib/vscp2drv_tcpiplink.so</path>
    <config>192.168.1.20;9598;admin;secret</config>
    <guid>FF:FF:FF:FF:FF:FF:FF:FC:10:20:30:40:50:60:70:80</guid>
</driver>
```



[filename](./bottom_copyright.md ':include')
