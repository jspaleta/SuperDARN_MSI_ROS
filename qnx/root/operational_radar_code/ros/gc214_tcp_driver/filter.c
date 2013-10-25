#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#define PI 3.14159265359
#define TXBW 50000

/*
 $Log: filter.c,v $
 Revision 1.3  2009/10/22 00:32:03  jspaleta
 jspaleta: gc214 stuff

 Revision 1.2  2009/10/22 00:22:22  jspaleta
 jspaleta: gc214 stuff

 Revision 1.1  2009/10/19 22:55:59  jspaleta
 jspaleta: old school gc214 driver

 Revision 1.1  2005/07/25 15:12:45  barnes
 Initial revision

*/
void hann(float *hann,int N){
  /* This code calculates a Hann window */
  float M;
  int n;

  M=(N-1)/2;
  for(n=0;n<N;n++){
    hann[n]=0.5*( 1 + cos( 2*PI*((float)n-M) /(2*M+1) ) );
  }
}

void hamming(float *hamming,int N){
  /* This code calculates a Hamming window */
  float M;
  int n;

  M=(N-1)/2;
  for(n=0;n<N;n++){
    hamming[n]=0.54+0.46*cos( (2*PI*((float)n-M))/(2*M+1) );
  }
}
void blackman(float *blackman,int N){
  /* This code calculates a Blackman window */
  float M;
  int n;

  M=(N-1)/2;
  for(n=0;n<N;n++){
    blackman[n]=0.42 + 0.5*cos( (2*PI*((float)n-M))/(2*M+1) ) + 0.08*cos( (4*PI*((float)n-M))/(2*M-1) );
  }
}

float setCFIR(int *cfircoeffs,float freq_in,float Fpass,float Fstop){
  /* This routine calculates coefficients for the CFIR filter.
     Because this filter is short, it is not very useful for
     signal spectrum shaping.  Its purpose is to provide anti-
     aliasing for it's decimation by 2, and to compensate for
     droop in the spectrum of the CIC (the CIC looks like a very
     broad sinc in frequency.  TI has caclualted coefficient sets
     to account for this droop, and provided them in the manual
     for the GC4016 (pg 57).  Since this droop is not well defined
     mathematically (at least not provided in the GC4016 manual), 
     the coefficients in the manual seem to work better than
     caclulated coefficients. I have commented-out the code
     to calculate coeffs if there is ever a desire to use it.
     I have also added a routine to scale the coefficients if the 
     gain of this filter is greater than 1.  A gain greater than 1
     causes an overflow in the output for large input signals; This
     is NOT accounted for in the coeffiecients given in the GC4016
     manual.  I have included CFIR17 and CFIR34 from the manual.
     CFIR17 is only EVER-SO-SLIGHTLY better, but is too narrow 
     for full bandwidth output if the resampler is not used, so I
     have hard coded CFIR34 into this routine.  CFIR17 can be used
     if the resampler is also being used (with decimation=Fsamp/(10*freqout).
  */ 
  
  int   cfir34[]={-24,	74,	494,	548, 	-977,
	 	 -3416,	-3672,	1525,	13074,	26547,
		 32767};

 
  float CFIRgain,gaintemp;
  int n;
  
  CFIRgain=0;
  for(n=0;n<10;n++){
    cfircoeffs[n]=cfir34[n];
    CFIRgain=CFIRgain+cfircoeffs[n];
  }
  cfircoeffs[10]=32767;
  CFIRgain=2*CFIRgain+cfircoeffs[10];
  CFIRgain=CFIRgain/65536;

 /* if CFIR gain > 1, and large signals, then overflows occur.  To absoluteley prevent
    overflows, coeffs must be chosen such that CFIRgain<=1 (see pg 8 in gc4016 manual) */

  if(CFIRgain>1){
    gaintemp=CFIRgain;
    CFIRgain=0;
    for(n=0;n<10;n++){
	cfircoeffs[n]=(int)( (1/gaintemp)*(float)cfircoeffs[n]+0.499999 );
        CFIRgain=CFIRgain+cfircoeffs[n];
    }
    cfircoeffs[10]=(int)( (1/gaintemp)*(float)cfircoeffs[n]+0.499999 );
    CFIRgain=2*CFIRgain+cfircoeffs[10];
    CFIRgain=CFIRgain/65536;
  }
  return CFIRgain;
}


