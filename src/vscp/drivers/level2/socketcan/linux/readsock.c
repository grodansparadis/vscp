// readsock.cpp : 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP Project (http://www.vscp.org) 
//
// Copyright (C) 2000-2019 Ake Hedman, 
// Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

int
main(void)
{
	int i;
	int sock;
	char devname[IFNAMSIZ + 1];
	fd_set rdfs;
	struct timeval tv;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct cmsghdr *cmsg;
	struct canfd_frame frame;
	char ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32))];
	const int canfd_on = 1;

	strncpy(devname, "can0", sizeof(devname)-1);
#if DEBUG	
	perror(LOG_ERR, "CWriteSocketCanTread: Interface: %s\n", ifname);
#endif	

	sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (sock < 0) {
		printf("%s",
			(const char *) "CReadSocketCanTread: Error while opening socket. Terminating!");
		return 0;
	}

	strcpy(ifr.ifr_name, devname);
	ioctl(sock, SIOCGIFINDEX, &ifr);

	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

#ifdef DEBUG
	printf("using interface name '%s'.\n", ifr.ifr_name);
#endif

	// try to switch the socket into CAN FD mode 
	setsockopt(sock,
		SOL_CAN_RAW,
		CAN_RAW_FD_FRAMES,
		&canfd_on,
		sizeof(canfd_on));

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		printf("%s",
			(const char *) "CReadSocketCanTread: Error in socket bind. Terminating!");
		return 0;
	}

	// We only read
	//shutdown(sock, SHUT_WR);

	int bRun = 1;
	while (bRun) {

		FD_ZERO(&rdfs);
		FD_SET(sock, &rdfs);

		tv.tv_sec = 0;
		tv.tv_usec = 5000;

		int ret;
		if ((ret = select(sock, &rdfs, NULL, NULL, &tv)) < 0) {
			bRun = 0;
			continue;
		}

		ret = read(sock, &frame, sizeof(struct can_frame));
		if (ret < 0) {
			bRun = 0;
			continue;
		}

		printf("Frame received size=%d \n", ret);

		// Must be Extended
		if (ret && !(frame.can_id & CAN_EFF_FLAG)) continue;

		printf("id=%0Xd ", frame.can_id & CAN_EFF_MASK);
		printf("dlc=%d ", frame.len);
		for (i = 0; i < frame.len; i++) {
			printf("%02X ", frame.data[i]);
		}
		
		printf("\n");
		
		frame.can_id  = 0x123;
		frame.len = 2;
		frame.data[0] = 0x11;
		frame.data[1] = 0x22;
 
		write(sock, &frame, sizeof(struct can_frame));
	}

	// Close the socket
	close(sock);

	return 0;
}