/*==============================================================================*/
/* Program to calculcate ACR/AMR registers...					*/
/*										*/
/* DATE:	2004-Jun-06 23:45:45						*/
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

cc acramr_calc.c -o acramr_calc

'./acramr_calc 0x3cb 0x3ca 0x3c8 0x348 0x3b' produces the following output :

Working with 5 IDs :
        3CB  001111001011
        3CA  001111001010
        3C8  001111001000
        348  001101001000
        03B  000000111011....
Number of possible combinations = 30
Best result = 9  Worst result = 264
  9 results :   ACR = 07606900    AMR = 000F106F
         :: 03B  348  349  34A  34B  3C8  3C9  3CA  3CB
  9 results :   ACR = 69000760    AMR = 106F000F
         :: 03B  348  349  34A  34B  3C8  3C9  3CA  3CB

*/

#include <stdio.h>
#include <stdlib.h>


#define	USE_SORTING	1


#define	MAXARRAYSIZE	32

int	ID[MAXARRAYSIZE];
int	numofids=0,numofposs;
int	WorstNo,BestNo,ResultPhase;
int	WID1[MAXARRAYSIZE],WID2[MAXARRAYSIZE],WIDW[MAXARRAYSIZE];
int	ACR,AMR;
#ifdef	USE_SORTING
int	*ResultsBuffer=NULL;
int	NoOfResults=0,SizeOfResultsBuffer=0;
#endif
unsigned char	PCOM[4096];


