# Configuring the VSCP Daemon

## Location of the configuration file

The daemon needs a configuration file called **vscpd.conf**. It will not start without this file nor if the file contains invalid information. Normally the installation program will create a default file.

On most machines it will be located in 

    /etc/vscp
    
The location that is searched to find the configuration file can be changed with switches when you start the daemon. see the section about [startup switches](http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_startup_switches) for each platform.

The configuration file have a path to a sql database file that store configuration setting among other things. Many configuration items are possible to set in both the xml file and in sql file. If a setting is present in both the setting in the xml file will be used. 

##  Description of the configuration :id=config-description

The configuration file is a standard XML file that contains information that tells the VSCP daemon what to do and how it should be done. The information in it is divided into sections and this documentation and each section and it's content is described below.

You can view a sample configuration file [here](Sample vscpd configuration file).

## The general section :id=config-general

In the general section you find settings that are common to all components of the VSCP daemon software. 

```xml
<general clientbuffersize="1024"
    runasuser="vscp"
    guid="FF:FF:FF:FF:FF:FF:FF:F5:00:00:00:00:00:00:00:01"
    servername="The VSCP daemon" />
```

### clientbuffersize

```xml
<clientbuffersize>1024</clientbuffersize>
```

This is the default buffer size (number of events) for all clients in the system. Everything from a driver to a TCP/IP user is regarded as a client. Default is 1024. 



### runasuser :id=config-general-runasuser

__Only on Unix/Linux__. User to run the VSCP daemon as.



### guid :id=config-gerneral-guid

```xml
<guid>
    FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:F5:00:00:00:00
</guid>
```

Set the server GUID for the daemon. This GUID is the base for all interface GUID's in the system. Interface GUID's is formed like xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:aa:aa:bb:bb where xx is guid set here, aa is the interface id and bb is nickname id's for devices on that interface. 

If not set here (or all nills) a GUID will be formed from the (first) MAC address of the machine the daemon runs on (on Linux needs to be run as root to get this) or if this fails the local IP address is used to form the GUID. The GUID string should be in MSB -> LSB order and have the four lsb set to zero.

Clients will be identified with a GUID that consist of this GUID plus an on the system unique number in the aa aa bytes. Devices will use this GUID as the source for events if no GUID is set for the driver or if it is set to all zeros.

**Example**
    FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:00:00:00:00

----

### Servername :id=config-general-servername

If set this real text name will be used as an identifier for the server along with the GUID. The default name will be something like

    VSCP Daemon @ FF:FF:FF:FF:FF:FF:FF:FE:00:26:55:CA:1F:DA:00:00


## security :id=config-security

```xml
<security admin="username" 
    password="450ADCE88F2FDBB20F3318B65E53CA4A;06D3311CC2195E80BE4F8EB12931BFEB5C630F6B154B2D644ABE29CEBDBFB545" 
    allowfrom="list of remotes" 
    vscptoken="Carpe diem quam minimum credula postero"
    vscpkey="/etc/vscp/certs/vscp.key"
    digest=""
/>
```

The admin user and other security settings can only be defined in the configuration file. It is __VERY important__ that this file only can be read by users that are allowed to do so.

###  admin :id=config-security-admin

Name of admin user. Default is "admin". **Use something else. PLEASE!**

### password :id=config-security-password

Password for admin user. Default is "secret". __Use something else. PLEASE!**  The password consist of a 16-byte salt and a 32 byte has on the form salt;hash. pbkdf2 is used for password handling with SHA256,

Use the **vscpmkpasswd** tool supplied with VSCP & Friends to generate the password (called mkpasswd in earlier versions).

Before version 1.12.26 the password stored here is formed by taking an MD5 over *username:authdomain:password*.

### allowfrom :id=config-security-allowfrom

A comma separated list (IPv4 and IPv6) of remote machines from which the admin user can log in. Wild-cards can be use. "192.168.1.*" means all machines on the 192.168.1.0 subnet. "*" means all remote machines and is the default.  

### vscptoken :id=config-security-vscptoken

This is used as a secret token (**key**) in traffic between server and client. Use any string you like and keep it secret. It should never be sent unencrypted over the wire.

### vscpkey :id=config-security-vscpkey

This entry points to a file containing a 256-bit, 32-byte, hexadecimal string that is used a the key for encryption/decryption. It can also be set as a command line switch for the VSCP server. Default is 

    A4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B633EF529D64544F8E
    
or 

```cpp  
uint8_t key[] = { 0xA4,0xA8,0x6F,0x7D,0x7E,0x11,0x9B,0xA3,
                   0xF0,0xCD,0x06,0x88,0x1E,0x37,0x1B,0x98,
                   0x9B,0x33,0xB6,0xD6,0x06,0xA8,0x63,0xB6,
                   0x33,0xEF,0x52,0x9D,0x64,0x54,0x4F,0x8E };
```

The **vscpkey** is normally located in the file /etc/vscp/certs/vscp.key. This folder should only be readable by the vscp user.

Read the [security information](./security_general.md) for more information about how the different values is used.

----

## debugflags :id=config-debugflags

Debugflags can be enabled to customize what is output to the syslog file. Beware that some (rx/tx) will output a lot of information to the log file. 

The debug flags are defined in eight bytes where each byte holds flags for a specific functionality.

```xml
<debugflags byte1="0"
            byte2="0"
            byte3="0"
            byte4="0"
            byte5="0"
            byte6="0"
            byte7="0"
            byte8="0"
/>
```

Set bits in the eight debug flag bytes that when set will output additional logging information,

| Byte | Bit | Description | 
| :---: | :---: | ----------- | 
| 0 | 0 | **extra general** debug info. |
| 0 | 1 | **automation** debug info. |
| 0 | 2 | **configuration** debug info. |
| 1 | 0 | **tcp/ip** interface debug info. |
| 1 | 1 | **tcp/ip receive** debug info. |
| 1 | 2 | **tcp/ip transmit** debug info. |
| 2 | 0 | **web server** interface debug info. |
| 2 | 1 | **REST** interface debug info. |
| 2 | 2 | **webserver access** debug info. |
| 3 | 0 | **websocket** interface debug info. |
| 3 | 1 | **websocket receive** debug info. |
| 3 | 2 | **websocket transmit** debug info. |
| 4 | 0 | **level 1 driver** debug info. |
| 4 | 1 | **level 2 driver** debug info. |
| 4 | 2 | **level 1 driver receive** debug info. |
| 4 | 3 | **level 1 driver transmit** debug info. |
| 4 | 4 | **level 2 driver receive** debug info. |
| 4 | 5 | **level 2 driver transmit** debug info. |



