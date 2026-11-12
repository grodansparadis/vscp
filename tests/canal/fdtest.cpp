#include <vscp-client-socketcan.h>
#include <cstdio>
#include <unistd.h>
int main() {
  vscpClientSocketCan c;
  c.init("vcan0", "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00", vscpClientSocketCan::FLAG_FD_MODE);
  printf("isFdCapable=%d\n", c.isFdCapable());
  if (c.connect()) { printf("connect failed\n"); return 1; }
  usleep(300000);

  // 1. CANAL send: FD, 10 bytes (padded to 12), extended, BRS
  canalMsgFD m = {};
  m.flags = CANAL_IDFLAG_EXTENDED | CANAL_IDFLAG_FD | CANAL_IDFLAG_BRS;
  m.id = 0x0A0B0C01; m.sizeData = 10;
  for (int i = 0; i < 10; i++) m.data[i] = i + 1;
  printf("send FD 10 -> %d\n", c.send(m));
  // 2. CANAL send: standard frame, 11-bit id, 3 bytes
  canalMsgFD s = {}; s.id = 0x123; s.sizeData = 3; s.data[0]=0xAA; s.data[1]=0xBB; s.data[2]=0xCC;
  printf("send std 3 -> %d\n", c.send(s));
  // 3. sizeData > 64 rejected
  canalMsgFD bad = {}; bad.sizeData = 65;
  printf("send 65 -> %d (expect %d)\n", c.send(bad), VSCP_ERROR_PARAMETER);

  printf("ready\n"); fflush(stdout);
  sleep(2); // external frames injected now

  canalMsgFD r; int n = 0;
  while (VSCP_ERROR_SUCCESS == c.receive(r)) {
    printf("CANAL rx: id=%08X flags=%08X size=%u data[0]=%02X data[last]=%02X\n",
           r.id, r.flags, r.sizeData, r.data[0], r.sizeData ? r.data[r.sizeData-1] : 0);
    n++;
  }
  printf("CANAL frames=%d\n", n);
  vscpEvent ev; int nv = 0;
  memset(&ev, 0, sizeof(ev));
  while (VSCP_ERROR_SUCCESS == c.receive(ev)) {
    printf("VSCP rx: class=%u type=%u size=%u\n", ev.vscp_class, ev.vscp_type, ev.sizeData);
    nv++;
  }
  printf("VSCP events=%d\n", nv);
  c.disconnect();
  return 0;
}
