/*==============================================================================*/
/* Very basic terminal program fpr restoring CAN232 basic settings...		*/
/*										*/
/* DATE:	2004-May-19 18:20:12						*/
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

gcc canterminal.c -o canterminal

*/

#define	BAUD	B115200
/*
B57600
B115200
B230400
*/

//#define DEVICE "/dev/ttyS0"
// Since I have a USR to Serial, I need this :

#define	DEVICE	"/dev/ttyUSB0"


/* RESTORE FACTORY SETTINGS :

- connect at 115200
- keep pressing the '.' key ( sending '.' continously )
- power on CAN232
- you will see CAN232 EEPROM >
- release '.' key
- type 'R01' enter
- result is 03 for 230K, 0F for 57K
- type 'W010F' enter
- a '*' shall appear
- type 'R01' again to verify, result should be '0F'
- close terminal program and cut power to CAN232.

*/


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
int	LP=2;
struct	termios	oldT,newT;
char	Buffer[256];
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
int	flag;

#ifdef COMM_DEBUG
printf("\nOpening port on %s.",DEVICE);fflush(stdout);
#endif

	if((Port=open(DEVICE, O_RDWR|O_NOCTTY)) < 0)
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
	newio.c_cc[VMIN]  = 1;   /* read min. one char at a time  */
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

	if(fcntl(Port, F_SETOWN, getpid())>=0)
	{
		flag|=FASYNC;
		if(fcntl(Port, F_SETFL, flag)>=0)
		{
printf("\nAll Set for Polling... FD = %d",Port);
		}
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

int Poll(void)
{
register int a,ret;

	if((ret=read(Port,Buffer,256))<1) return(0);
	if(LP!=1) printf("\n :");fflush(stdout);
	for(a=0;a<ret;a++)
	{
		if((Buffer[a]<' ')||(Buffer[a]>'~'))
		{
			switch(Buffer[a])
			{
				case 13 : printf("[CR]");
				break;
				case 7 : printf("[BELL]");
				break;
				default : printf("[%02x]",Buffer[a]);
			}
		}
		else
		printf("%c",Buffer[a]);
	}
	LP=1;
fflush(stdout);

return(ret);
}

void ProcessSigIo()
{
struct timeval	to;
fd_set		fdset;


//printf("\nSigIo !");fflush(stdout);

	FD_ZERO(&fdset);
	FD_SET(Port,&fdset);
	to.tv_sec=0;
	to.tv_usec=0;
	if(select(FD_SETSIZE,&fdset,0,0,&to)!=-1)
	{
		if(FD_ISSET(Port,&fdset)) Poll();
	}
}


void GoodBye(void)
{
#ifdef COMM_DEBUG
printf("\nClosing port %d",Port);fflush(stdout);
#endif
	if(Port>=0) close(Port);
	ioctl(0,TCSETS,&oldT);
}

void SigCatch(int sig)
{
	switch(sig)
	{
		case SIGQUIT	:

// We could print here...  CTRL+4

		break;
		case SIGBUS  :
		case SIGSEGV : 
			printf("\nSYSTEM TERMINATION ...");fflush(stdout);
			GoodBye();
			exit(1);
		case SIGINT	: 
		case SIGHUP	:
		case SIGTERM	:	// Ctrl+C
			GoodBye();
			exit(1);
		break;
		case SIGIO :
			ProcessSigIo();
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
	signal(SIGIO,SigCatch);
}

int main (int argc,char **argv)
{
int	go=1,a;

	SetUpSignals();

	ioctl(0,TCGETS,&oldT);
	ioctl(0,TCGETS,&newT);
	newT.c_lflag&=~ECHO;
	newT.c_lflag&=~ICANON;
	ioctl(0,TCSETS,&newT);

	zerotime();

printf("\n... Opening port");fflush(stdout);
	if(OpenAndConfigurePort())
	{
		return;
	}

	gettime();
printf("\n%6d : Port opened.",curtime);fflush(stdout);

	sprintf(Message,"\015\015\015");
	WriteToPort(Message);

printf("\n\n RESTORING FACTORY SETTINGS :\n\
- connect at 115200\n\
- keep pressing the '.' key ( sending '.' continously )\n\
- power on CAN232\n\
- you will see CAN232 EEPROM >\n\
- release '.' key\n\
- type 'R01' enter\n\
- result is 03 for 230K, 0F for 57K\n\
- type 'W010F' enter\n\
- a '*' shall appear\n\
- type 'R01' again to verify, result should be '0F'\n\
- close terminal program and cut power to CAN232.\n");fflush(stdout);

	while(go)
	{
		if((a=getchar())==27) go=0;
		else
		{
			if(LP==1) printf("\n");
			switch(a)
			{
				case 10 : printf("[CR]");
					  a=13;
				break;
				case 7 : printf("[BELL]");
				break;
				default :
					printf("%c",(char)a);
			}
			fflush(stdout);
			Message[0]=(char)a;Message[1]=0;
			WriteToPort(Message);
			LP=0;
		}
	}
	GoodBye();

printf("\n\n");fflush(stdout);
}
