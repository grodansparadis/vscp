/*
cc cleanfile.c -o cleanfile
*/

#include <stdio.h>
#include <sys/types.h>


int main(int argc, char *argv[])
{
unsigned int	a,b=0;
FILE		*fp_in=NULL,*fp_out=NULL;
long		FileL,ctr;

	if(argc>1)
	{
		if((fp_in=fopen(argv[1],"r"))!=NULL)
		{
			fseek(fp_in,0L,SEEK_END);
			FileL=ftell(fp_in);
			fseek(fp_in,0L,SEEK_SET);	
		}
		if(argv[2]!=NULL)
		{
			if(argv[2][0]!='\0')
			{
				sscanf(argv[2],"%u",&b);
			}
		}
	}
	else
	{
		printf("Need a source file...");fflush(stdout);
		return(1);
	}

	printf("\nCutting first %d bytes...",b);fflush(stdout);

	if((fp_out=fopen("out.txt","w"))==NULL)
	{
		fclose(fp_in);
		printf("\nPRC WRITE : Can not open 'out.txt' for writing...");fflush(stdout);
		return(1);
	}

	for(ctr=0;ctr<FileL;ctr++)
	{
		a=fgetc(fp_in);
		if((a!=0)&&(a!=' ')&&(ctr>=b))
		{
			if((a<' ')&&(a>126)) a='.';
			if(a==0x0d) a=0x0a;
			fputc(a,fp_out);
		}
	}

	fflush(fp_out);
	fclose(fp_out);
	fclose(fp_in);

return(0);
}
