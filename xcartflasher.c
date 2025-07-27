/*--------------------------------------------------------------------*/
/* XCartFlasher                                                       */
/* by GienekP                                                         */
/* (c) 2025                                                           */
/*--------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
/*--------------------------------------------------------------------*/
#include "xcartflasher.h"
#include "block.h"
#include "last.h"
/*--------------------------------------------------------------------*/
typedef unsigned char U8;
const unsigned int carsize=(4*1024*1024);
/*--------------------------------------------------------------------*/
char ach(char c)
{
	char r=0;
	if ((c>32) && (c<96)) {r=c+32;};
	if ((c>='a') && (c<='z')) {r=c;};
	return r;
}
/*--------------------------------------------------------------------*/
void save(const char *filename, U8 *data, unsigned int size)
{
	unsigned int i,j;
	FILE *pf;
	pf=fopen(filename,"wb");
	if (pf)
	{
		printf("Prepare XCart flasher.\n");
		fwrite(xcartflasher_xex,sizeof(U8),sizeof(xcartflasher_xex),pf);
		printf("Save Menu\n");
		for (j=0; j<255; j++)
		{
			unsigned int noempty=0;
			block_xex[7]=j;
			for (i=0; i<16384; i++)
			{
				U8 d=data[16384*j+i];
				block_xex[24+i]=d;
				if ((d!=0xff) || (j==0)) {noempty=1;};
			};
			if (noempty)
			{
				fwrite(&block_xex[2],sizeof(U8),sizeof(block_xex)-2,pf);
				printf("Save bank:$%02X Sum=$%02X XOR=$%02X\n",j,xcartflasher_xex[6+j],xcartflasher_xex[6+256+j]);
			}
			else
			{
				printf("Avoid bank %02X\n",j);
			};
		};
		fwrite(&last_xex[2],sizeof(U8),sizeof(last_xex)-2,pf);
		fclose(pf);
		printf("Save XEX \"%s\".\n",filename);
	};	
}
/*--------------------------------------------------------------------*/
void clear(U8 *data, unsigned int size)
{
	unsigned int i;
	for (i=0; i<size; i++) {data[i]=0xFF;};
}
/*--------------------------------------------------------------------*/
void calcsums(U8 *data, const char *title)
{
	unsigned int j,i;
	for (j=0; j<256; j++)
	{
		U8 sum=0,xor=0;
		for (i=0; i<16384; i++)
		{
			U8 b=data[16384*j+i];
			sum+=b;
			xor^=b;
		};
		xcartflasher_xex[6+j]=sum;
		xcartflasher_xex[6+256+j]=xor;
	};
	printf("Calc checksums.\n");
	for (i=0; i<32; i++)
	{
		U8 c=title[i];
		if (c) {xcartflasher_xex[6+512+i]=ach(c);} else {i=32;};
	};
	printf("Add title \"%s\".\n",title);
}
/*--------------------------------------------------------------------*/
unsigned int toInt(const U8 *str)
{
	unsigned int i,ret=0;
	for (i=0; i<4; i++)
	{
		ret<<=8;
		ret|=(unsigned int)(str[i]);
	};
	return ret;
}
/*--------------------------------------------------------------------*/
unsigned int car(U8 *data, unsigned int size)
{
	unsigned int ret=size;
	if (toInt(&data[0])==0x43415254)
	{
		unsigned int i,sum=0,rs=toInt(&data[8]);
		for (i=16; i<size; i++) {sum+=data[i];};
		if (sum==rs)
		{
			for (i=0; i<(size-16); i++) {data[i]=data[i+16];};
			for (i=(size-16); i<size; i++) {data[i]=0xFF;};
			ret-=16;
			printf("Detect CAR header, convert to bin.\n");
		};
	};
	return ret;
}
/*--------------------------------------------------------------------*/
unsigned int load(const char *filename, U8 *data, unsigned int size)
{
	unsigned int ret=0;
	FILE *pf;
	pf=fopen(filename,"rb");
	if (pf)
	{
		ret=fread(data,sizeof(U8),size,pf);
		fclose(pf);
		printf("Load \"%s\" size %i bytes.\n",filename,ret);
	};
	return ret;
}
/*--------------------------------------------------------------------*/
void flashBuilder(const char *title, const char *filein, const char *fileout, U8 *cardata, unsigned int maxsize)
{
	unsigned int size;
	clear(cardata,maxsize);
	size=load(filein,cardata,maxsize);
	if (size>0)
	{
		size=car(cardata,size);
		calcsums(cardata,title);
		save(fileout,cardata,size);
	}
	else
	{
		printf("Can't convert \"%s\"\n",filein);
	};
}
/*--------------------------------------------------------------------*/
int main( int argc, char* argv[] )
{
	U8 *cardata;
	cardata=(U8 *)malloc((carsize+16)*sizeof(U8));
	if (cardata)
	{	
		printf("XCart Flasher - ver: %s\n",__DATE__);
		switch (argc)
		{
			case 4:
			{
				flashBuilder(argv[1],argv[2],argv[3],cardata,carsize+16);
			} break;
			default:
			{
				printf("(c) GienekP\n");
				printf("use:\nxcartflasher title file.bincar flasher.xex\n");
			} break;
		};
		free(cardata);
		cardata=NULL;
	}
	else
	{
		printf("Memory alocation problem.\n");
	};
	return 0;
}
/*--------------------------------------------------------------------*/