##  tcp/ip client interface :id=config-tcpip

```xml
<tcpif interface=”ip-address:port” 
       encryption=""
       ssl_certificate="/srv/vscp/certs/tcpip_server.pem"
       ssl_certificate_chain=""
       ssl_verify_peer="false"
       ssl_ca_path=""
       ssl_ca_file=""
       ssl_verify_depth="9"
       ssl_default_verify_paths="true"
       ssl_cipher_list="DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256"
       ssl_protocol_version="3"
       ssl_short_trust="false" 
/>
```

Set the port and interface(s) the TCP/IP client should be listening on.

**Examples**
```xml
<tcpip interface="9598"/>
```

###   enable

*deprecated in 1.12.6 now always enabled* \\  
Enable/disable the VSCP daemon TCP/IP interface. Default is enabled. 

###   interface

**From version 13.1**

Interfaces should be set as a comma separated list of ports to listen on. The format for each item is

    ip-address:port[s]

but

    port[s]

can be used and then the localhost will be used as ip-address.

Adding "s" to the end of the port will sett up a secure port. SSL parameters is needed in this case.
    
Default is "9598".

**Before version 13.1**

Set one or more addresses separated with space the interface to bind to on a machine with multiple interfaces. If no address (just a port) is given the TCP/IP interface will be available on all interfaces (this is the default). If an address is given the daemon listens only on that interface or if several is listen on all listed interfaces . Default is “9598” meaning the TCP/IP client will be available on all interfaces on the machine. For SSL, server certificate must be specified as
 
   ssl://[ip:]port:server_cert.pem

here server_cert.pem is the full path including the filename to the cert. To enable client certificate authentication (two-way SSL), a CA certificate should be specified: 

   ssl://[ip:]port:server_cert.pem:ca_cert.pem 

where both server_cert.pem and ca_cert.pem are full paths including filename to the certificates. Server certificate must be in pem format. Pem file should contain both certificate and the private key concatenated together.

###  encryption

Normally the tcp/ip communication is in clear text (if not using ssl/tls). It is however possible to use AES encryption for the tcp/ip interface. This is a way for lower end systems, that don't have the full resources to implement ssl/tls, to add security to the connection. Encryption can have four values

 | Value          | Description                | 
 | :-----:          | -----------                | 
 | Blank/no value | Clear text is used.        | 
 | aes128         | AES128 encryption is used. | 
 | aes192         | AES192 encryption is used. | 
 | aes256         | AES256 encryption is used. | 

It is an overhead for this use that increases the higher AES key length that is used. 

The key is set in the security settings in the XML file.

Default is no encryption.


###   ssl_certificate

Valid from version 13.1

