/*==============================================================================*/
/* Program to check if the CAN232 is working properly without being on the CAN	*/
/* I also use this to change UART speed. #ifdef SET_NEW_UART_SPEED		*/
/*										*/
/* DATE:	2004-Jun-06 17:22:42						*/
/*										*/
/* Copyright (c) 2004 Attila Vass						*/
/*										*/
/* Permission is hereby granted, free of charge, to any person obtaining a copy */
/* of this software and associated documentation files (the "Software"),to deal */
/* in the Software without restriction, including without limitation the rights */
/* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell	*/
/* copies of the Software, and to permit persons to whom the Software is 	*/
/* furnished to do so, subject to the following conditions:			*/
/*										*/
/* The above copyright notice and this permission notice shall be included in	*/
/* all copies or substantial portions of the Software.				*/
/*										*/
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR	*/
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,	*/
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE	*/
/* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER	*/
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING	*/
/* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS	*/
/* IN THE SOFTWARE.								*/
/*==============================================================================*/

/*

gcc canread.c -o canread

*/

//#define	BAUD	B57600
//#define	BAUD	B115200
#define	BAUD	B230400

//#define DEVICE "/dev/ttyS0"		// Normal access to serial port
#define	DEVICE	"/dev/ttyUSB0"		// USBtoRS232 converter access



#include <stdio.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <termios.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

//#define COMM_DEBUG	1

long	zeso=0;
long	curtime=0;
int	Port=-1;
char	*IB=NULL;
int	IBS=0;
int	IBCP=0;
int	CRSignal=0;
unsigned char	StatusFlag=0;
char	Version[4];
char	Serial[5];
char	Message[8];


void gettime(void)
{
struct timeval	ltim;

	gettimeofday(&ltim,NULL);
	curtime=((ltim.tv_sec-zeso)*1000)+(ltim.tv_usec/1000);
}

void zerotime(void)	// This could take up to a second...
{
struct timeval	ltim;
register int	a;
register long	b=0;


	while(1)
	{
		gettimeofday(&ltim,NULL);
		if(ltim.tv_usec<b)
		{
			zeso=ltim.tv_sec;
			return;
		}
		b=ltim.tv_usec;
	}
}

int OpenAndConfigurePort(void)
{
struct termios newio;

#ifdef COMM_DEBUG
printf("\nOpening port on %s.",DEVICE);fflush(stdout);
#endif

	if((Port=open(DEVICE,O_RDWR|O_NOCTTY)) < 0)
	{
		printf("\nError Opening Serialport ( %s ) : '%s'",DEVICE,strerror(errno));fflush(stdout);
		return(1);
	}
	memset(&newio,0, sizeof(newio)); /* Clears termios struct  */   
	newio.c_cflag = CS8 | CLOCAL | CREAD; 
 	newio.c_iflag = IGNPAR; 
	newio.c_oflag = 0; 
	newio.c_lflag = 0; 
	newio.c_cc[VTIME] = 0;
	newio.c_cc[VMIN]  = 0;   /* read min. one char at a time  */
	if (cfsetispeed(&newio, BAUD) == -1)
	{
		printf("Error setting serial input baud rate\n");
		close(Port);
		return(1);
	}
	if(cfsetospeed(&newio, BAUD) == -1)
	{
		printf("Error setting serial output baud rate\n");
		close(Port);
		return(1);
	}
	tcflush(Port, TCIFLUSH);
	if (tcsetattr( Port, TCSANOW, &newio) == -1)
	{
		printf("Error setting terminal attributes\n");
		close(Port);
		return(1);
	}

return(0);
}


int WriteToPort(char* ZTV)
{
register a=0,b=0,str=0;

	while(*(ZTV+a)!='\0') ++a;
	b=write(Port,ZTV,a);
//	tcflush(Port,TCOFLUSH);

#ifdef COMM_DEBUG
printf("wrote %d.",b);fflush(stdout);
#endif

return(b);
	

	while(*(ZTV+a)!='\0')
	{
		do
		{
			b=0;
			b=write(Port,ZTV+a,1);

#ifdef COMM_DEBUG
printf("wrote:%d",*(ZTV+a));fflush(stdout);
#endif

		} while(!b);
		++a;
	}
	tcflush(Port,TCOFLUSH);
return(a);
}

