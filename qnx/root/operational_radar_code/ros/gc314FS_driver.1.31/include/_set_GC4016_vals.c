#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "_prog_conventions.h"
#include "_regs_GC314FS.h"
#include "_regs_GC4016.h"
#include "_structures.h"
#include "_global.h"

#define PI 3.14159265359
#define GAIN_ADJ 10

short	cfir_coeffs[11]={-16,	48,	320,	355, 	-633,
	 	        -2214,	-2380,	988,	8474,	17206,
		 	21238};
short	pfir_coeffs[32]={	132,	182,	1,	-309,	-330,	119,
					560,	343,	-424,	-832,	-278,	696,
					960,	149,	-916,	-1132,	-212,	1075,
					1514,	421,	-1518,	-2374,	-673,	2529,
					3817,	726,	-4796,	-6888,	-650,	12967,
					26898,	32767};
short	resamp_coeffs[256];

float	blackman_cfir[11]={	0,	0.0092,	0.0402,	0.1014, 0.2008,	0.3400, 
					0.5098,	0.6892,	0.8492,	0.9602, 1.0};
float	blackman_pfir[32]={	0, 	0.0009, 0.0038, 0.0086, 0.0156, 0.0251,
					0.0374, 0.0527, 0.0715, 0.0939, 0.1203,
					0.1508, 0.1856, 0.2247, 0.2680, 0.3151,
					0.3657, 0.4194, 0.4754, 0.5330, 0.5912,
					0.6493, 0.7060, 0.7604, 0.8115, 0.8582,
					0.8995, 0.9346, 0.9627, 0.9883, 0.9958,
					1};
				

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

float setPFIR(short *pfircoeffs,float freq_in,float BW,int matched){
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


  PFIRgain=0;
  if(matched){
    wc=freq_in/BW/2;
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
    wc=pi*(float)BW/(float)freq_in;
    for(n=0;n<31;n++){
      pfircoeffs[n]=(short)(32767*window[n]*(pi/wc)*sin(wc*((float)n-31))/(pi*((float)n-31))+.49999);
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
	pfircoeffs[n]=(short)( (1/gaintemp)*(float)pfircoeffs[n]+0.499999 );
 	PFIRgain=PFIRgain+pfircoeffs[n];
    }
    pfircoeffs[31]=(short)( (1/gaintemp)*(float)pfircoeffs[31]+0.49999 );
    PFIRgain=2*PFIRgain+pfircoeffs[31];
    PFIRgain=PFIRgain/65536;
  }
  //printf("PFIRbad wc=%f\n",wc);
  return PFIRgain;
}

float setresamp(short *resampcoeffs,float freq_in,float BW){
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
  int length=128;
  float window[128];
	
  float resampgain;
  float gaintemp;
  double wp,ws,wc;
  int n;

  blackman(window,length);

  wc=pi*(float)BW/(float)freq_in;
  resampgain=0;
  for(n=0;n<64;n++){
    resampcoeffs[n]=(short)(2047*window[n]*(pi/wc)*sin(wc*((float)n-63.5))/(pi*((float)n-63.5))+.49999);
    resampgain=resampgain+resampcoeffs[n];
  }
  resampgain=2*resampgain/4096;
  if(resampgain>1){
    gaintemp=resampgain;
    resampgain=0;
    for(n=0;n<64;n++){
	resampcoeffs[n]=(short)( (1/gaintemp)*(float)resampcoeffs[n]+0.499999 );
 	resampgain=resampgain+resampcoeffs[n];
    }
    resampgain=2*resampgain;
    resampgain=resampgain/4096;
  }
  //printf("RESAMPbad wc=%f\n",wc);
  return resampgain;
}