int GiveACRAMR(int *IDBuffer)	// -1 terminated
{
int	a,b,c,d;
	ACR=0;AMR=0;
	for(a=0;a<11;a++)
	{
		c=-1;
		for(b=0;IDBuffer[b]!=-1;b++)
		{
			d=((IDBuffer[b]>>a)&1);
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
	c=1;
	for(b=11;b>=0;b--)
	{
		if(((AMR>>b)&1)) c*=2;
	}
return(c);
}


void PrintHexa(int NumOfDigits,int Number)
{
register int a,b;

	for(a=(NumOfDigits-1);a>=0;a--)
	{
		b=((Number>>(a<<2))&0xF);
		if(b<10)
			printf("%d",b);
		else
			printf("%c",('A'+b-10));
	}
}

#ifdef	USE_SORTING

void SortAndPrintResults(void)		// Just a quick bubble... No brainer, since result is limited...
{
register int a,b,c;

	for(a=0;a<(NoOfResults-1);a++)
	{
		for(b=(a+1);b<NoOfResults;b++)
		{
			if(ResultsBuffer[a]>ResultsBuffer[b])
			{
				c=ResultsBuffer[a];
				ResultsBuffer[a]=ResultsBuffer[b];
				ResultsBuffer[b]=c;
			}
		}
	}
	for(a=0;a<NoOfResults;a++)
	{
//		printf("%03x  ",ResultsBuffer[a]);
		PrintHexa(3,ResultsBuffer[a]);printf("  ");
	}
	fflush(stdout);
}

void AddToResultsBuffer(int NewValue)
{
register int	a;

	if(SizeOfResultsBuffer<=NoOfResults)
	{
		SizeOfResultsBuffer+=20;
		if((ResultsBuffer=(int*)realloc((void*)ResultsBuffer,(size_t)SizeOfResultsBuffer*sizeof(int)))==NULL)
		{
			printf("\n\nError allocating %d numbers for result buffer... Terminated...\n\n");fflush(stdout);
			exit(0);
		}
	}
	for(a=0;a<NoOfResults;a++)
	{
		if(ResultsBuffer[a]==NewValue) return;	// duplicate element...
	}
	ResultsBuffer[NoOfResults]=NewValue;
	++NoOfResults;
}
#endif

void CEval(void)
{
register int	a,tam,c;

	tam=0;c=1;
	for(a=0;a<11;a++)
	{
		if(PCOM[a]==1) tam+=c;
		c*=2;
	}
	tam+=ACR;
#ifdef	USE_SORTING
	AddToResultsBuffer(tam);
#else
	printf("%03x  ",tam);fflush(stdout);
#endif
}

void CWalkThisMany(int npc,int tm,int frh)
{
int	a,c=tm,k,tam;

	for(a=frh;a<npc;a++)
	{
		tam=AMR;
		for(k=0;k<a;k++) tam>>=1;
		if(tam&1)
		{
			PCOM[a]=1;
			if(--c==0)
			{
				CEval();
				PCOM[a]=0;
			}
			else
				CWalkThisMany(npc,c,a+1);
			PCOM[a]=0;c=tm;
		}
	}
	return;
}

void CWalker(int npc)
{
int	a,k;

	for(a=1;a<npc;a++)
	{
		for(k=0;k<npc;k++) PCOM[k]=0;
		CWalkThisMany(npc,a,0);
	}
}

void GiveCombo(int npc,int lacr,int lamr)
{
int	ac,am,b,c,d;

	ac=lacr>>5;
	am=lamr>>5;
	c=1;d=0;
	for(b=11;b>=0;b--)
	{
		if(((am>>b)&1)) {c*=2;++d;}
	}
#ifdef	USE_SORTING
	AddToResultsBuffer(ac);
#else
	printf("%03x  ",ac);fflush(stdout);
#endif
	if(d)
	{
		ACR=ac;AMR=am;
		CWalker(npc);
	}
}

void Eval(void)
{
int	a,b,c,r,r1,r2;
int	lACR,lAMR,lsACR,lsAMR;

	numofposs++;b=0;c=0;
	for(a=0;a<numofids;a++)
	{
		if(WIDW[a]==1)
			WID1[b++]=ID[a];
		else
			WID2[c++]=ID[a];
	}
	WID1[b]=-1;
	WID2[c]=-1;
	r=GiveACRAMR(WID1)+GiveACRAMR(WID2);
	if(ResultPhase==0)
	{
		if(r<BestNo) BestNo=r;
		if(r>WorstNo) WorstNo=r;
	}
	else
	{
		if(r==BestNo)
		{
			printf("\n  %d results : ",r);fflush(stdout);
			r1=GiveACRAMR(WID1);
			ACR<<=5;AMR<<=5;
			AMR|=0xF;	// don't care about data bytes
			lACR=ACR;lAMR=AMR;
			r2=GiveACRAMR(WID2);
			ACR<<=5;AMR<<=5;
			AMR|=0xF;	// don't care about data bytes
			lsACR=ACR;lsAMR=AMR;
//			printf("\tACR = %04x%04x    AMR = %04x%04x",lACR,ACR,lAMR,AMR);
			printf("\tACR = ");PrintHexa(4,lACR);PrintHexa(4,ACR);
			printf("    AMR = ");PrintHexa(4,lAMR);PrintHexa(4,AMR);
			printf("\n\t :: ");fflush(stdout);
			NoOfResults=0;
			GiveCombo(r1,lACR,lAMR);
			GiveCombo(r2,lsACR,lsAMR);
#ifdef	USE_SORTING
			SortAndPrintResults();
#endif
		}
	}
}

void WalkThisMany(int tm,int frh)
{
int	a,c=tm;

	for(a=frh;a<numofids;a++)
	{
		WIDW[a]=1;
		if(--c==0)
		{
			Eval();
			WIDW[a]=0;
		}
		else
			WalkThisMany(c,a+1);
		WIDW[a]=0;c=tm;
	}
	return;
}

void Walker(void)
{
int	a,k;

	if(numofids==1)
	{
			WalkThisMany(1,0);
	}
	else
	{
		for(a=1;a<numofids;a++)
		{
			if(ResultPhase==0) printf(".");fflush(stdout);
			for(k=0;k<numofids;k++) WIDW[k]=0;
			WalkThisMany(a,0);
		}
	}
}

int main(int argc, char *argv[])
{
unsigned int	a=1,c,d;
int		b;

	if(argc>1)
	{
		while(a<argc)
		{
			if(numofids>=MAXARRAYSIZE)
			{
				printf("\nSorry, code configured for max %d values...",MAXARRAYSIZE);
				printf("\nPlease change MAXARRAYSIZE variable and rebuild the code.\n(I was lazy to do proper malloc... Sorry...)\n\n");fflush(stdout);
				exit(0);
			}
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

	printf("\nWorking with %d IDs : ",numofids);
	for(a=0;a<numofids;a++)
	{
		printf("\n        ");PrintHexa(3,ID[a]);printf("  ");
		for(b=11;b>0;b--)
		{
			printf("%d",((ID[a]>>b)&1));
		}
		printf("%d",((ID[a]>>b)&1));
	}

	for(a=0;a<numofids;a++) WIDW[a]=0;
	numofposs=0;ResultPhase=0;
	WorstNo=0,BestNo=0xFFFFFF;
	Walker();

	printf("\nNumber of possible combinations = %d",numofposs);fflush(stdout);
	printf("\nBest result = %d  Worst result = %d",BestNo,WorstNo);fflush(stdout);

	for(a=0;a<numofids;a++) WIDW[a]=0;
	ResultPhase=1;
	Walker();

	printf("\n");fflush(stdout);
#ifdef	USE_SORTING
	if(ResultsBuffer!=NULL) free((void*)ResultsBuffer);
#endif

return(0);
}

