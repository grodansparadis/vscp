vscpEvent e;
e.vscp_class = 10;
e.vscp_type = 6;
e.sizeData = 4;
e.pdata = new uint8_t[4];
e.pdata[0] = 0x80;
e.pdata[1] = 0x02;
e.pdata[2] = 0x1B;
e.pdata[3] = 0x22;
//vscp_convertLevel1MeasuremenToLevel2String(&e);
vscp_convertLevel1MeasuremenToLevel2Double(&e);
double dv;
memcpy( (uint8_t *)&dv, e.pdata+4, 8 );
fprintf(stderr,"Double: %g\n", dv);
dv = VSCP_UINT64_SWAP_ON_LE(dv);
fprintf(stderr,"Double: %g\n", dv);
for (int i; i<8;i++) {
    fprintf(stderr,"%02X \n", e.pdata[i+4]);
}