//int _set_GC4016_vals(int BW, int fout, int freq, struct GC4016_channel_regs *GC4016){
int _set_GC4016_vals(struct channel_params *chanX, struct GC4016_channel_regs *GC4016, short *resampcoeffs, unsigned int *resampratio){

	int 		i, n;
	int 		FREQ;


	float		cfir_pass=0.5, cfir_stop=0.5, pfir_pass=0.5, pfir_stop=0.5;
	int		decimation, big_scale, shift=4, finalshift_adj,scale=0, coarse, cicscale;
	float		cic_gain_temp, wc, cfir_gain, pfir_gain, gain, gain_tmp,pi=3.14159;
	float		cicgain, CFIRgain, PFIRgain;
	float		CFIRfreq, PFIRfreq;
	float		resampgain, resampfreq;
	int		ndelay=16, fine_gain;
	int		finalshift=4;
	double		smpdlyF, Tdelay;
	int		smpdlyI;

	// parameters for each channel to get appropriate performance
	// calculate decimation
	/* If the resampler is used, then the decimation must be
  	   calculated differently.  The resampler performance
	   decreases when its input rate is less than 1.5 times
	   the signal bandwidth.  This deicmation calculation 
	   assures that the input bandwidth is always at least
	   1.5 (6/4) times the desired output rate (BW). See
	   note 1 on page 12 of the GC4016 manual. It should be
 	   noted, that when the resampler is used, there is an
	   introduced jitter of 1/(2*Ndelay)*output rate. See 
   	   page 12.  For our settings, this is a max jitter of
	   1/64th the output rate.
	*/
	//decimation=(int)((float)FCLOCK/(6*chanX->rate));
	decimation=(int)((float)FCLOCK/(100*chanX->rate));

	//printf("when calculated, decimation is %d, fclock is %d, freqout is %f\n", decimation, fclock, freqout);

  	/* Now that the decimation has been calculated, apply the restrictions
	   defined in the GC4096 Manual, pg.7. It should be noted that when the 
	   resampler is used, the decimation is less than when no resampler is used
	   (this is due to the factor of 10 in the decimation calculation, vs the
	   factor of 4 when the resampler is not used).  This means that the
	   decimation may be too small at large output frequencies.  This is not a
	   significant problem, so long as the output sample rate is >= Fsamp/32.
	   If the decimation is too small, it will be limited to the lowest allowed
	   value, and the performance of the resampler filter will degrade.  However,
	   if an output rate of >= Fsamp/32 is requested, then there will be significant
	   aliasing within the band of interest.
	*/
	if (decimation>4096){
         	//fprintf(stderr, "Decimation too great: %d, srate=%f\n",decimation, chanX->rate);
	 	decimation=4096;
  	}
  	if (decimation<8){
         	//fprintf(stderr, "Decimation too small: %d, srate=%f\n",decimation, chanX->rate);
	 	decimation=8;
  	}
	/* Calculate the delay throught the GC4016 chips so the delay can be later removed
	*/
	smpdlyF=0;
	Tdelay=0;
	if(chanX->matched) smpdlyF+=1.0;
	Tdelay=((2.5*decimation)+(0.5*decimation*21)+(decimation*63)+(2*decimation*8)+40)/FCLOCK;
	//Tdelay+=170e-6;
	smpdlyF=Tdelay*(double)chanX->rate;
	smpdlyI=(int)(smpdlyF+0.5);
	//smpdlyI+=4;
	smpdlyI+=1;
	/* If the decimation is greater than 3104, then the gain through
	   the CIC decimator will be greater than 1 if MIX20 is not disabled
	   and shift is not reduced.  A gain of greater than 1 in the CIC
	   will overflow the output of the CIC, clipping either I, Q, or both
	   if the input signal it large.  This next  routine calculates when
	   this condition may occur, and prevents sets MIX20 and shift 
	   appropriately (see pages 8,39 in the GC4016 Manual
	*/
	if (decimation>3104){
	  shift=0;
	  big_scale=(int)((62-5*log10(decimation)/log10(2)-shift-scale)/6);
	  if(big_scale>7) big_scale=7;
	  if(big_scale<0) big_scale=0;
	  cicscale=((big_scale<<3)+(scale)) & 0x3f;
	}
	else{
	  shift=4;
	  big_scale=(int)((62-5*log10(decimation)/log10(2)-shift-scale)/6);
	  if(big_scale>7) big_scale=7;
	  if(big_scale<0) big_scale=0;
	  cicscale=((0x1<<6)+(big_scale<<3))+(scale) & 0x7f;
	}
	/* Calculate the cicgain, and use coarse to adjust it accordingly.
	   See page 8 in GC4016 Manual
	*/
	cicgain=pow((float) decimation,5)*pow(2,shift+scale+6*big_scale-62);
	coarse=(int)(log10(1.0/cicgain)/log10(2));  
coarse+=3;  //ADDED BY TODD TO INCREASE THE GAIN
	if(coarse>7) coarse=7;
	if(coarse<0) coarse=0;
	/* Calculate the sample rates for the inputs of the CFIR, PFIR, and
	   resampler
	*/
	CFIRfreq=(float)FCLOCK/(float)decimation;
	PFIRfreq=CFIRfreq/2;
	resampfreq=ndelay*PFIRfreq/2;
	/* Again, if a matched filter is desired, the
	   PFIR will be set to match the sinc spectral shape of the ideal TX pulse.
	   The last parameter of 'setPFIR' tells the routine
	   to set the filter response equal to the spectrum of a pulse width of 1/freqout.
	   Setting that parameter to 0 tells 'setPFIR' to use an ideal lorpass filter with 
	   a two-sided BW of freqout. There is also a provision in 'setresamp' to set
	   the resampler coeffs appropriately for the resampler to be bypassed (see page
	   15 in GC4016 manual).  IF THE RESAMPLER IS BYPASSED, THEN THE OUTPUT DATA RATE
	   OF THE GC214PCI/TS WILL NOT NECESSARILY BE EXACTLY THE RATE REQUESTED.
	*/
	CFIRgain=0.999969482422;
	//CFIRgain=1.5428009;
	PFIRgain=setPFIR(pfir_coeffs,PFIRfreq,chanX->BW,chanX->matched);
	resampgain=setresamp(resamp_coeffs,resampfreq,chanX->BW);
	//finalshift=7;
        //finalshift=4; //ADDED BY TODD TO INCREASE GAIN
        finalshift=(int)( log10((float)ndelay/resampgain)/log10(2) );         // see reampler gain on page 15 of the GC4016 manual
        finalshift=finalshift+1;                                              // the +1 was added to keep the gain of the resampler > 1 to prevent saturation in the PFIR stage (where the finegain is added)
        if(finalshift>15) finalshift=15;                                      // limit on final shift (page 14 in GC4016 manual
        if(finalshift<0) finalshift=0;                                        // limit on final shift (page 14 in GC4016 manual

	resampgain=((resampgain*4096)/(ndelay*32768))*pow(2,finalshift);
	
	gain=cicgain * pow(2,coarse) * CFIRgain * PFIRgain * resampgain;      // calculate the gain without fine gain set to allow fine gain to be used to achieve a tital gain of 1
	fine_gain=(int) (2*1024/gain); 				// the factor of 2 compensates for adding 1 to finalshift, giving a gain of 1
        //fine_gain=fine_gain*8; //ADDED BY TODD TO INCREASE GAIN
        gain_tmp=16384.0/(float)fine_gain;
        if (GAIN_ADJ<gain_tmp){
                fine_gain=(int)(2.0*(float)GAIN_ADJ*1024.0/gain);
                finalshift_adj=0;
        }
        else{
                gain_tmp=(float)GAIN_ADJ-gain_tmp;
                finalshift_adj=(int)ceil(log10(gain_tmp)/log10(2));
                gain_tmp=GAIN_ADJ-pow(2,finalshift_adj);
                fine_gain=(int)(2.0*gain_tmp*1024.0/gain);
        }
        finalshift=finalshift+finalshift_adj;
        if(finalshift>15) finalshift=15;                                      // limit on final shift (page 14 in GC4016 manual
        if(finalshift<0) finalshift=0;                                        // limit on final shift (page 14 in GC4016 manual

	if (fine_gain>=16384) fine_gain=16384;					// limit on finegain (pg.9 in GC4016 manual
	if (fine_gain<=1) fine_gain=1;					// if negative or zero gain is needed, then signals will be wrong, so set fine gain to 1

	cicgain=pow((float) decimation,5)*pow(2,shift+scale+6*big_scale-62);


	FREQ=4294967296*chanX->freq/FCLOCK;	

// *******************************************************************************************************
	for (i=0;i<11;i++){
		GC4016->cfir_coeffs[i]=cfir_coeffs[i];
	}
	for (i=0;i<5;i++){
		GC4016->FILL[i]=-1;
	}
	for (i=0;i<32;i++){
		GC4016->pfir_coeffs[i]=pfir_coeffs[i];
	}
	for(i=0;i<64;i++){
		resampcoeffs[4*i+chanX->channel]=resamp_coeffs[i];
	}

	GC4016->phase		=0x0000;
	GC4016->freq_lsb	=(unsigned short)(FREQ & 0x0000ffff);
	GC4016->freq_msb	=(unsigned short)( (FREQ >>16) & 0x0000ffff);	
	for (i=0;i<5;i++){
		GC4016->FILL2[i]=-1;
	}
	//GC4016->ch_reset	=0x0c;	
	GC4016->ch_reset = 0x08 + (shift & 0x07);
	GC4016->ch_reset	=0x0c;	
	GC4016->freq_sync	=0x77;	
	GC4016->nco_sync	=0x72;	
	GC4016->zero_pad_mode_ctrl	=0x00;	
	GC4016->dec_flush_sync	=0x22;	
	GC4016->dec_ratio_lsb	=(char)( (decimation-1) & 0xff );	
	GC4016->dec_ratio_msb	=0x70 + (char)( ((decimation-1)>>8) & 0xf);	
	//GC4016->cic_scale	=((0x1<<6)+(big_scale<<3)+scale) & 0x7f;	
	GC4016->cic_scale	=cicscale;	
	GC4016->split_IQ	=0x00;	
	GC4016->cfir		=0x00 | (coarse<<4);	
	GC4016->pfir		=0x00;	
	GC4016->input		=0x00;// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!	
	GC4016->peak_ctrl	=0x00;	
	GC4016->peak_count	=0x00;	
	GC4016->fine_gain	=fine_gain;	
	resampratio[chanX->channel]=(int)((((float)FCLOCK/(4*(float)decimation))/(float)chanX->rate)*(float)0x04000000);
	return smpdlyI;
}