void DumpBuffer(void)
{
register int a=IBCP,b;

printf("\n%d bytes in buffer :\n",a);fflush(stdout);
	for (b=0;b<a;b++)
	{
		printf("%02x",IB[b]);
		if((IB[b]>=' ')&&(IB[b]<127)) printf("'%c':", IB[b]);
			else printf("'.'");
		if(b%8) printf("\n");
		fflush(stdout);
	}

}

int Poll(void)
{
register int a,b,c,d,ret,go=1;

	if((ret=read(Port,IB+IBCP,(IBS-IBCP)))<1) return(0);
	IBCP+=ret;
	a=IBCP;

#ifdef COMM_DEBUG
printf("STS");fflush(stdout);
DumpBuffer();
#endif

	while(go)
	{
		a=IBCP;
		for(b=0;b<a;b++)
		{
			if((IB[b]==13)||(IB[b]==7))	// Terminator
			{

#ifdef COMM_DEBUG
printf("\n%d/%d : ",b,IBCP);fflush(stdout);
#endif

				switch(IB[0])	// 0 is OK, because we tidy up the buffer as we go...
				{
					case 'V' :	// version
						for(c=0;c<4;c++) Version[c]=IB[c+1];
#ifdef COMM_DEBUG
printf("Version Recognized");fflush(stdout);
#endif
					break;
					case 'N' :
						for(c=0;c<4;c++) Serial[c]=IB[c+1];
					break;
					case 'F' :	// Status
						if(IB[1]>='A')
							StatusFlag=(IB[1]-'A')+10;
						else
							StatusFlag=(IB[1]-'0');
						StatusFlag<<=4;
						if(IB[2]>='A')
							StatusFlag+=(IB[2]-'A')+10;
						else
							StatusFlag+=(IB[2]-'0');
						CRSignal=1;
					break;
					case 'z' :	// transmission ack
					case 'Z' :
#ifdef COMM_DEBUG
printf("Transmission Success");fflush(stdout);
#endif
						CRSignal=1;
					break;
					case 13 :
						CRSignal=1;
					break;
					case 7 :
printf("\n*****  ERROR BELL RECEIVED...");fflush(stdout);
						CRSignal=1;
					break;
					default :
#ifdef COMM_DEBUG
printf("Header Byte : '%c' ",IB[0]);fflush(stdout);
for(c=0;c<b;c++) printf("'%c'",IB[c]);
fflush(stdout);
#endif
					break;
				}
		// Tidy up...
				if(IBCP==(b+1))	// This is the only message, so no copy req.
					IBCP=0;
				else
				{
					a=b+1;		// from here
					c=IBCP-b;	// this many
#ifdef COMM_DEBUG
printf(" => %d/%d : ",a,c);fflush(stdout);
#endif
					for(d=0;d<c;d++)
					{
						IB[d]=IB[d+a];
					}
					IBCP-=(b+1);
#ifdef COMM_DEBUG
printf(" -> %d",IBCP);fflush(stdout);
#endif
				}
				b=a+2;	// quit from loop in 'C' fashion...
			}
#ifdef COMM_DEBUG
DumpBuffer();
#endif
		}
		if(b==a) go=0;
	}

#ifdef COMM_DEBUG
printf("RET");fflush(stdout);
#endif

return(ret);
}

void GoodBye(void)
{

	printf("\nClosing port %d and freeing buffer %x\n\n",Port,IB);fflush(stdout);

	sprintf(Message,"C\015");	// Close the CAN channel
	WriteToPort(Message);

	if(Port>=0) close(Port);
	if(IB!=NULL) free((void*)IB);
}

void SigCatch(int sig)
{
	switch(sig)
	{
		case SIGQUIT	:

// We could print here...  CTRL+4

			DumpBuffer();

		break;
		case SIGBUS  :
		case SIGSEGV : 
			printf("\nSYSTEM TERMINATION ...");fflush(stdout);
			GoodBye();
			exit(1);
		case SIGINT	: 
		case SIGHUP	:
		case SIGTERM	:	// Ctrl+C
			DumpBuffer();
			GoodBye();
			exit(1);
		break;
	}
}

void SetUpSignals()
{
	signal(SIGINT,SigCatch);
	signal(SIGHUP,SigCatch);
	signal(SIGBUS,SigCatch);
	signal(SIGSEGV,SigCatch);
	signal(SIGQUIT,SigCatch);
	signal(SIGTERM,SigCatch);
}

