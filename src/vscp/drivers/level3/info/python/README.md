Level III VSCP Sample driver 
===========================

Entry in /etc/vscp/vscpd.conf

<!-- List level III VSCP drivers here -->
<level3driver enable="true">

    <!-- Level III Python -->
    <driver enable="true">
        <name>VSCP Level III Python test</name>
        <port>5005</port>
        <config></config>
        <path>/home/akhe/development/VSCP/vscp/src/vscp/drivers/level3/info/python/sampledrv.py</path>
    </driver>

</level3driver>



