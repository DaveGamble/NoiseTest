//
//  main.cpp
//  NoiseTest
//
// Contains random number generator from Numerical Recipes
//

#include <stdio.h>
#include <math.h>
#include "WavFile.h"
extern void rdft(int n, int isgn, double *a);

#define STD_DEV 1
#define EPS 1.2e-7
#define RNMX (1.0-EPS)
#define IM1 2147483563
#define IM2 2147483399
#define AM1 (1.0/IM1)
#define IMM1 (IM1-1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB1 32
#define NDIV1 (1+IMM1/NTAB1)

double
ran2(volatile long *idum)
{
	volatile int j;
	volatile long k;
	static volatile long idum2=123456789;
	static volatile long iy=0;
	static volatile long iv[NTAB1];
	volatile double temp;
	
	if (*idum <= 0)
	{
		if (-(*idum) < 1) *idum=1;
		else *idum = -(*idum);
		idum2=(*idum);
		for (j=NTAB1+7;j>=0;j--)
		{
			k=(*idum)/IQ1;
			*idum=IA1*(*idum-k*IQ1)-k*IR1;
			if (*idum < 0) *idum += IM1;
			if (j < NTAB1) iv[j] = *idum;
		}
		iy=iv[0];
	}
	
	k=(*idum)/IQ1;
	*idum=IA1*(*idum-k*IQ1)-k*IR1;
	if (*idum < 0) *idum += IM1;
	k=idum2/IQ2;
	idum2=IA2*(idum2-k*IQ2)-k*IR2;
	if (idum2 < 0) idum2 += IM2;
	j=iy/NDIV1;
	iy=iv[j]-idum2;
	iv[j] = *idum;
	if (iy < 1) iy += IMM1;
	if ((temp=AM1*iy) > RNMX) return RNMX;
	else return temp;
}

volatile long idnum_gauss2=0;
double gauss2_2()
{
	volatile double x1,x2,w,y1,y2;
	do
	{
		x1 = 2.0 * ran2(&idnum_gauss2) - 1.0;
		x2 = 2.0 * ran2(&idnum_gauss2) - 1.0;
		w = x1 * x1 + x2 * x2;
	}
	while ( w >= 1.0 );
	
	w = sqrt( (-2.0 * log( w ) ) / w );
	y1 = x1 * w;
	y2 = x2 * w;
	return y1/STD_DEV;
}

#define RATE 44100

int main(int argc, const char * argv[])
{
	if (argc==2)
	{
		WavFile *f=new WavFile(argv[1],false,1,RATE);
		for (int i=0;i<RATE*10;i++)
			f->AddSample((float)(gauss2_2()*0.15));
		delete f;
		return 0;
	}
	if (argc==4)
	{
		WavFile *f=new WavFile(argv[1]),*f2=new WavFile(argv[2]);
		int len1=f->GetSamples(),len2=f2->GetSamples();
		int maxlen=(len1>len2)?len1:len2;
		int p2=1;while (p2<maxlen) p2*=2;p2*=2;
		double *buffer1=new double[p2],*buffer2=new double[p2];
		memset(buffer1,0,p2*sizeof(double));memset(buffer2,0,p2*sizeof(double));
		for (int i=0;i<len1;i++)buffer1[i]=f->GetSample(i);
		for (int i=0;i<len2;i++)buffer2[i]=f2->GetSample(i);
		delete f;delete f2;

		rdft(p2,1,buffer1);
		rdft(p2,1,buffer2);
		buffer1[0]=buffer2[0]/buffer1[0];
		buffer1[1]=buffer2[1]/buffer1[1];
		for (int i=2;i<p2;i+=2)
		{
			// a+ib * c+id = ac -bd + i(ad+bc)
			// c+id / (a+ib) = (a-ib)(c+id) / (a*a+b*b)
			double imag=1.0/(buffer1[i]*buffer1[i]+buffer1[i+1]*buffer1[i+1]);
			double re=(buffer1[i]*buffer2[i]+buffer1[i+1]*buffer2[i+1])*imag;
			double im=(buffer1[i]*buffer2[i+1]-buffer1[i+1]*buffer2[i])*imag;
			buffer1[i]=re;
			buffer1[i+1]=im;
		}
		rdft(p2,-1,buffer1);
		double max=0;
		for (int i=0;i<p2;i++) if (fabs(buffer1[i])>max) max=fabs(buffer1[i]);
		double k=1.0/max;
		for (int i=0;i<p2;i++) buffer1[i]*=k;
		WavFile *f3=new WavFile(argv[3],false,1,RATE);
		for (int i=0;i<p2;i++) f3->AddSample((float)buffer1[i]);
		delete f3;delete[] buffer1;delete[] buffer2;
		return 0;
	}
	fprintf(stderr,"Usage:\nGenerate noise with: NoiseTest <outfile.wav>\nTest with: NoiseTest <noise.wav> <measured.wav> <out.wav>\n\n");


    return 0;
}