float setPFIR(int *pfircoeffs,float freq_in,float Fpass,float Fstop,int matched){
  /* This routine calculates the filter coefficients for the PFIR
     filter.  The PFIR filter is a longer filter which is used for 
     most of the spectrum shaping in the GC4016.  This is the 
     'working filter'.  This routine calculates the ideal coefficients
     for a desired bandwidth, then windows those coeffs to minimize
     Gibbs Phenomeonon.  There are three windows available (coded);
     Hann, Hamming, and Blackman.  I am using a Blackman because
     the out-of-band rejection is better.  The fall-off is less steep,
     but for SuperDARN, I think out of band signals will cause more
     harm than very near band noise.  The PFIR coeffs provided on page
     58 are for generic bandwidths (and they are pretty crappy filters
     at that).  This routine calculate the coeffs for a specifice bandwith.
     Also, a routine has been added to keep the gain
     of the PFIR less than 1.  Finally, there is a routine to calculate
     coefficients for a matched filter with the requested bandwidth.
  */
  
  int pi=PI;
  int length=63;
  float window[63];
  float PFIRgain, gaintemp;
  double wp,ws,wc;
  int n;
  
  blackman(window,length);


  wp=pi*(float)Fpass/(float)freq_in;
  ws=pi*(float)Fstop/(float)freq_in;
  wc=(wp+ws)/2;
  PFIRgain=0;
  if(matched){
    wc=freq_in/((Fpass+Fstop)/2)/2;
    //printf("freq_in=%f, wc=%f\n",freq_in,wc);
    for(n=0;n<31;n++){
      if(n>ceil(31-wc)){
	pfircoeffs[n]=32767;
      }
      else{
	pfircoeffs[n]=0;
      }
      PFIRgain=PFIRgain+pfircoeffs[n];
    }
    pfircoeffs[31]=32767;
    PFIRgain=2*PFIRgain+pfircoeffs[31];
    PFIRgain=PFIRgain/65536;
  }
  else{
    for(n=0;n<31;n++){
      pfircoeffs[n]=(int)(32767*window[n]*(pi/wc)*sin(wc*((float)n-31))/(pi*((float)n-31))+.49999);
      PFIRgain=PFIRgain+pfircoeffs[n];
    }
    pfircoeffs[31]=32767;
    PFIRgain=2*PFIRgain+pfircoeffs[31];
    PFIRgain=PFIRgain/65536;
  }

 /* if PFIR gain > 1, and large signals, then overflows occur.  To absoluteley prevent
    overflows, coeffs must be chosen such that PFIRgain<=1 */

  if(PFIRgain>1){
    gaintemp=PFIRgain;
    PFIRgain=0;
    for(n=0;n<31;n++){
	pfircoeffs[n]=(int)( (1/gaintemp)*(float)pfircoeffs[n]+0.499999 );
 	PFIRgain=PFIRgain+pfircoeffs[n];
    }
    pfircoeffs[31]=(int)( (1/gaintemp)*(float)pfircoeffs[31]+0.49999 );
    PFIRgain=2*PFIRgain+pfircoeffs[31];
    PFIRgain=PFIRgain/65536;
  }
  //printf("PFIRbad wc=%f\n",wc);
  return PFIRgain;
}

float setresamp(short *resampcoeffs,float freq_in,float Fpass,float Fstop,int filtnum,int bypass){
  /* The resampler is truly intended to allow oversampling of
     already band limited data (see description on page 12 of
     the GC4016 manual).  For the purposes here, we do not
     want to oversample band-limited data (our data rate is
     usually equal to our two-sided bandwidth). So, we wish
     to use the resampler only to provide an ACCURATE, ARBITRARY
     baseband rate (arbitrary in that we do not want to be limited
     to an integer division of Fsamp).  Because of this, and the
     band limitation of the resampler described on pgs 12 and 13
     in the GC4016 manual, the resampler does not add significantly
     to the spectrum shaping of out received signal.  It is however
     nesessary to filter the zero-padded input that is decimated to 
     effectively resample the data.  Setting the BW of the resampler
     equal to our desired BW adds attenuation near the edge of the 
     band of interest, and the resampler filter does NOT significantly
     help reduce out-of-band rejection, so I have set the band width 
     of the coeffs to be 2 times the requested band width (see comments
     for CFIR coeffs for more argument on this).
  */
  
  int pi=PI;
  int length=256;
  float window[256];
	
  float resampgain;
  double wp,ws,wc;
  int n;

  blackman(window,length);

  wp=pi*(float)Fpass/(float)freq_in;
  ws=pi*(float)Fstop/(float)freq_in;
  wc=(wp+ws);  // divide by 2 omitted, effectively multiplying by 2
  resampgain=0;
  if(bypass){
    resampcoeffs[0]=1024;
    for(n=1;n<256;n++){
	resampcoeffs[n]=0;
    }
    resampgain=0.3125;
  }
  else{
    for(n=0;n<128;n++){
      resampcoeffs[2*n+filtnum]=(short)(2047*window[n]*(pi/wc)*sin(wc*((float)n-127.5))/(pi*((float)n-127.5))+.49999);
      resampgain=resampgain+resampcoeffs[2*n+filtnum];
    }
    resampgain=2*resampgain/32768;
  }
  //printf("RESAMPbad wc=%f\n",wc);
  return resampgain;
}
