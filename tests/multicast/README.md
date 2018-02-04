<h1>Test multicas channel functionality</h1>

Files to test the multicast interface of the VSCP server. Use make to build.
Only work on unix systems.

<h2>alistener</h2>
Listens for announce frames on the segment and print out info about them. Abort
with ctrl + C.

<b>Usage:</b>
"alistener" - Listen for all announcements from all hosts.
"alistener dump" - Same as above but the farme is also dumped.
"alistener nodump" - Same as "listener"
"alistener dump|nodump ip-address" - Listen only for announcement frames from
    the host with ip-address as its origin.

<h2>mclistener</h2>
Multicast listner. Used to listen on the traffic on a VSCP multicast channel.

<h2>mcsender</h2>

Send multicast VSCP event to VSCP multicast group once a second.

<h2>mcast_sender.py</h2>

Python sample code that send encrypted or unencrypted frames to a VSCP multicast group.

<pre>
    mcast_send.py -e <encryption> -g <group> -p <port> -t <ttl>

    or

    mcast_send.py --encrypt=<0|1|2|3> --group=<multicast group> --port=<port> --ttl=<n>
</pre>

Encryption is 0 for no encryption, 1 for AES128, 2 for AES192, 3 for AES256. The sample code used the default key for the VSCP server.

<h2>mcast_receiver.py</h2>

sample code that receives VSCP events from a VSCP multicast group.
