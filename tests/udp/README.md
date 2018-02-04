This is some test programs to test the UDP connections of the VSCP Server and other Level II VSCP nodes.

The format for VSCP over UDP is described <a href="http://www.vscp.org/docs/vscpspec/doku.php?id=physical_level_lower_level_protocols#vscp_over_udp">here</a>

The VSCP UDP of the VSCP server is <a href="http://www.vscp.org/docs/vscpd/doku.php?id=daemon_udp_protocol_description_general">described here</a>

<h2>To build test programs</h2>
Issue <b>make</b> to build. Will work on a Debian system at least.

<h2>send_vscp_udp_frame</h2>

Format: <b>send_vscp_udp_event</b> host port packettype count bReply

Send VSCP udp event to a Level II UDP node. All parameters are optional and defaults to <b>host="127.0.0.1", <b>port</b>=33333, <b>pakettype</b>=0 <b>count</b>=1 <b>bReply</b>=0.

<h4>host</h4>
The IP address of the host where the VSCP UDP interface is located.

<h4>port</h4>
The port on the host where the VSCP UDP interface is located.

<h4>packettype</h4>
Type of test packet to send to the node.

<li>
    <ul>0 - Unencrypted frame</ul>
    <ul>1 - A frame encrypted with AES128</ul>
    <ul>2 - A frame encrypted with AES192</ul>
    <ul>3 - A frame encrypted with AES256</ul>
<lil>

the key set on the node must be the default VSCP server key

<pre>
uint8_t key[] = { 0xA4,0xA8,0x6F,0x7D,0x7E,0x11,0x9B,0xA3,
                  0xF0,0xCD,0x06,0x88,0x1E,0x37,0x1B,0x98,
                  0x9B,0x33,0xB6,0xD6,0x06,0xA8,0x63,0xB6,
                  0x33,0xEF,0x52,0x9D,0x64,0x54,0x4F,0x8E };
</pre>

<h4>cnt</h4>
Number of frames to send.

<h4>bReply</h4>
This parameter tells if the testprogram should wait for a ACK package from the VSCP node or not. If set to zero it will not wait, if set to 0 it will.

<h4>Example</h4>

<pre>
    send_vscp_udp_frame 127.0.0.1 33333 1 1
</pre>

Send one AES128 encoded frame to the VSCP UDP node at 127.0.0.1:33333

<h2>receive_vscp_udp_frame</h2>

Receive UDP frames from (a) VSCP UDP node(s) and optionally dump there content. Encrypted (AES128/AES192/AES256) and non encrypted frames can be received.

<b>Format:</b> receive_vscp_udp_frame port "dump"

<h4>port</h4>
The port to listen for VSCP UDP frames on.

<h4>dump</h4>
If "dump" is give as second argument the contents of the received frames and other information will be displayed. The default is to not show this information.

<h4>Example</h4>

<pre>
    receive_vscp_udp_frame 9999 dump
</pre>    

Receive VSCP frames on this machine on port 9999 and dump frame contents.
