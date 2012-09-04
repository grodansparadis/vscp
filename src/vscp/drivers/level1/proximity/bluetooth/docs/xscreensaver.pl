#! /usr/bin/perl

# http://www.linuxguru.se/comment/reply/10
#
# xscreensaver.pl
# Activates screen saver when Bluetooth device goes out of range
#
# Written 2003 by Jon Allen <jj@jonallen.info>
# http://perl.jonallen.info/code
#
# C code adapted from hcitool.c 
# Copyright (C) 2000-2001 Qualcomm Incorporated
# Written 2000,2001 by Maxim Krasnyansky <maxk@qualcomm.com>
# http://bluez.sourceforge.net
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;

use Inline C => Config => MYEXTLIB => '/usr/lib/libbluetooth.so';
use Inline C => <<EOT;

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

SV* devname(char* address) {
  bdaddr_t bdaddr;
  char name[248];
  int dd;

  str2ba(address, &bdaddr);

  int dev_id;
  dev_id = hci_get_route(&bdaddr);
  if (dev_id < 0) {
    printf("Device not available\\n");
    return (newSVpvf(""));
  }
  
  dd = hci_open_dev(dev_id);
  if (dd < 0) {
    printf("HCI device open failed\\n");
    return (newSVpvf(""));
  }

  if (hci_read_remote_name(dd, &bdaddr, sizeof(name), name,25000) != 0) {
    close(dd);
    printf("Could not find device %s\\n",address);
    return (newSVpvf(""));
  }

  close(dd);
  return (newSVpvf("%s",name));
}


static int find_conn(int s, int dev_id, long arg)
{
	struct hci_conn_list_req *cl;
	struct hci_conn_info *ci;
	int i;

	if (!(cl = malloc(10 * sizeof(*ci) + sizeof(*cl)))) {
		perror("Can't allocate memory");
		exit(1);
	}
	cl->dev_id = dev_id;
	cl->conn_num = 10;
	ci = cl->conn_info;

	if (ioctl(s, HCIGETCONNLIST, (void*)cl)) {
		perror("Can't get connection list");
		exit(1);
	}

	for (i=0; i < cl->conn_num; i++, ci++)
		if (!bacmp((bdaddr_t *)arg, &ci->bdaddr))
			return 1;
	return 0;
}


SV* read_rssi(char* address) {
  int cc = 0;
  int dd;
  int dev_id;
  uint16_t handle;
  struct hci_conn_info_req *cr;
  struct hci_request rq;
  read_rssi_rp rp;
  bdaddr_t bdaddr;
  
  str2ba(address, &bdaddr);
  
  dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
  if (dev_id < 0) {
    dev_id = hci_get_route(&bdaddr);
    cc = 1;
  }
  if (dev_id < 0) {
    printf("Device not available\\n");
    return (newSVpvf(""));
  }
  
  dd = hci_open_dev(dev_id);
  if (dd < 0) {
    printf("Cannot open device\\n");
    return (newSVpvf(""));
  }
  
  if (cc) {
    if (hci_create_connection(dd, &bdaddr, 0x0008 | 0x0010, 0, 0, &handle, 25000) < 0) {
      printf("Cannot create connection\\n");
      close(dd);
      return (newSVpvf(""));
    }
  }
  
  cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
  if (!cr) {
    printf("Could not allocate memory\\n");
    return (newSVpvf(""));
  }
    
  bacpy(&cr->bdaddr, &bdaddr);
  cr->type = ACL_LINK;
  if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
    printf("Get connection info failed\\n");
    return (newSVpvf(""));
  }
  
  memset(&rq, 0, sizeof(rq));
  rq.ogf    = OGF_STATUS_PARAM;
  rq.ocf    = OCF_READ_RSSI;
  rq.cparam = &cr->conn_info->handle;
  rq.clen   = 2;
  rq.rparam = &rp;
  rq.rlen   = READ_RSSI_RP_SIZE;
  
  if (hci_send_req(dd, &rq, 100) < 0) {
    printf("Read RSSI failed\\n");
    return (newSVpvf(""));
  }
  
  if (rp.status) {
    printf("Read RSSI returned (error) status 0x%2.2X\\n", rp.status);
    return (newSVpvf(""));
  }
  
  if (cc) {
    hci_disconnect(dd, handle, 0x13, 10000);
  }
  
  close(dd);
  free(cr);
  return (newSVpvf("%d",rp.rssi));
}

EOT

#-------------------------------------------------------------------

use strict;
use warnings;

my $rssi_threshold = -4;
my $rssi_linger    = 2;    # No. of measurements needed before changing state
my @rssi_stack     = ();
#my $near_command   = '/usr/local/bin/xscreensaver-command -deactivate';
#my $far_command    = '/usr/local/bin/xscreensaver-command -activate';
#/usr/share/xscreensaver
my $near_command   = 'echo "------------> near\n"';
my $far_command    = 'echo "------------> far\n"';
my $state          = 'far';
my $mac            = $ARGV[0] || 'BD:B2:03:00:57:89';


$|++;
printf("Monitoring RSSI for device %s (%s)\\n",$mac,devname($mac));
RSSI_CHECK: while (1) {
  # Trim the stack
  while (scalar @rssi_stack >= $rssi_linger) {
    shift @rssi_stack;
  }
  
  # Try to measure the RSSI
  my $current_rssi = read_rssi($mac);
  if ($current_rssi eq '') {
    $current_rssi = $rssi_threshold;
  }
  push @rssi_stack,$current_rssi;
  
  printf("\n[%s] RSSI = %s     ",scalar localtime,$current_rssi);
  
  # Check if we need to change state
  if ($state eq 'far') {
    foreach my $measurement (@rssi_stack) {
      next RSSI_CHECK if ($measurement <= $rssi_threshold);
    }
    $state = 'near';
    `$near_command`;
  }
  if ($state eq 'near') {
    foreach my $measurement (@rssi_stack) {
      next RSSI_CHECK if ($measurement > $rssi_threshold);
    }
    $state = 'far';
    `$far_command`;
  }
} continue {
#  sleep 1;
}
