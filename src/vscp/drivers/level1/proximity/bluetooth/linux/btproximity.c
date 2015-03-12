///////////////////////////////////////////////////////////////////////////////
// test app for bluetooth proximity  
//
// From
// http://www.linuxguru.se/comment/reply/10
//
// Written 2003 by Jon Allen <jj@jonallen.info>
// http://perl.jonallen.info/code
//
// C code adapted from hcitool.c 
// Copyright (C) 2000-2001 Qualcomm Incorporated
// Written 2000,2001 by Maxim Krasnyansky <maxk@qualcomm.com>
// http://bluez.sourceforge.net
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// Build with gcc btproximity.c /usr/lib/libbluetooth.so
// Must be "root" to be able to make connections.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#include <termios.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <asm/types.h>
#include <netinet/in.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

///////////////////////////////////////////////////////////////////////////////
// devname 
//
// pAddress should point to bt address in string form xx:yy:zz.....
// pName should point to char buffer with length 248 or more
//

int devname(char *pAddress, char *pName, int sz ) 
{
  bdaddr_t bdaddr;
  //char name[248]
  int dd;

  str2ba( pAddress, &bdaddr );

  int dev_id;
  dev_id = hci_get_route( &bdaddr );

  if ( dev_id < 0 ) {
    printf("Device not available\n");
    return -1;
  }
  
  dd = hci_open_dev(dev_id);
  if (dd < 0) {
    printf("HCI device open failed\n");
    return -2;
  }

  if ( hci_read_remote_name( dd, 
			     &bdaddr, 
			     sz, 
			     pName,
			     25000) != 0) {
    close(dd);
    printf("Could not find device %s\n", pAddress);
    return -3;
  }

  close(dd);
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// find_con
//

static int find_conn(int s, int dev_id, long arg)
{
  struct hci_conn_list_req *cl;
  struct hci_conn_info *ci;
  int i;
  int rv = 0;

  if (!(cl = malloc(10 * sizeof(*ci) + sizeof(*cl)))) {
    perror("Can't allocate memory\n");
    exit(1);
  }
  cl->dev_id = dev_id;
  cl->conn_num = 10;
  ci = cl->conn_info;

  if (ioctl(s, HCIGETCONNLIST, (void*)cl)) {
    perror("Can't get connection list\n");
    exit(1);
  }

  for (i=0; i < cl->conn_num; i++, ci++) {
    if (!bacmp((bdaddr_t *)arg, &ci->bdaddr)) {
      rv = 1;
      break;
    }
  }
  
  free(cl);
  cl = NULL;

  return rv;

}

///////////////////////////////////////////////////////////////////////////////
// read_rss
//
// pAddress should point to bt address in string form xx:yy:zz.....
// pRSSI Pointer to int for RSSI
//

int read_rssi(char *pAddress, int *pRSSI ) 
{
  int cc = 0;
  int dd;
  int dev_id;
  uint16_t handle;
  struct hci_conn_info_req *cr;
  struct hci_request rq;
  read_rssi_rp rp;
  bdaddr_t bdaddr;
  
  str2ba( pAddress, &bdaddr);
  
  dev_id = hci_for_each_dev( HCI_UP, find_conn, (long)&bdaddr );
  if (dev_id < 0) {
    dev_id = hci_get_route( &bdaddr );
    cc = 1;
  }

  if (dev_id < 0) {
    printf("Device not available\n");
    return -1;
  }
  
  dd = hci_open_dev(dev_id);
  if (dd < 0) {
    printf("Cannot open device\n");
    return -2;
  }
  
  if (cc) {
    if (hci_create_connection( dd, 
			       &bdaddr, 
			       0x0008 | 0x0010, 
			       0, 
			       0, 
			       &handle, 
			       25000 ) < 0) {
      printf("Cannot create connection\n");
      close(dd);
      return -3;
    }
  }
  
  cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
  if (!cr) {
    printf("Could not allocate memory\n");
    return -4;
  }
    
  bacpy(&cr->bdaddr, &bdaddr);
  cr->type = ACL_LINK;
  if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
    printf("Get connection info failed\n");
    return -5;
  }
  
  memset(&rq, 0, sizeof(rq));
  rq.ogf    = OGF_STATUS_PARAM;
  rq.ocf    = OCF_READ_RSSI;
  rq.cparam = &cr->conn_info->handle;
  rq.clen   = 2;
  rq.rparam = &rp;
  rq.rlen   = READ_RSSI_RP_SIZE;
  
  if ( hci_send_req( dd, &rq, 100 ) < 0 ) {
    printf("Read RSSI failed\n");
    return -6;
  }
  
  if (rp.status) {
    printf("Read RSSI returned (error) status 0x%2.2X\n", rp.status);
    return -7;
  }
  
  if (cc) {
    hci_disconnect(dd, handle, 0x13, 10000);
  }
  
  close(dd);
  free(cr);

  *pRSSI = rp.rssi;

  return 0;
}


#define BLUETOOTH_DEVICE  "00:16:B8:73:A2:FA" // Hedman
//#define BLUETOOTH_DEVICE  "00:80:37:EE:F6:C0" // T68
//#define BLUETOOTH_DEVICE  "00:16:B8:73:A7:B0" // MOO
//#define BLUETOOTH_DEVICE  "BD:16:B8:73:A2:FA" // Embedded Artist board 1
//#define BLUETOOTH_DEVICE  "BD:B2:03:00:57:88" // Embedded Artist board 2

///////////////////////////////////////////////////////////////////////////////
// main
//

int main()
{
  int rssi;
  char name[248];

  if ( 0 == devname( BLUETOOTH_DEVICE, name, 248 ) ) {
    printf( "Unit = %s Devicename = %s\n", BLUETOOTH_DEVICE, name );
  }
  read_rssi( BLUETOOTH_DEVICE, &rssi );
  printf( "Unit = %s RSSI = %d\n", BLUETOOTH_DEVICE, rssi );

  return 0;
}

