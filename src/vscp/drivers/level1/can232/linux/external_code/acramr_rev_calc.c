/*==============================================================================*/
/* Program to calculcate results based on ACR/AMR registers...			*/
/*										*/
/* DATE:	2004-Jun-06 22:17:10						*/
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

cc acramr_rev_calc.c -o acramr_rev_calc

'./acramr_rev_calc M69000760 m106F000F' produces the following output :

                                                                                      
Working with ACR = 69000760   AMR = 106F000F

9 results =  03B  348  349  34A  34B  3C8  3C9  3CA  3CB

*/

#include <stdio.h>
#include <stdlib.h>


#define	USE_SORTING	1
#define PRIUS_SPECIFIC	1

int	ACR,AMR,OriACR,OriAMR;
#ifdef	USE_SORTING
int	*ResultsBuffer=NULL;
int	NoOfResults=0,SizeOfResultsBuffer=0;
#endif

#ifdef PRIUS_SPECIFIC
int	PriusValues[]={0x2,0x20,0x22,0x23,0x25,0x30,0x38,0x39,0x3A,0x3B,0x3E,0x60,0x87,0xB0,0xB1,0xB3,0xB4,0xC9,0x120,0x230,0x244,0x262,0x348,0x34F,0x3C8,0x3C9,0x3CA,0x3CB,0x3CD,0x3CF,0x423,0x4C1,0x4C3,0x4C6,0x4C7,0x4C8,0x4CE,0x4D0,0x4D1,0x520,0x521,0x526,0x527,0x528,0x529,0x52C,0x540,0x553,0x554,0x56D,0x57F,0x591,0x5A4,0x5B2,0x5B6,0x5C8,0x5CC,0x5D4,0x5EC,0x5ED,0x5F8,0x602,0xC00,0xFFF};
#endif

unsigned char	PCOM[4096];


int GiveAMRComb(void)
{
register int	c,b;

	c=1;
	for(b=11;b>=0;b--)
	{
		if(((AMR>>b)&1))
		{
			c*=2;
		}
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
#ifdef PRIUS_SPECIFIC
		b=ResultsBuffer[a];
		c=0;
		while((c<0xFFF)&&(PriusValues[c]!=0xFFF))
		{
			if(b==PriusValues[c])
			{
				PrintHexa(3,ResultsBuffer[a]);printf("  ");
				c=0xFFE;
			}
			++c;
		}
#else
		PrintHexa(3,ResultsBuffer[a]);printf("  ");
#endif
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

#if 0
void CEval(void)
{
register int	a,tam,c;

	tam=0;c=1;
	for(a=0;a<11;a++)
	{
		if(PCOM[a]==1) tam+=c;
//		c*=2;
		c<<=1;
	}
	tam+=ACR;
#ifdef	USE_SORTING
	AddToResultsBuffer(tam);
#else
	printf("%03x  ",tam);fflush(stdout);
#endif
}
#endif

void CWalkThisMany(int npc,int tm,int frh)
{
register int	a,c=tm,k,tam,ca,ctam,cc;

	for(a=frh;a<npc;a++)
	{
		tam=AMR;
		for(k=0;k<a;k++) tam>>=1;
		if(tam&1)
		{
			PCOM[a]=1;
			if(--c==0)
			{

				ctam=0;cc=1;
				for(ca=0;ca<11;ca++)
				{
					if(PCOM[ca]==1) ctam+=cc;
//					cc*=2;
					cc<<=1;
				}
				ctam+=ACR;
#ifdef	USE_SORTING
				AddToResultsBuffer(ctam);
#else
				printf("%03x  ",ctam);fflush(stdout);
#endif

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
int	a,k,d=0;

	for(a=1;a<npc;a++)
	{
		if(((a*100)/npc)>d)
		{
			printf("%d%% ",d);fflush(stdout);
			d+=5;
		}
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
int	r1,r2;
int	lACR,lAMR,lsACR,lsAMR;

	AMR=(OriAMR&0xFFFF);
	AMR>>=5;
	r1=GiveAMRComb();		// AMR based possible combinations
	ACR=(OriACR&0xFFFF);
	AMR=(OriAMR&0xFFFF);
	lACR=ACR;lAMR=AMR;

	AMR=((OriAMR>>16)&0xFFFF);
	AMR>>=5;
	r2=GiveAMRComb();
	ACR=((OriACR>>16)&0xFFFF);
	AMR=((OriAMR>>16)&0xFFFF);
	lsACR=ACR;lsAMR=AMR;
	NoOfResults=r1+r2;
//	printf("\tACR = %04x%04x    AMR = %04x%04x",lACR,ACR,lAMR,AMR);
	printf("\n\n%d results =  ",NoOfResults);fflush(stdout);
	NoOfResults=0;
	GiveCombo(r1,lACR,lAMR);
	GiveCombo(r2,lsACR,lsAMR);
#ifdef	USE_SORTING
	SortAndPrintResults();
#endif
}

int main(int argc, char *argv[])
{
unsigned int	a=1,c,d;
int		b;

	if(argc==3)
	{
		if((sscanf(argv[1],"M%0xd",&OriACR))!=1)
		{
			printf("Error reading '%s'",argv[1]);fflush(stdout);
			return(1);
		}
		if((sscanf(argv[2],"m%0xd",&OriAMR))!=1)
		{
			printf("Error reading '%s'",argv[1]);fflush(stdout);
			return(1);
		}
	}
	else
	{
		printf("Usage : %s Mxxxxxxxx mxxxxxxxx ( hexa numbers, M is for Acceptance m is for Mask register )\n\n",argv[0]);fflush(stdout);
		return(1);
	}

	printf("\nWorking with ACR = ");PrintHexa(8,OriACR);printf("   AMR = ");PrintHexa(8,OriAMR);fflush(stdout);
	Eval();

	printf("\n\n");fflush(stdout);
#ifdef	USE_SORTING
	if(ResultsBuffer!=NULL) free((void*)ResultsBuffer);
#endif

return(0);
}