Path to SSL certificate file. This option is only required when at least one of the listening\_ports is SSL The file must be in PEM format, and it must have both private key and certificate, see for example [ssl_cert.pem](https://github.com/civetweb/civetweb/blob/master/resources/ssl_cert.pem). If this option is set, then the webserver serves SSL connections on the port set up to listen on. 

**Default:** /srv/vscp/certs/server.pem


### ssl_certificate_chain

Valid from version 13.1

T.B.D.


### ssl_verify_peer

Valid from version 13.1

Enable client's certificate verification by the server.

**Default:** false

### ssl_ca_path

Valid from version 13.1

Name of a directory containing trusted CA certificates for peers. Each file in the directory must contain only a single CA certificate. The files must be named by the subject name’s hash and an extension of “.0”. If there is more than one certificate with the same subject name they should have extensions ".0", ".1", ".2" and so on respectively.


### ssl_ca_file

Valid from version 13.1

Path to a .pem file containing trusted certificates for peers. The file may contain more than one certificate.

### ssl_verify_depth

Valid from version 13.1

Sets maximum depth of certificate chain. If client's certificate chain is longer than the depth set here connection is refused.

**Default:** 9


### ssl_default_verify_paths

Valid from version 13.1

Loads default trusted certificates locations set at openssl compile time. 

**Default:** true




### ssl_cipher_list

Valid from version 13.1

List of ciphers to present to the client. Entries should be separated by colons, commas or spaces.

 | Selection   | Description                         | 
 | ---------   | -----------                         | 
 | ALL         | All available ciphers               | 
 | ALL:!eNULL  | All ciphers excluding NULL ciphers  | 
 | AES128:!MD5 | AES 128 with digests other than MD5 | 

See [this entry in OpenSSL documentation](https://www.openssl.org/docs/manmaster/apps/ciphers.html) for full list of options and additional examples.




### ssl_protocol_version

Valid from version 13.1

Sets the minimal accepted version of SSL/TLS protocol according to the table:

 | Selected protocols             | setting | 
 | ------------------             | :-------: | 
 | SSL2+SSL3+TLS1.0+TLS1.1+TLS1.2 | 0       | 
 | SSL3+TLS1.0+TLS1.1+TLS1.2      | 1       | 
 | TLS1.0+TLS1.1+TLS1.2           | 2       | 
 | TLS1.1+TLS1.2                  | 3       | 
 | TLS1.2                         | 4       | 

Default = 4.

### ssl_short_trust

Enables the use of short lived certificates. This will allow for the certificates and keys specified in ssl_certificate, ssl_ca_file and ssl_ca_path to be exchanged and reloaded while the server is running.

In an automated environment it is advised to first write the new pem file to a different filename and then to rename it to the configured pem file name to increase performance while swapping the certificate.

Disk IO performance can be improved when keeping the certificates and keys stored on a tmpfs (linux) on a system with very high throughput.

**Default:** false


##  udp interface :id=config-udp

The UDP interface, if enabled, consist of two parts. The first is the receiving interface which receive UDP datagrams from remote clients. The second is one or many clients that receive UDP datagrams from the VSCP daemon. 

```xml
<udp enable="true|false" 
    interface=”[udp://]ip-address:port” 
    encryption="none|aes128|aes192|aes256"
    user="udp"
    bAllowUnsecure="false"
    bSendAck="true"
    filter=""
    mask=""
    guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00" >

    <!-- Nodes that will receive VSCP Server events -->
    <client enable="false"
        interface="udp://127.0.0.1:9999"
        filter=""
        mask=""
        encryption="none|aes128|aes192|aes256"
        bSetBroadcast="false"
    />
</udp>
```

### enable

Enable the udp functionality. If disabled both receiving and sending of udp datagrams will be disabled.

### interface

This is the interface on which the VSCP daemon will listen for udp datagrams. It should be specified on the form

   [udp://]ip-addr:port

The "udp://" can be left out.

**ip-addr** is the interface on the server to bind to. If left out all interfaces will be listened on.

**port** is the port used. Port defaults to 44444 and can be left out.

If left empty. The udp server will listen on port 44444 on all interfaces.

If just a port is specified the udp server will listen on that port on all interfaces.

### encryption

This is the encryption level that incoming frames must be encoded with to be accepted. 

Can be set to

| Setting | Encryption |
| :-----: | ---------- |
| none | No encyption |
| aes128 | Content in frame is encrypted with AES128 encryption. |
| aes192 | Content in frame is encrypted with AES192 encryption. |
| aes256 | Content in frame is encrypted with AES256 encryption. |

### user

Specify the user account that the udp server is tied to. This allow for remote ip-checks and event checks among other things.

### bAllowUnsecure

Set to true to allow unencrypted frames.

### bSendAck
Send ACK/NACK udp frames to clients sending frames to udp server.

### filter
Filter for received events on string form. 

> priority;class;type;guid

### mask
Filter for received events on string form.

> priority;class;type;guid

### guid
GUID to use for udp interface.

## client

Clients are nodes that receive UDP datagrams from the VSCP daemon. You can have as many udp clients defined as you like.

### enable
Enable the client when set to true. This means that the sever will send events to the client.

### interface

This is the address on which the client is expected to listen for udp datagrams. It should be specified on the form

   [udp://]ip-addr:port

The "udp://" can be left out.

**ip-addr** is the ipo.address for the client. It must be specified.

**port** is the port used. Port defaults to 44444 and can be left out.

### filter
Filter for sent events on string form. 

> priority;class;type;guid

### mask
Filter for sent events on string form.

> priority;class;type;guid

### encryption

This is the encryption level that sent frames will be encoded with. 

Can be set to

| Setting | Encryption |
| :-----: | ---------- |
| none | No encyption |
| aes128 | Content in frame is encrypted with AES128 encryption. |
| aes192 | Content in frame is encrypted with AES192 encryption. |
| aes256 | Content in frame is encrypted with AES256 encryption. |

### bSetBroadcast
The broadcast bit of the datagram will be set if set to true. Default is false.


##   webserver :id=config-webserver

```xml
<webserver enable="true" 
              document_root="/srv/vscp/www"
              listening_ports="[::]:8888r,[::]:8843s,8884"
              index_files="index.xhtml,index.html,index.htm,index.lp,index.lsp,index.lua,index.cgi,index.shtml,index.php"
              authentication_domain="mydomain.com"
              enable_auth_domain_check="false"
              ssl_certificate="/srv/vscp/serts/server.pem"
              ssl_certificate_chain=""
              ssl_verify_peer="false"
              ssl_ca_path=""
              ssl_ca_file=""
              ssl_verify_depth="9"
              ssl_default_verify_paths="true"
              ssl_cipher_list="DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256"
              ssl_protocol_version="3"
              ssl_short_trust="false"
              cgi_interpreter=""
              cgi_patterns="**.cgi$|**.pl$|**.php|**.py" 
              cgi_environment=""
              protect_uri=""
              trottle=""
              enable_directory_listing="true"
              enable_keep_alive="false"
              keep_alive_timeout_ms="0"
              access_control_list=""
              extra_mime_types=""
              num_threads="50"
              run_as_user=""
              url_rewrite_patterns=""
              hilde_file_patterns=""
              request_timeout_ms="10000"
              linger_timeout_ms=""
              decode_url="true"
              global_auth_file=""
              per_directory_auth_file=""
              ssi_patterns=""
              access_control_allow_origin="*"
              access_control_allow_methods="*"
              access_control_allow_headers="*"
              error_pages=""
              tcp_nodelay="0"
              static_file_max_age="3600"
              strict_transport_security_max_age=""
              allow_sendfile_call="true"
              additional_header=""
              max_request_size="16384"
              allow_index_script_resource="false"
              duktape_script_patterns="**.ssjs$"
              lua_preload_file=""
              lua_script_patterns="**.lua$"
              lua_server_page_patterns="**.lp$|**.lsp$"
              lua_websocket_patterns="**.lua$"
              lua_background_script=""
              lua_background_script_params=""
</webserver>
```




###   enable

Enable or disable the web server functionality.

**Default:** true




###   document_root

Path to webserver root filesystem. The daemon should have read/write permissions to this area. Webserver and websocket interface look for content here. Defaults to 

    /programdata/vscp/www” 

on windows and to

    /srv/vscp/www” 

on Unix/Linux.




###   listening_ports

Comma-separated list of ports to listen on. If the port is SSL, a letter s must be appended, for example, *80,443s* will open port 80 and port 443, and connections on port 443 will be SSL-ed. For non-SSL ports, it is allowed to append letter r, meaning 'redirect'. Redirect ports will redirect all their traffic to the first configured SSL port. For example, if listening_ports is 80r,443s, then all HTTP traffic coming at port 80 will be redirected to HTTPS port 443.

It is possible to specify an IP address to bind to. In this case, an IP address and a colon must be pre-pended to the port number. For example, to bind to a loopback interface on port 80 and to all interfaces on HTTPS port 443, use 127.0.0.1:80,443s.

If the server is built with IPv6 support, [::]:8884 can be used to listen to IPv6 connections to port 8080. IPv6 addresses of network interfaces can be specified as well, e.g. [::1]:8884 for the IPv6 loopback interface.

[::]:80 will bind to port 80 IPv6 only. In order to use port 80 for all interfaces, both IPv4 and IPv6, use either the configuration 80,[::]:80 (create one socket for IPv4 and one for IPv6 only), or +80 (create one socket for both, IPv4 and IPv6). The +-notation to use IPv4 and IPv6 will only work in no network interface is specified. Depending on your operating system version and IPv6 network environment, some configurations might not work as expected, so you have to test to find the configuration most suitable for your needs. In case +80 does not work for your environment, you need to use 80,[::]:80.

It is possible to use network interface addresses (e.g., 192.0.2.3:80, [2001:0db8::1234]:80). To get a list of available network interface addresses, use ipconfig (in a cmd window in Windows) or ifconfig (in a Linux shell). Alternatively, you could use the hostname for an interface. Check the hosts file of your operating system for a proper hostname (for Windows, usually found in C:\Windows\System32\drivers\etc, for most Linux distributions: /etc/hosts). E.g., to bind the IPv6 local host, you could use ip6-localhost:80. This translates to [::1]:80. Beside the hosts file, there are several other name resolution services. Using your hostname might bind you to the localhost or an external interface. You could also try hostname.local, if the proper network services are installed (Zeroconf, mDNS, Bonjour, Avahi). When using a hostname, you need to test in your particular network environment - in some cases, you might need to resort to a fixed IP address.

**Default:** [::]:8888r,[::]:8843s,8884

### index_files

Comma-separated list of files to be treated as directory index files. If more than one matching file is present in a directory, the one listed to the left is used as a directory index.

**Default:** index.xhtml,index.html,index.htm,index.lp,index.lsp,index.lua,index.cgi,index.shtml,index.php




###   authentication_domain

Authorization realm used for HTTP digest authentication. This domain is used in the encoding of the .htpasswd authorization files as well. Changing the domain retroactively will render the existing passwords useless.

**Default:** “mydomain.com” 


###   enable_auth_domain_check

When using absolute URLs, verify the host is identical to the authentication_domain. If enabled, requests to absolute URLs will only be processed if they are directed to the domain. If disabled, absolute URLs to any host will be accepted. Set to "true" (**default**) to enable, "false" to disable.
URL encoded request strings are decoded in the server, unless it is disabled by setting this option to no.



###   ssl_certificate

Path to SSL certificate file. This option is only required when at least one of the listening\_ports is SSL The file must be in PEM format, and it must have both private key and certificate, see for example [ssl_cert.pem](https://github.com/civetweb/civetweb/blob/master/resources/ssl_cert.pem). If this option is set, then the webserver serves SSL connections on the port set up to listen on. 

**Default:** /srv/vscp/certs/server.pem




### ssl_certificate_chain

T.B.D.




### ssl_verify_peer

Enable client's certificate verification by the server.

**Default:** false

### ssl_ca_path

Name of a directory containing trusted CA certificates for peers. Each file in the directory must contain only a single CA certificate. The files must be named by the subject name’s hash and an extension of “.0”. If there is more than one certificate with the same subject name they should have extensions ".0", ".1", ".2" and so on respectively.




### ssl_ca_file

Path to a .pem file containing trusted certificates for peers. The file may contain more than one certificate.

### ssl_verify_depth

Sets maximum depth of certificate chain. If client's certificate chain is longer than the depth set here connection is refused.

**Default:** 9




### ssl_default_verify_paths

Loads default trusted certificates locations set at openssl compile time. 

**Default:** true




### ssl_cipher_list

List of ciphers to present to the client. Entries should be separated by colons, commas or spaces.

 | Selection   | Description                         | 
 | ---------   | -----------                         | 
 | ALL         | All available ciphers               | 
 | ALL:!eNULL  | All ciphers excluding NULL ciphers  | 
 | AES128:!MD5 | AES 128 with digests other than MD5 | 

See [this entry in OpenSSL documentation](https://www.openssl.org/docs/manmaster/apps/ciphers.html) for full list of options and additional examples.




### ssl_protocol_version

Sets the minimal accepted version of SSL/TLS protocol according to the table:

 | Selected protocols             | setting | 
 | ------------------             | :-------: | 
 | SSL2+SSL3+TLS1.0+TLS1.1+TLS1.2 | 0       | 
 | SSL3+TLS1.0+TLS1.1+TLS1.2      | 1       | 
 | TLS1.0+TLS1.1+TLS1.2           | 2       | 
 | TLS1.1+TLS1.2                  | 3       | 
 | TLS1.2                         | 4       | 

Default = 4.

### ssl_short_trust

Enables the use of short lived certificates. This will allow for the certificates and keys specified in ssl_certificate, ssl_ca_file and ssl_ca_path to be exchanged and reloaded while the server is running.

In an automated environment it is advised to first write the new pem file to a different filename and then to rename it to the configured pem file name to increase performance while swapping the certificate.

Disk IO performance can be improved when keeping the certificates and keys stored on a tmpfs (linux) on a system with very high throughput.

**Default:** false





###   cgi_interpreter

Path to an executable to be used use as an interpreter for all CGI scripts regardless script extension. Default: not set, the webserver looks at [shebang line "#!"](https://en.wikipedia.org/wiki/Shebang_(Unix%5C)) in the first line of the script .

For example, if both PHP and perl CGIs are used, then #!/path/to/php-cgi.exe and #!/path/to/perl.exe must be first lines of the respective CGI scripts. Note that paths should be either full file paths, or file paths relative to the directory where mongoose executable is located.

If all CGIs use the same interpreter, for example they are all PHP, then cgi_interpreter option can be set to the path to php-cgi.exe executable and shebang line in the CGI scripts can be omitted. Note: PHP scripts must use php-cgi.exe, not php.exe.



###   cgi_patterns

All files that match cgi_pattern are treated as CGI files. Default pattern allows CGI files to be anywhere. To restrict CGIs to a certain directory, use **/path/to/cgi-bin/\*\*.cgi** as a pattern. Note that full file path is matched against the pattern, not the URI.

When the webserver start a CGI program, it creates new environment for it (in contrast, usually child program inherits the environment from parent). Several environment variables however are inherited from the webservers environment, they are: PATH, TMP, TEMP, TMPDIR, PERLLIB, MONGOOSE_CGI. 

On UNIX it is also LD_LIBRARY_PATH. On Windows it is also COMSPEC, SYSTEMROOT, SystemDrive, ProgramFiles, ProgramFiles(x86), CommonProgramFiles(x86).

**Default:**  \*\*.cgi$|\*\*.pl$|\*\*.php|\*\*.py|\*\*.lua `

There is no requirement for CGI scripts to be located in a special directory. CGI scripts can be anywhere. CGI (and SSI) files are recognized by the file name pattern. Shell-like glob patterns is used. Pattern match starts at the beginning of the string, so essentially patterns are prefix patterns. Syntax is as follows:
    
 | Pattern | Description |
 | :-------: | ----------- |
 | \*  | Matches everything but slash character, `<nowiki>`/`</nowiki>` |
 | ?   | Matches any character |
 | $   | Matches the end of the string |
 | \|  | Matches if pattern on the left side or the right side matches. | 

All other characters in the pattern match themselves. Examples:

 | Pattern | Description |
 | :-------:    | ----------- |
 | \*\*.cgi$ | Any string that ends with .cgi |
 | /foo    | Any string that begins with /foo |
 | \*\*a$\|\*\*b$  | Any string that ends with a or b |




###   cgi_environment

This option set extra variables to be passed to the CGI script in addition to the standard environment variables. The list of variables must be a comma separated list of **name=value** pairs like this: 

    VARIABLE1=VALUE1,VARIABLE2=VALUE2. 




###  protect_uri

This is a comma separated list of URI=PATH pairs, specifying that given URIs must be protected with password files specified by PATH. All Paths must be full file paths.




###  trottle

Limit download speed for clients. throttle is a comma-separated list of key=value pairs, where key could be:

 | key  | Description | 
 | :---: | ----------- | 
 | \* | limit speed for all connections  | 
 | x.x.x.x/mask       | limit speed for specified subnet | 
 | uri_prefix_pattern | limit speed for given URIs       | 

The value is a floating-point number of bytes per second, optionally followed by a k or m character, meaning kilobytes and megabytes respectively. A limit of 0 means unlimited rate. The last matching rule wins. Examples:

| Pattern | Description |
| :-------: | ---------------- |
| \*=1k,10.0.0.0/8=0 | limit all accesses to 1 kilobyte per second, but give connections the from 10.0.0.0/8 subnet unlimited speed | 
| /downloads/=5k | limit accesses to all URIs in `/downloads/` to 5 kilobytes per second. All other accesses are unlimited      | 

Not set by default.




###   enable_directory_listing

Enable directory listings. 

**Default:** true.




###   enable_keep_alive

Enable connection keep alive.

Allows clients to reuse TCP connection for subsequent HTTP requests, which improves performance. For this to work when using request handlers it is important to add the correct Content-Length HTTP header for each request. If this is forgotten the client will time out.

Note: If you set keep_alive to true, you should set keep_alive_timeout_ms to some value > 0 (e.g. 500). If you set keep_alive to no, you should set keep_alive_timeout_ms to 0. Currently, this is done as a default value, but this configuration is redundant. In a future version, the keep_alive configuration option might be removed and automatically set to true if a timeout > 0 is set.

**Default:** false


### keep_alive_timeout_ms

Idle timeout between two requests in one keep-alive connection. If keep alive is enabled, multiple requests using the same connection are possible. This reduces the overhead for opening and closing connections when loading several resources from one server, but it also blocks one port and one thread at the server during the lifetime of this connection. Unfortunately, browsers do not close the keep-alive connection after loading all resources required to show a website. The server closes a keep-alive connection, if there is no additional request from the client during this timeout.

Note: if enable_keep_alive is set to no the value of keep_alive_timeout_ms should be set to 0, if enable_keep_alive is set to yes, the value of keep_alive_timeout_ms must be >0. Currently keep_alive_timeout_ms is ignored if enable_keep_alive is no, but future versions my drop the enable_keep_alive configuration value and automatically use keep-alive if keep_alive_timeout_ms is not 0.




### access_control_list

IP ACL. By default, empty or nondefined, meaning all IPs are allowed to connect.

An Access Control List (ACL) allows restrictions to be put on the list of IP addresses which have access to the web server. The ACL is a comma separated list of IP subnets, where each subnet is pre-pended by either a - or a + sign. A plus sign means allow, where a minus sign means deny. If a subnet mask is omitted, such as -1.2.3.4, this means to deny only that single IP address.

Subnet masks may vary from 0 to 32, inclusive. The default setting is to allow all accesses. On each request the full list is traversed, and the last match wins. Examples:

    "-0.0.0.0/0,+192.168/16     deny all accesses, 
                                only allow 192.168/16 subnet"

Subnet masks are [described here](https://en.wikipedia.org/wiki/Subnetwork).




###   extra_mime_types

Extra mime types to recognize, in form 

   extension1=type1,extension2=type2,.... 

Extension must include a leading dot. 

**Example:**  
   .cpp=plain/text,.java=plain/text. 


See the [Wikipedia page on Internet media types](https://en.wikipedia.org/wiki/Internet_media_type).
   

**Default:** not set.




### num_threads

Number of worker threads. The web server handles each incoming connection in a separate thread. Therefore, the value of this option is effectively the number of concurrent HTTP connections the web server can handle.

**Default:** 50




### run_as_user

Switch to given user credentials after startup. Usually, this option is required when the web server needs to bind on privileged ports on UNIX. To do that, the web server needs to be started as root. From a security point of view, running as root is not advisable, therefore this option can be used to drop privileges. 





### hide_file_patterns

A pattern for the files to hide. Files that match the pattern will not show up in directory listing and return 404 Not Found if requested. Pattern must be for a file name only, not including directory names. 

Example:

    secret.txt|**.hide

**Note:** hide_file_patterns uses the pattern described above. If you want to hide all files with a certain extension, make sure to use `<nowiki>`.extension`</nowiki>` (not just *.extension).




###   url_rewrite_patterns

Comma-separated list of URL rewrites in the form of uri_pattern=file_or_directory_path. When the webserver receives any request, it constructs the file name to show by combining document_root and the URI. However, if the rewrite option is used and uri_pattern matches the requested URI, then document_root is ignored. Instead, file_or_directory_path is used, which should be a full path name or a path relative to the web server's current working directory. Note that uri_pattern, as all patterns, is a prefix pattern.

This makes it possible to serve many directories outside from document_root, redirect all requests to scripts, and do other tricky things. For example, to redirect all accesses to .doc files to a special script, set url_rewrite_patters to

     "**.doc$=/path/to/cgi-bin/handle_doc.cgi"

or, to imitate support for user home directories, set them to

    /~joe/=/home/joe/,/~bill=/home/bill/




### global_auth_file

Leave as NULL to disable authentication. Normally, only selected directories in the document root are protected. If absolutely every access to the web server needs to be authenticated, regardless of the URI, set this option to the path to the passwords file. Format of that file is the same as ".htpasswd" file. Make sure that file is located outside document root to prevent people fetching it.

The path to the global passwords file can either be a full path or relative path to the current working directory. If set, per-directory .htpasswd files are ignored, and all requests are authorized against that file.

The file has to include the realm set through authentication_domain and the password in digest format:

    user:realm:digest
    test:test.com:ce0220efc2dd2fad6185e1f1af5a4327

[This online tolls](https://www.askapache.com/online-tools/htpasswd-generator) can be used to generate password files.




### per_directory_auth_file

Leave as NULL to disable authentication. To enable directory protection with authentication, set this to ".htpasswd". Then, creating ".htpasswd" file in any directory automatically protects it with digest authentication. Use `mongoose` web server binary, or `htdigest` Apache utility to create/manipulate passwords file. Make sure `auth_domain` is set to a valid domain name.




### ssi_patterns

All files that match ssi_pattern are treated as Server Side Includes (SSI).

SSI is a simple interpreted server-side scripting language which is most commonly used to include the contents of another file into a web page. It can be useful when it is desirable to include a common piece of code throughout a website, for example, headers and footers.

In order for a webpage to recognize an SSI-enabled HTML file, the filename should end with a special extension, by default the extension should be either .shtml or .shtm. These extensions may be changed using the ssi_pattern option.

Unknown SSI directives are silently ignored. Currently, two SSI directives are supported, 
    `<!--#include ...>` 
and 
    `<!--#exec "command">`. 

Note that the *`<!--#include ...>`* directive supports three path specifications:

   `<!--#include virtual="path">`  Path is relative to web server root
   `<!--#include abspath="path">`  Path is absolute or relative to
                                 web server working dir
   `<!--#include file="path">`,    Path is relative to current document
   `<!--#include "path">`

The include directive may be used to include the contents of a file or the result of running a CGI script. The exec directive is used to execute a command on a server, and show the output that would have been printed to stdout (the terminal window) otherwise. Example:

   `<!--#exec "ls -l" -->`

If not set, "**.shtml$|**.shtm$" is used.   

Wikipedia article about [SSI](https://sv.wikipedia.org/wiki/Server_Side_Includes) file pattern. 




### request_timeout_ms

Timeout for network read and network write operations, in milliseconds. If a client intends to keep long-running connection, either increase this value or (better) use keep-alive messages.




### linger_timeout_ms

Set TCP socket linger timeout before closing sockets (SO_LINGER option). The configured value is a timeout in milliseconds. Setting the value to 0 will yield in abortive close (if the socket is closed from the server side). Setting the value to -1 will turn off linger. If the value is not set (or set to -2), the web server will not set the linger option at all.

Note: For consistency with other timeouts, the value is configured in milliseconds. However, the TCP socket layer usually only offers a timeout in seconds, so the value should be an integer multiple of 1000.




### decode_url

URL encoded request strings are decoded in the server, unless it is disabled by setting this option to false.

**Default:** true 




### access_control_allow_origin

Access-Control-Allow-Origin header field, used for cross-origin resource sharing (CORS). See the [Wikipedia page on CORS](https://en.wikipedia.org/wiki/Cross-origin_resource_sharing) for aditional information.

**Default:** *




### access_control_allow_methods

Access-Control-Allow-Methods header field, used for cross-origin resource sharing (CORS) pre-flight requests. [See the Wikipedia page on CORS](https://en.wikipedia.org/wiki/Cross-origin_resource_sharing) for aditional information.

If set to an empty string, pre-flights will not be supported directly by the server, but scripts may still support pre-flights by handling the OPTIONS method properly. If set to "*", the pre-flight will allow whatever method has been requested. If set to a comma separated list of valid HTTP methods, the pre-flight will return exactly this list as allowed method. If set in any other way, the result is unspecified.

**Default:** *




### access_control_allow_headers

Access-Control-Allow-Headers header field, used for cross-origin resource sharing (CORS) pre-flight requests. [See the Wikipedia page on CORS](https://en.wikipedia.org/wiki/Cross-origin_resource_sharing) for aditional information.

If set to an empty string, pre-flights will not allow additional headers. If set to "*", the pre-flight will allow whatever headers have been requested. If set to a comma separated list of valid HTTP headers, the pre-flight will return exactly this list as allowed headers. If set in any other way, the result is unspecified.

**Default:** *




### error_pages

This option may be used to specify a directory for user defined error pages. The error pages may be specified for an individual http status code (e.g., 404 - page requested by the client not found), a group of http status codes (e.g., 4xx - all client errors) or all errors. The corresponding error pages must be called error404.ext, error4xx.ext or error.ext, whereas the file extension may be one of the extensions specified for the index_files option. [See this Wikipedia page on HTTP status codes](https://en.wikipedia.org/wiki/HTTP_status_code) for additional information.




### tcp_nodelay

Enable TCP_NODELAY socket option on client connections.

If set the socket option will disable Nagle's algorithm on the connection which means that packets will be sent as soon as possible instead of waiting for a full buffer or timeout to occur.

 | Setting | Description                                 | 
 | :-------: | -----------                               | 
 | 0       | Keep the default: Nagel's algorithm enabled | 
 | 1       | Disable Nagel's algorithm for all sockets   | 


**Default:** 0




### static_file_max_age

Set the maximum time (in seconds) a cache may store a static files.

This option will set the header field
   Cache-Control: max-age value 
for static files. Dynamically generated content, i.e., content created by a script or callback, must send cache control headers by themselves.

A value >0 corresponds to a maximum allowed caching time in seconds. This value should not exceed one year ([RFC 2616](https://www.ietf.org/rfc/rfc2616.txt), Section 14.21). A value of 0 will send "do not cache" headers for all static files. For values `<0 and values >`31622400, the behavior is undefined.

**Default:** 3600




### strict_transport_security_max_age

Set the Strict-Transport-Security header, and set the max-age value. This instructs web browsers to interact with the server only using HTTPS, never by HTTP. If set, it will be sent for every request handled directly by the server, except scripts (CGI, Lua, ..) and callbacks. They must send HTTP headers on their own.

The time is specified in seconds. If this configuration is not set, or set to -1, no Strict-Transport-Security header will be sent. For values `<-1 and values >`31622400, the behavior is undefined.




### allow_sendfile_call

This option can be used to enable or disable the use of the Linux sendfile system call. It is only available for Linux systems and only affecting HTTP (not HTTPS) connections if throttle is not enabled. While using the sendfile call will lead to a performance boost for HTTP connections, this call may be broken for some file systems and some operating system versions.

**Default:** true




### additional_header

Send additional HTTP response header line for every request. The full header line including key and value must be specified, excluding the carriage return line feed.

Example (used as command line option): -additional_header "X-Frame-Options: SAMEORIGIN"




### max_request_size

Maximum request size URL.

**Default:** 16384




### allow_index_script_resource

Index scripts (like index.cgi or index.lua) may have script handled resources.

It this feature is activated, that 

    /some/path/file.ext 
    
 might be handled by:

1. /some/path/file.ext (with PATH_INFO='/', if ext = cgi)
2. /some/path/index.lua with mg.request_info.path_info='/file.ext'
3. /some/path/index.cgi with PATH_INFO='/file.ext'
4. /some/path/index.php with PATH_INFO='/file.ext'
5. /some/index.lua with mg.request_info.path_info=='/path/file.ext'
6. /some/index.cgi with PATH_INFO='/path/file.ext'
7. /some/index.php with PATH_INFO='/path/file.ext'
8. /index.lua with mg.request_info.path_info=='/some/path/file.ext'
9. /index.cgi with PATH_INFO='/some/path/file.ext'
10. /index.php with PATH_INFO='/some/path/file.ext'

Note: This example is valid, if the default configuration values for index_files, cgi_pattern and lua_script_pattern are used, and the server is built with CGI and Lua support enabled.

If this feature is not activated, only the first file (/some/path/file.cgi) will be accepted.

Note: This parameter affects only index scripts. A path like /here/script.cgi/handle/this.ext will call /here/script.cgi with PATH_INFO='/handle/this.ext', no matter if this option is set to yes or no.

This feature can be used to completely hide the script extension from the URL.

**Default:** false




### duktape_script_pattern

A pattern for files that are interpreted as JavaScripts by the server. 

**Default:** "\*\*.ssjs$"

### lua_preload_file

This configuration option can be used to specify a Lua script file, which is executed before the actual web page script (Lua script, Lua server page or Lua websocket). It can be used to modify the Lua environment of all web page scripts, e.g., by loading additional libraries or defining functions required by all scripts. It may be used to achieve backward compatibility by defining obsolete functions as well.




### lua_script_patterns

A pattern for files that are interpreted as Lua scripts by the server. In contrast to Lua server pages, Lua scripts use plain Lua syntax. 

**Default:** "\*\*.lua$"




### lua_server_page_patterns

Files matching this pattern are treated as Lua server pages. In contrast to Lua scripts, the content of a Lua server pages is delivered directly to the client. Lua script parts are delimited from the standard content by including them between tags. 

**Default:** "\*\*.lp$|\*\*.lsp$"




### lua_websocket_patterns

**Default:** "\*\*.lua$"




### lua_background_script

**Experimental feature**, and subject to change. 

Run a Lua script in the background, independent from any connection. The script is started before network access to the server is available. It can be used to prepare the document root (e.g., update files, compress files, ...), check for external resources, remove old log files, etc.

The Lua state remains open until the server is stopped. In the future, some callback functions will be available to notify the script on changes of the server state. See example lua script : [background.lua](https://github.com/civetweb/civetweb/blob/master/test/background.lua).

Additional functions available in background script : *sleep, root path, script name, isterminated*




### lua_background_script_params

Used to add dynamic parameters to background script. Parameters mapped to global 'mg' table 'params' field.

##   websockets 

```xml
<websockets enable="true" 
    document_root="/var/vscp/web"
    timeout_ms="30000"
/>
```

Set **enable** to true to enable the interface and false to disable. 

 Both **enable** and **auth** is deprecated from version 12.29.1.13




### document_root

Document root for websockets. Since websockets use a different URL scheme (ws, wss) than other http pages (http, https), the Lua scripts used for websockets may also be served from a different directory. By default, the document_root is used as websocket_root as well.

**Default:** /srv/vscp/web




### timeout_ms

Timeout for websocket calls.

**Default:** 30000



----



## Remote user settings :id=config-remote-user

Example

```xml
<remoteuser>
    <user name="mrsmith"                    
                password="0F7AB661663B5BCA3B9B725F933B39A0;1F1D138C273A3FBB66A2CFB9120512F8BC825F88B9C1AEF7091679237515CCF2"
                privilege="user"
                allowfrom=""
                filter=""
                events=""
                fullname="Sample user"
                note="A normal user. username="user" password='secret'"
        />
    <user name="udp"                
                password="BCE76E44226D1DEC47E9E3C5C6BEDBE1;A3A546E00E13B60A1F5A6AAA92B78251925E79A43253CD2AE56D43109CD1B138"
                privilege="udp"
                allowfrom=""
                filter=""
                events=""
                fullname="UDP user"
                note="A normal user. username="user" password='secret'"
        />
</remoteuser>
```

In the *\<remoteuser\>** tab are settings that affect which users that can access the daemon through the TCP/IP interface, web interface, rest-interface and through the websocket inteface. Each user is defined between a *\<user\>…\</user\>* pair. 

##   user

```xml
<name>user</name>
```

Username for this user.

###  password

```xml
<password>password-hash</password>
```

The password hash for the user. This hash should be the md5 over "username:vscptoken:password". The **mkpasswd**  utility that is distributed with the VSCP & Friends package can be user to generate the password. There are also plenty of services on the net that can be used. One example is [here](http://www.md5online.org/md5-encrypt.html).  

The vscptoken is set in the `<securitytab>` and it should be kept secret and never be sent on it's own over the wire.

###  privilege

```xml
<privilege>level</privilege>
```

Privilege defines what a user can do. This is 32-bits defining different privileges.

This value can either be set from a 32-bit value or as a forward slahs '/' separated list of values where each item is either a numerical value or a token (See token definitions below).

| Bit | Priviledge |
| :---: | -------------- |
| 0 | User can log in on tcp/ip interface. |
| 1 | User can log in on websocket interface (ws1/ws2). |
| 2 | User can log in on web interface. |
| 3 | User can log in on rest interface. |
| 4 | User can log in on udp interface. |
| 5 | User can log in on mqtt interface. |
| 6 | Reserved. |
| 7 | Reserved. |
| 8 | Reserved. |
| 9 | Reserved. |
| 10 | Reserved. |
| 11 | Reserved. |
| 12 | Reserved. |
| 13 | Reserved. |
| 14 | Reserved. |
| 15 | Reserved. |
| 16 | User allowed to send events. |
| 17 | User allowed to send Level I protocol events. |
| 18 | User allowed to send Level II protocol events. |
| 19 | User allowed to send HLO events. |
| 20 | User can receive events. |
| 21 | Reserved. |
| 22 | Reserved. |
| 23 | Reserved. |
| 24 | Reserved. |
| 25 | Reserved. |
| 26 | User can set filters. |
| 27 | User can set interface GUID. |
| 28 | User allowed to use 'test' command. |
| 29 | User allowed to use 'interface' command. |
| 30 | User allowed to use 'restart' command. |
| 31 | User allowed to use 'shutdown' command. |

There are some shortcut btokens defined

| Token | Priviledge |
| :---: | -------------- |
| admin | This user is allowed to do anything. |
| user | This user is allowed to use tcp/ip, websockets, webinterface, rest, udp, mqtt and also to send and receive events and level I and send level II protocol events. |
| driver | This user is allowed to send and receive events and level I and send level II protocol events and HLO events. |
| tcp | This user is allowed to login on the tcp/ip interface and send and receive events and level I and send level II protocol events and HLO events. |
| websocket | This user is allowed to login on the websockets (ws1/ws2) interface and send and receive events and level I and send level II protocol events and HLO events. |
| web | This user is allowed to login on the web interface and send and receive events and level I and send level II protocol events and HLO events. |
| rest | This user is allowed to login on the rest interface and send and receive events and level I and send level II protocol events and HLO events. |
| udp | This user is allowed to login on the udp interface and send and receive events and level I and send level II protocol events and HLO events. |
| mqtt | This user is allowed to login on the mqtt interface and send and receive events and level I and send level II protocol events and HLO events. |
| send-events | This user is allowed to send events. |
| receive-events | This user is allowed to receive events. |
| l1ctrl-events | This user is allowed to send level I protocol events. |
| l2ctrl-events | This user is allowed to send level II protocol events. |
| hlo-events | This user is allowed to send HLO events. |
| shutdown | This user is allowed to use the 'shutdown' command. |
| restart | This user is allowed to use the 'restart' command. |
| interface | This user is allowed to use the 'interface' command. |
| test | This user is allowed to use the 'test' command. |

###  allowfrom

```xml
<allowfrom>comma-seperated-list</allowfrom>
```

This is a comma separated list with host addresses this user is allowed to log in to the server from. Wild cards can be used to indicate “all”. This mean that

    "*.*.*.*"

all users can log in from all remote machines. While

    "194.*.*.*"

means that he/she can log on from all remote machines with a IP addresses that starts with 194 and son on. Default is all so if you want to allow everyone to access your server you can skip this tag or leave it blank. 

###  allowevent

```xml
<allowevent>comma-seperated-list</allowevent>
```

This is a comma separated list of events this user is allowed to send. The form is

    class:type,class:type

wild-cards (*)can be used for both. This means that 20:*,30:* means that the user is allowed to send all events in class=20 and class=30. Default allows user to send all events. "*.*" means the user is allowed to send anything. And "20:1,20:2,20:10" that the user is allowed to send event with Class=20 and Type=1 and Class=20 and Type=2 and Class=20 and Type=10

### mask

```xml
<mask priority="xxx" class="xxx" type="xxx" GUID="xxx" />
```

Set the mask for incoming events to a client. The mask has a bit set for a binary digit that is of interest. Default is all is null == disabled. It is important that filter and mask is both set if they are used.

### filter
```xml
<filter priority="xxx" class="xxx" type="xxx" GUID="xxx" />
```

This is the event filter. Set the filter for incoming events to a client. The form is If a mask bit is set to one for a position then the filter bit must be equal to the bit of the incoming event for the client to get it. It is important that filter and mask is both set if they are used.


----
##  Level I Drivers :id=config-level1-driver

Enable/disable daemon Level I driver interface. If disabled no Level I drivers will be loaded. Default is enabled. This is the driver type that in the past used to be called CANAL (CAN Abstraction Layer) drivers.

```xml
<level1driver enable=”true|false” />
```

The content consist of one or several driver entries each representing a level 1 driver that should be used. 

### enable

Set to true to enable level I driver functionality , set to false to disable. This is convenient if one wants to disable all level I drivers without removing them from the configuration file.

### driver

```xml
<driver enable="false">
```

A level I driver entry.

### enable

The driver should be loaded if set to true. If set to false the driver will not be loaded.

### name

```xml
<name>driver-name</name>
```

A name given by the user for the driver. This is the name by which the driver is identified by the system.

### config

```xml
<config>config1;config2</config>
```

The semicolon separated configuration string for the driver. The meaning of this string is driver specific.

### flags

```xml
<flags>0</flags>
```

A 32 bit driver specific number that have bits set that have special meaning for a driver. See the documentation for a specific driver for an explanation of the meaning of each flag bit for that driver.

### path

```xml
<path>path-to-driver</path>
```

Path to where the driver is located. A dll file on Windows or a .so dl file on Linux etc.

### guid

```xml
<guid>guid-for-driver</guid>
```

The GUID that the driver will use for it's interface. This means that this is the GUID that will be the source of Level I events but only if the GUID is not set to all zeros in which case the generated interface GUID from the general section will be used as the base for the GUID. 

Note that the GUID set here will have the two least significant bytes replaced by the node id so when set here they should be set to zero.

### Translation

`<translation>` is a list of semicolon separated fields with translations that should be carried out by the system on event passing through driver.

 | Mnemonic  | Description  | 
 | :--------:  | -----------  | 
 | Bit 1 (1) | Incoming Level I measurement events is translated to Level II measurement, floating point events. | 
 | Bit 2 (2) | Incoming Level I measurement events is translated to Level II measurement, string events.  | 
 | Bit 3 (4) | All incoming events will be translated to class Level I events over Level II, that is have 512 added to it's class. | 

** Example **

```xml
<leve1driver enable="true" >

    <!-- The level I logger driver  -->
    <driver enable="false">
        <name>logger</name>
        <config>/tmp/canallog.txt</config>
        <path>/ust/local/lib/canallogger.so</path>
        <flags>1</flags>
        <guid>00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00</guid>
        <translate>2</translate>
    </driver>

    <!-- The can232 driver -->
    <driver enable="false" >
        <name>can232</name>
        <config>/dev/ttyS0;19200;0;0;125</config>
        <path>/usr/local/lib/can232drv.so</path>
        <guid>00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00</guid>
        <flags>0</flags>
    </driver>

    <!-- The xap driver is documented  -->
    <driver enable="false" >
        <name>xap</name>
        <config>9598;3639</config>
        <path>/usr/local/lib/xapdrv.so</path>
        <flags>0</flags>
        <guid>00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00</guid>
    </driver>

</leve1driver>
```

----

## Level II drivers :id=config-level2-driver

Level II drivers can handle the full VSCP abstraction and don't have the limitation of the Level I drivers. 


```xml
<leve2driver enable=”true|false” />
```

Enable/disable daemon Level II driver interface. If disabled no Level II drivers will be loaded. Default is enabled. 


```xml
`<driver enable="false">`
```

The content consist of one or several driver entries each representing a driver that should be used. 

### enable

The driver should be loaded if set to true. If false the driver will not be loaded.

### name

```xml
`<name>`driver-name`</name>`
```

A name given by the user for the driver.

### config

```xml
`<config>`config1;config2`</config>`
```

The semi colon separated configuration string for the driver. The meaning of this string is driver specific.

###  guid

```xml
`<guid>`guid-for-driver`</guid>`
```

The GUID that the driver will use for it's interface.

### translation

See explanation for Level I drivers above.

### known-nodes

See explanation for Level I drivers above.

**Example**

```xml
<level2driver enable="true|false">

    <driver enable="true|false" >
        <name>test</name>		
        <path>path_to_driver`</path>
        <config>param2;param2;param3;...</config>
        <guid>00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00</guid>
        <known-nodes>
            <node   guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01" name="Known node1" />
            <node guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:02" name="Known node2" />
            <node guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:03" name="Known node3" />
            <node guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:04" name="Known node4" />
        </known-nodes>
    </driver>

</level2driver>
```


[filename](./bottom_copyright.md ':include')
