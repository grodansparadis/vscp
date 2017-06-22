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
