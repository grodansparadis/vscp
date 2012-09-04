/*
cc cleanfile.c -o cleanfile
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

char		*sb=NULL;
FILE		*fp=NULL;

unsigned int GetNumero(unsigned int ofs)
{
unsigned int	b=ofs,muler=1,valu=0;

	while((sb[b]!=',')&&(sb[b]!=10)&&(sb[b]!=0)) ++b;
	if(sb[b]!=0)
	{
		--b;
		while(b>=ofs)
		{
			if(sb[b]>='A')
				valu+=(((unsigned int)((sb[b]-'A')+10))*muler);
			else
				valu+=((unsigned int)(sb[b]-'0')*muler);
			muler*=16;
			--b;
		}		
	}
	else
		return(0xFFFFFFFF);
return(valu);
}

void PrintHexa(int NumOfDigits,int Number)
{
register int a,b;

	for(a=(NumOfDigits-1);a>=0;a--)
	{
		b=((Number>>(a<<2))&0xF);
		if(b<10)
			fprintf(fp,"%d",b);
		else
			fprintf(fp,"%c",('A'+b-10));
	}
}

int main(int argc, char *argv[])
{
unsigned int	a,b,c,d;
long		FileL,ctr;

	if(argc>1)
	{
		if((fp=fopen(argv[1],"r"))!=NULL)
		{
			fseek(fp,0L,SEEK_END);
			FileL=ftell(fp);
			fseek(fp,0L,SEEK_SET);	
		}
	}
	else
	{
		printf("Need a source file...");fflush(stdout);
		return(1);
	}

	printf("\n'%s' is %d long... loading...",argv[1],FileL);fflush(stdout);

	if((sb=(char*)malloc((size_t)FileL))==NULL)
	{
		fclose(fp);
		printf("\n\nCan not allocate memory...");fflush(stdout);
		return(0);
	}
	
	if((fread(sb,sizeof(char),(size_t)FileL,fp))!=(size_t)FileL)
	{
		free((void*)sb);
		fclose(fp);
		printf("\n\nCan not load file...");fflush(stdout);
		return(0);
	}
	fclose(fp);

	printf("done...\n");

	if((fp=fopen("out.txt","w"))==NULL)
	{
		printf("\nPRC WRITE : Can not open 'out.txt' for writing...");fflush(stdout);
		free((void*)sb);
		return(1);
	}

	a=0;
	while(a<FileL)
	{
		while((a<FileL)&&(sb[a]!=0x0A)) ++a;
		if(sb[a]==0xA)
		{
			b=0;
			while(b<3)
			{
				while(sb[a]!=',') ++a;
				++a;++b;
			}
			b=GetNumero(a);
			fprintf(fp,"t");
			PrintHexa(3,b);
			b=a;
			c=0;
			while((b<FileL)&&(sb[b]!=0x0A))
			{
				if(sb[b]==',')
				{
					if((sb[b+1]!=',')&&(sb[b+1]!=0x0D)&&((b+1)<FileL)) ++c;
				}
				++b;
			}
			if(b<FileL)
			{
				fprintf(fp,"%1x",(unsigned char)c);
				d=0;
				while(d<c)
				{
					while(sb[a]!=',') ++a;
					++a;
					b=GetNumero(a);
					PrintHexa(2,b);
					++d;++a;
				}
			}
			fprintf(fp,"%c",0xA);
		}
	}

	free((void*)sb);
	fflush(stdout);

// 33,2.012232,0.00025,30,84,0,A0,0,0,20,0,DC -> t03088400A000002000DC
// 0x0D, 0x0A

	fflush(fp);
	fclose(fp);

return(0);
}
