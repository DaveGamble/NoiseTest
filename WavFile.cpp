#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "WavFile.h"
#define _fgetc(x) (fgetc(x)&255)

void WRITEINT(FILE *f,int x)
{ fputc(x&255,f);fputc((x>>8)&255,f);fputc((x>>16)&255,f);fputc(x>>24,f); }

int READINT(FILE *f)
{ int x=0; x|=_fgetc(f);x|=_fgetc(f)<<8;x|=_fgetc(f)<<16;x|=_fgetc(f)<<24;return x;}

void WRITESAMP(FILE *f,int x)
{ fputc(x&255,f);fputc((x>>8)&255,f);fputc((x>>16)&255,f); }

void WRITESHORT(FILE *f,int x) 
{ fputc(x&255,f);fputc((x>>8)&255,f); }

void WRITESHORTb(FILE *f,int x) 
{ fputc((x>>8)&255,f);fputc(x&255,f); }


void WRITEHDR(FILE *f,int samples,int channels,int rate)
{
	int b,size;
	
	b=4;
	size=samples*b;
	fseek(f,0,SEEK_SET);
	
	fputc('R',f);fputc('I',f);fputc('F',f);fputc('F',f);
	WRITEINT(f,size+4+24+8);
	fputc('W',f);fputc('A',f);fputc('V',f);fputc('E',f);

	fputc('f',f);fputc('m',f);fputc('t',f);fputc(' ',f);
	WRITEINT(f,16);
	WRITESHORT(f,3);
	WRITESHORT(f,channels);
	WRITEINT(f,rate); 
	WRITEINT(f,rate*b*channels);
	WRITESHORT(f,b*channels);
	WRITESHORT(f,32);
	
	fputc('d',f);fputc('a',f);fputc('t',f);fputc('a',f);
	WRITEINT(f,size);
	fseek(f,0,SEEK_END);
}

WavFile::WavFile(const char *na,bool re,int c,int r,bool ca)
: f(0)
, samples(0)
, channels(c)
, rate(r)
, read(re)
, current(0)
, data(0)
, cached(ca)
{
	strcpy(name,na);
	if (read)
	{
		f=fopen(name,"rb");
		// first thing... find the fmt chunk and the data chunk.
		pfmt=0;pdata=0;
		fseek(f,12,SEEK_SET);
		while (!pfmt || !pdata)
		{
			long chunk_id=READINT(f);
			if (chunk_id==0x20746D66)
				pfmt=ftell(f)-4;
			else if (chunk_id==0x61746164)
				pdata=ftell(f)-4;

			long size=READINT(f);
			fseek(f,size,SEEK_CUR);
		}
		
		fseek(f,pfmt+8,SEEK_SET);
		channels=READINT(f)>>16;
		rate=READINT(f);
		READINT(f);
		bytesize=READINT(f)>>19;

		fseek(f,pdata+4,SEEK_SET);
		samples=READINT(f)/(bytesize);
		fseek(f,pdata+8,SEEK_SET);

		if (cached)
		{
			data=new float[samples];
			if (bytesize==4)
			{
				for (int i=0;i<samples;i++)	{
					int x=READINT(f);data[i]=(float)*((float*)&x);
				}
			}
			else {
				float scalar=powf(2.f,-31.f);
				switch (bytesize)
				{
				case 3:	for (int i=0;i<samples;i++) {int x=(_fgetc(f)<<8)|(_fgetc(f)<<16)|(_fgetc(f)<<24);data[i]=x*scalar;}	break;
				case 2:	for (int i=0;i<samples;i++) {int x=(_fgetc(f)<<16)|(_fgetc(f)<<24);data[i]=x*scalar;}					break;
				case 1:	for (int i=0;i<samples;i++) {int x=(_fgetc(f)<<24)^0x80000000;data[i]=x*scalar;}						break;
				}
			}

			fclose(f);

			f=0;
		}
	}
}

WavFile::~WavFile() { Close(); delete data;}

void	WavFile::Close()
{
	if (cached)
	{
		if (!read)
		{
			if (!f)		f=fopen(name,"w+b");
			WRITEHDR(f,samples,channels,rate);
			
			for (size_t i=0;i<(size_t)samples;i++)
			{
				float s=dataw[i];
				int *d=(int*)&s;
				WRITEINT(f,*d);
			}
			dataw.clear();
			fclose(f);
			f=0;
		}
		else if (f)
		{
			fclose(f);f=0;
		}	
	}
	else
	{
		if (f)
		{
			if (!read)	WRITEHDR(f,samples,channels,rate);
			fclose(f);
			f=0;
		}
	}
}

void	WavFile::AddSample(float s)
{
	if (read) return;
	if (cached)
	{
		dataw.push_back(s);
	}
	else
	{
		if (!f)		{f=fopen(name,"w+b");WRITEHDR(f,samples,channels,rate);/*fseek(f,44,SEEK_SET);*/current=0;}

		int *i=(int*)&s;
		WRITEINT(f,*i);
	}

	current++;
	samples++;
}

float	WavFile::ReadSample()
{
	if (cached)
	{
		if (current<samples)
			return data[current++];
		else
			return 0;	
	}
	else
	{
		float s;
		if (!f)	{f=fopen(name,"r+b");fseek(f,pdata+8,SEEK_SET);current=0;}	

		int x=READINT(f);
		s=(float)*((float*)&x);
		current++;
		return s;
	}
}