int main (int argc,char **argv)
{
int	to;

	IBS=256*1024;
	if((IB=(char*)malloc((size_t)IBS))==NULL)
	{
		printf("\n\nCan not allocate buffer with %d size...",IBS);fflush(stdout);
		return(0);
	}
	Version[0]='\0';
	Serial[0]='\0';Serial[4]='\0';
	IBCP=0;

#ifdef COMM_DEBUG
printf("\nMemory allocated for %d bytes at %x",IBS,IB);fflush(stdout);
#endif

	SetUpSignals();
	zerotime();

printf("\n... Opening port");fflush(stdout);
	if(OpenAndConfigurePort())
	{
		return;
	}

	gettime();
printf("\n%6d : Port opened. (%d)",curtime,Port);fflush(stdout);
printf("\n... Clearing buffer");fflush(stdout);

	sprintf(Message,"\015\015\015");
	WriteToPort(Message);

printf("\n... Sending Version Request");fflush(stdout);

	sprintf(Message,"V\015");	// get version
	WriteToPort(Message);

	to=1000000;
	while(to)
	{
		if(Poll())
		{
			if(Version[0]!='\0') to=1;
		}
		--to;
	}
	gettime();

printf("\n%6d : Version = HW : %c.%c  SW : %c.%c",curtime,Version[0],Version[1],Version[2],Version[3]);fflush(stdout);
printf("\n... Sending Serial Request");fflush(stdout);

	sprintf(Message,"N\015");	// get Serial
	WriteToPort(Message);

	to=1000000;
	while(to)
	{
		if(Poll())
		{
			if(Serial[0]!='\0') to=1;
		}
		--to;
	}

printf("\n%6d : Serial ID = %s  ( %02x:%02x:%02x:%02x )",curtime,Serial,Serial[0],Serial[1],Serial[2],Serial[3]);fflush(stdout);


// When I need to switch from 230K to 115K and wise versa, I uncomment the #def on this one and change U1/U2...

//#define SET_NEW_UART_SPEED	1
#ifdef SET_NEW_UART_SPEED

	CRSignal=0;
	sprintf(Message,"U1\015");	// UART with U0=230K, U1=115.2K U2=57.6K
	WriteToPort(Message);
	to=1000000;
	while(to)
	{
		if(Poll())
		{
			if(CRSignal) to=1;
		}
		--to;
	}
	if(CRSignal) printf("Set Successfully...");fflush(stdout);

	GoodBye();
	return;
#endif

	CRSignal=0;
	sprintf(Message,"S6\015");	// CAN with 500Kbps S0-10 S1-20 S2-50 S3-100 S4-125 S5-250 S7-800  S8-1M
	WriteToPort(Message);
	to=1000000;
	while(to)
	{
		if(Poll())
		{
			if(CRSignal) to=1;
		}
		--to;
	}


printf("\nCAN speed setup.");fflush(stdout);

// The below needs to be done only once...
#if 0
	CRSignal=0;
	sprintf(Message,"X1\015");	// Turn on out poll
	WriteToPort(Message);
	to=1000000;
	while(to)
	{
		if(Poll())
		{
			if(CRSignal) to=1;
		}
		--to;
	}

printf("\nAuto Poll setup.");fflush(stdout);

#endif


	CRSignal=0;
	sprintf(Message,"O\015");	// Open the CAN channel
	WriteToPort(Message);
	to=1000000;
	while(to)
	{
		if(Poll())
		{
			if(CRSignal) to=1;
		}
		--to;
	}

printf("\nChannel opened.");fflush(stdout);

	CRSignal=0;
	sprintf(Message,"t0000\015");	// Send 0 byte message to test CAN
	WriteToPort(Message);
	to=1000000;
	while(to)
	{
		if(Poll())
		{
			if(CRSignal) to=1;
		}
		--to;
	}

printf("\nBogus message sent, waiting 5 sec., check red LED, should be lit...");fflush(stdout);

	sleep(5);

	CRSignal=0;
	sprintf(Message,"F\015");	// Get Status
	WriteToPort(Message);
	to=1000000;
	while(to)
	{
		if(Poll())
		{
			if(CRSignal) to=1;
		}
		--to;
	}

printf("\nStatus Flag = %02x",StatusFlag);fflush(stdout);

	to=1000;
	while(to)
	{
		if(Poll())
		{
		}
		--to;
	}

	GoodBye();

printf("\n\n");fflush(stdout);
}
