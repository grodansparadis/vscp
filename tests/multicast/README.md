<h1>Test multicast</h1>

Files to test the multicast interface of the VSCP server. Use make to build.
Only work on unix systems.

<b>listener</b>
Listens for announce frames on the segment and print out info about them. Abort
with ctrl + C.

<b>Usage:</b>
"listener" - Listen for all announcements from all hosts.
"listener dump" - Same as above but the farme is also dumped.
"listener nodump" - Same as "listener"
"listener dump|nodump ip-address" - Listen only for announcement frames from
    the host with ip-address as its origin.
