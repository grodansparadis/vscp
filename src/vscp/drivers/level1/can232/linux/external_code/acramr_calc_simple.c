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

gcc acramr_calc_simple.c -o acramr_calc_simple

*/

#include <stdio.h>
#include <stdlib.h>

int		ID[256];
int		numofids=0;

int main(int argc, char *argv[])
{
unsigned int	a=1,c,b,d,ACR,AMR;

	if(argc>1)
	{
		while(a<argc)
		{
			if((sscanf(argv[a],"%0xd",&ID[numofids]))!=1)
				if((sscanf(argv[a],"%d",&ID[numofids]))!=1)
				{
					printf("Error reading '%s'",argv[a]);fflush(stdout);
					return(1);
				}
			++numofids;
			++a;
		}
	}
	else
	{
		printf("Usage : %s id id ...  ( 0xXXXX for hexa or DDDDD for decimals...\n\n",argv[0]);fflush(stdout);
		return(1);
	}

	printf("\n\nWorking with %d IDs : ",numofids);
	for(a=0;a<numofids;a++)
	{
		printf("\n        0x%03x  ",ID[a]);
		for(b=11;b>0;b--)
		{
			printf("%d",((ID[a]>>b)&1));
		}
		printf("%d",((ID[a]>>b)&1));
	}


	ACR=0;AMR=0;
	for(a=0;a<11;a++)
	{
		c=-1;
		for(b=0;b<numofids;b++)
		{
			d=((ID[b]>>a)&1);
			switch(c)
			{
				case -1 :			// no data yet.
					c=d;
				break;
				case 1 :			// so far 1s
					if(d==0) c=2;		// this is going to be a don't care bit
				break;
				case 0 :
					if(d==1) c=2;
				break;
			}
		}
		if(c==2)
			AMR|=(1<<a);
		else
			if(c==1)
				ACR|=(1<<a);
	}

	printf("\n  ACR = 0x%04x ",ACR);
	for(b=11;b>0;b--)
	{
		printf("%d",((ACR>>b)&1));
	}
	printf("%d",((ACR>>b)&1));
	printf("\n  AMR = 0x%04x ",AMR);
	for(b=11;b>0;b--)
	{
		printf("%d",((AMR>>b)&1));
	}
	printf("%d",((AMR>>b)&1));
	printf("  ( %d value[s] )",((AMR==0)?(1):(AMR)));


	ACR<<=5;AMR<<=5;
	AMR|=0xF;	// don't care about data bytes

printf("\n\nRegister Values = ACR=%04x  AMR=%04x\n\n",ACR,AMR);fflush(stdout);
return(0);
}

/*

M07207960 & m00400000   =>   lets through only 3B and 3CB
M07207960 & m00000040   =>   lets through only 3C9 and 3CB
M07207960 & m00430003	=>   lets trough 39, 3B, 3CB
M79600720 & m00000040	=>   lets through only 39 and 3B
M79600720 & m00400000	=>   lets through only 39


*/

