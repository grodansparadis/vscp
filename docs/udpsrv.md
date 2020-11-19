# UDP - General description

## General

UDP communication is simple and easy to set up but also insecure in the way that you do not know if a datagram is received or not. This is normally not a problem for VSCP as there is almost always a way to determine that an event actually has been received. A typical example is a lamp that should be turned on. The controller typically send

> **CLASS1.CONTROL, Type=5 ,TurnOn**

we then expect the receiving node to reply with

> **CLASS1.INFORMATION, Type=3, On**

to verify that the lamp was turned on. It is therefore easy to make sure a command has been received by a remote node and has been carried out and if not resend.

The VSCP server can be setup to receive UDP datagrams on a specific port and it can be setup to send VSCP events to a specified number of nodes. Filters can be used to limit sent and received events. It is possible to get an automatic acknowledgement event when a VSCP UDP event is received by the VSCP server. Encryption is available in the form of AES128/AES192/AES256 or unencrypted frames.
Security

One can send unencrypted frames to the VSCP server. The problem with this is that anyone can send a VSCP event to a central VSCP server/system. This will of course be very dangerous on most setups. To improve on this you can specify a user that the UDP sub system will be acting as and this gives some possibilities to limit what can be sent and be received over the UDP interface. Best is to use encrypted frames. Here the remote node and the server share a secret key and therefore they can trust each other using state of the art encryption. This also makes it impossible for someone that is not authorized to see what is sent on the wire or over the air.

VSCP UDP frame format is specified in the VSCP specification [here](https://docs.vscp.org/spec/latest/#/./vscp_over_udp).

## How does UDP work on the VSCP daemon

The VSCP UDP subsystem consist of two parts a server and clients that are totally separated from each other. You can therefore just decide to receive UDP frames or just to send them or both.

### Server

If enabled the VSCP daemon accept UDP frames from clients. Frames can be be plain or encrypted. You must configure what type if frames your setup accept. A specific user is tied to udp so you can define which client to accept events from etc. The UDP server can ACL/NACk incoming frames.

By default the server listens on port 44444.

When an encrypted frame is received it will be encrypted with the vscpkey a secret 128-bit key that is common to server and client.

### Clients

You can define as many udp clients as you want. An udp client is a receiving udp remote node that can accept UDP frames that is either plain or encrypted. This node will receive events just as any other client. A filter can be setup. Encryption can be none/aes128/aes192/aes256.

When an encrypted frame is sent it will be encrypted with the vscpkey a secret 128-bit key that is common to server and client.

## Configuration

The configuration of the UDP server is in the standard vscpd configuration file (in XML format) which usually us located at **/etc/vscp/vscpd.conf**. THe format for the section is like this

```xml
<udp enable="true"
    interface="udp://44444"
    user=""
    password=""
    bAllowUnsecure="true"
    bSendAck="true"
    filter=""
    mask=""
    guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00" >

    <!-- 
        Nodes that will receive VSCP Server events 
        Any number of clients can be defined.
    -->
    <client enable="false"
        filter=""
        mask=""
        encryption=""
        bSetBroadcast="false"
    />
</udp>
```

The configuration consist of two parts.

* The receiving UDP port and the functionality for it.
* Configuration for receiving UDP nodes.

### Configuring the receiving UDP port

#### enable
Enable enables the UDP interface if set to **true**. This applies for receiving nodes also. Set to **false** to disable VSCP UDP functionality.

#### interface
The second thing to configure is the interface to use. This is typically __udp://44444__ which says "listen on all interfaces on this machine and use port 44444 (default)". One can use an IP address like udp//:192.168.1.45:44444 to listen only on a specific interface.

#### user / password
Next you should consider using a user account that UDP connections use. You do this by setting the username in user and the password in password. Password should be set as salt;password in the standard way. By doing this you can specify which events that can be received and from which IP addresses. Leave both user and password blank to not connect to a specific user account.

#### bAllowUnsecure
The next thing is to decide if you should allow unencrypted frames. You do this by setting bAllowUnsecure to **true**. If set to **false** only encrypted frames will be received.

#### bSendAck
With bSendAck you can decide if the node sending an UDP frame should get an acknowledge event back. If set to //"true"// a CLASS1.ERROR, Type=0, Success event will be replied on success and a CLASS1.ERROR, Type=1, Error is sent if something is wrong with the UDP frame. If set no //"false"// no acknowledgement frames will be sent.

#### filter/mask
By setting the filter/mask you can filter out unwanted events. Both filter and mask is set as text strings on the form **"priority;class;type;guid"**

#### guid
With guid you set the GUID for the interface. If empty or all set to zeros the system will set a GUID for you.

### Configuring receiving UDP nodes

Receiving UDP nodes are nodes that will get events from the VSCP server. You can specify as many nodes as you need.

#### enable
Enable enables the UDP receiving node if set to **true**. Set to **false** to disable VSCP UDP functionality.

#### interface
interface is the address and port of the receiving node. It should be entered on the form udp//:192.168.1.45:29001

#### filter/mask
By setting the filter/mask you can filter out events that should not be sent to this node. Both filter and mask is set as text strings on the form "priority;class;type;guid"

#### encryption
Encryption tells how the sent UDP frames should be encrypted. Valid values are "none"/"aes128"/"aes192"/"aes256" and empty

## Sample code

You can find some sample/test code [here](https://github.com/grodansparadis/vscp/tree/master/tests/udp).