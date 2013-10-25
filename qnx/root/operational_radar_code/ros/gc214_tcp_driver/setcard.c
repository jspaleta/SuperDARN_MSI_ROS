#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "control_program.h"
#include "global_server_variables.h"
#include "utils.h"
#include "gc214_defines.h"
#include "gc4016.h"
#include "filter.h"
#define RESAMP_BYPASS 0
#define MATCHED 1

extern int verbose;
//int verbose=2;

void setcard(struct GC4016Global *global,
  struct GC4016Channel *chana, 
  struct GC4016Channel *chanb, 
  struct GC4016Channel *chanc, 
  struct GC4016Channel *chand,
  int Fsamp,int *smpdlyAB,int *smpdlyCD,
  int *cfircoeffsAB,int *cfircoeffsCD,int *pfircoeffsAB,int *pfircoeffsCD, 
  short *resampcoeffs) 
{

  struct GC4016Channel *chan;
  int decimation;
  int coarse,finalshift,coarse_tmp, finalshift_tmp;
  float freqout;
  float CFIRgain, CFIRfreq;
  float PFIRgain, PFIRfreq;
  float cicgain;
  float resampgain, resampfreq;
  int cicscale;
  int bigscale,shift,scale=0,ndelay;
  float gain,gain_tmp;
  float Tdelay,smpdlyF;
  int finegain,finegain_tmp, finalshift_adj;
  struct timespec start,stop;
  int matched;

  /* CALCULATE PARAMETERS FOR CHANNELS A&B *********************************************/
  /* Channels A and B are set up for use with typical superDARN
     modes.  Channel A is dedicated to input CH1, which is connected
     to the main array of the radar.  Channel B is dedicated to input
     CH2, which is connected to the secondary/interferometry array.
  */

  /* If a matched filter is desired, then the sample rate into
     the CFIR filter must be great enough to allow a good filter
     with a cutoff at the TX bandwidth to be implemented.  This
     requres calculating the decimation assuming a basband rate
     equale to the TX bandwidth (50kHz for most radars).  The re-
     sampler is used to decimate to the desired baseband rate. If
     a matched filter is not desired, then out if band rejection 
     can be improved by setting the bandwidths of the CFIR and PFIR
     to be similar.  The necessitates calculating the decimation
     differently when a matched filter is used.  
  */
  if (verbose > 1) printf("SetCard A:: Rate: %lf matched: %d\n",chana->rate,chana->match);
  if (verbose > 1) printf("SetCard B:: Rate: %lf matched: %d\n",chanb->rate,chanb->match);
  if (verbose > 1) printf("SetCard C:: Rate: %lf matched: %d\n",chanc->rate,chanc->match);
  if (verbose > 1) printf("SetCard D:: Rate: %lf matched: %d\n",chand->rate,chand->match);
  matched=chana->match;
  freqout=chana->rate;
  if (verbose > 1) printf("SetCard FreqoutAB: %lf matched: %d\n",freqout,matched);
  if(RESAMP_BYPASS){
  	decimation=(int) ((float) Fsamp/(4*freqout));
        ndelay=1;
        if(matched){
	   fprintf(stderr,"Cannot use matched filter with resampler bypass: \n");
	}
        matched=0;
  }
  else if(matched){
  	decimation=(int) ((float) Fsamp/(4*50000));
	ndelay=32;
        matched=1;
  }
  else{
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
  	decimation=(int) ((float) Fsamp/(6*freqout));
        ndelay=32;
  	matched=0;
  }

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
	 decimation=4096;
         fprintf(stderr, "Decimation too great: \n");
  }
  if (decimation<8){
	 decimation=8;
         fprintf(stderr, "Decimation too small: \n");
  }
  /* Now that the decimation is known, the sample delay through
     the receiver card can be calculated.  This is basedd on the delay
     throught tht receiver as defined in the GC4016 manula on pg. 25.
     This delay is calculated (now that we use the resampler, the output
     data rate is NOT directly related to the decimation, so this is NOT
     a fixed number of samples, regardless of output rate).  Two 'samples'
     are added to account for the header (64 bits), and the rise time of the
     TX pulse is removed (if we are sampleing very fast, the those samples
     in the rise of the pulse are not considered - this is crucial for 
     range calculations in the oversampled data).
  */
  smpdlyF=0;
  Tdelay=0;
  if(matched) smpdlyF+=1;
  Tdelay+=((2.5*decimation)+(0.5*decimation*21)+(decimation*63)+(2*decimation*8)+40)/Fsamp;
  smpdlyF+=Tdelay*(float)freqout;
  smpdlyF+=2;					// skip two samples for header
  *smpdlyAB=(int)(smpdlyF+0.5);
  if (verbose > 1) printf("smpdlyAB address: 0x%x  Value: %d\n",smpdlyAB,*smpdlyAB);
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
    bigscale=(int)((62-5*log10(decimation)/log10(2)-shift-scale)/6);
    if(bigscale>7) bigscale=7;
    if(bigscale<0) bigscale=0;
    cicscale=((bigscale<<3)+(scale)) & 0x3f;
  }
  else{
    shift=4;
    bigscale=(int)((62-5*log10(decimation)/log10(2)-shift-scale)/6);
    if(bigscale>7) bigscale=7;
    if(bigscale<0) bigscale=0;
    cicscale=((0x1<<6)+(bigscale<<3))+(scale) & 0x7f;
  }

  /* Calculate the cicgain, and use coarse to adjust it accordingly.
     See page 8 in GC4016 Manual
  */
  cicgain=pow((float) decimation,5)*pow(2,shift+scale+6*bigscale-62);
  coarse=(int)(log10(1.0/cicgain)/log10(2));  
  if(coarse>7) coarse=7;
  if(coarse<0) coarse=0;

  /* Calculate the sample rates for the inputs of the CFIR, PFIR, and
     resampler
  */
  CFIRfreq=(float)Fsamp/(float)decimation;
  PFIRfreq=CFIRfreq/2;
  resampfreq=ndelay*PFIRfreq/2;


  /* Again, if a matched filter is desired, then the CFIR and resampler will
     be set to 50kHz BW (to match the filter on the transmitter), while the 
     PFIR will be set to match the sinc spectral shape of the ideal TX pulse.
     The last parameter of 'setPFIR' tells the routine
     to set the filter response equal to the spectrum of a pulse width of 1/freqout.
     Setting that parameter to 0 tells 'setPFIR' to use an ideal lorpass filter with 
     a two-sided BW of freqout. There is also a provision in 'setresamp' to set
     the resampler coeffs appropriately for the resampler to be bypassed (see page
     15 in GC4016 manual).  IF THE RESAMPLER IS BYPASSED, THEN THE OUTPUT DATA RATE
     OF THE GC214PCI/TS WILL NOT NECESSARILY BE EXACTLY THE RATE REQUESTED.
  */
  if(matched){
  	CFIRgain=setCFIR(cfircoeffsAB,CFIRfreq,50000,50000);
  	PFIRgain=setPFIR(pfircoeffsAB,PFIRfreq,10000,10000,1);
  	resampgain=setresamp(resampcoeffs,resampfreq,25000,25000,0,RESAMP_BYPASS);
  }
  else{
  	CFIRgain=setCFIR(cfircoeffsAB,CFIRfreq,freqout,freqout);
  	PFIRgain=setPFIR(pfircoeffsAB,PFIRfreq,freqout,freqout,0);
  	resampgain=setresamp(resampcoeffs,resampfreq,freqout,freqout,0,RESAMP_BYPASS);
  }

  /* Calculate a number of parameters for the GC4016 Channels
  */
  finalshift=(int)( log10((float)ndelay/resampgain)/log10(2) );		// see reampler gain on page 15 of the GC4016 manual
  finalshift=finalshift+1;  						// the +1 was added to keep the gain of the resampler > 1 to prevent saturation in the PFIR stage (where the finegain is added)
  if(finalshift>15) finalshift=15;					// limit on final shift (page 14 in GC4016 manual
  if(finalshift<0) finalshift=0;					// limit on final shift (page 14 in GC4016 manual
  resampgain=(resampgain/ndelay)*pow(2,finalshift);

  gain=cicgain * pow(2,coarse) * CFIRgain * PFIRgain * resampgain;      // calculate the gain without fine gain set to allow fine gain to be used to achieve a tital gain of 1
//  printf("Gain is %f\n", gain);
  finegain=(int) (2*1024/gain); 				// the factor of 2 compensates for adding 1 to finalshift, giving a gain of 1

	gain_tmp=16384.0/(float)finegain;
	if (GAIN_ADJ<gain_tmp){
 		finegain=(int)(2.0*(float)GAIN_ADJ*1024.0/gain);
		finalshift_adj=0;
	}
	else{
		gain_tmp=(float)GAIN_ADJ-gain_tmp;
		finalshift_adj=(int)ceil(log10(gain_tmp)/log10(2));
		gain_tmp=GAIN_ADJ-pow(2,finalshift_adj);
		finegain=(int)(2.0*gain_tmp*1024.0/gain);
	}


//  printf("Fine gain is %d\n", finegain);
  finalshift=finalshift+finalshift_adj;
//  printf("Final shift is %d\n", finalshift);
//  printf("Final shift adjust is %d\n", finalshift_adj);
  if(finalshift>15) finalshift=15;					// limit on final shift (page 14 in GC4016 manual
  if(finalshift<0) finalshift=0;					// limit on final shift (page 14 in GC4016 manual
  if (finegain>=16384) finegain=16383;					// limit on finegain (pg.9 in GC4016 manual
  //if (finegain<=1) finegain=1024;					// if negative or zero gain is needed, then signals will be wrong, so set fine gain to 1
  if (finegain<=1) finegain=1;					// if negative or zero gain is needed, then signals will be wrong, so set fine gain to 1


  /* Set the parameters to the values calculated */
  chan=chana;
  if (verbose > 1) printf("Setcard Chan A Address 0x%x\n",chan);
  chan->decratio=0x7000 | (decimation-1);
  chan->cicscale=cicscale;
  chan->cfir=0x00 | coarse<<4;
  chan->finegain=finegain;
  chan->input= 0x00;
  chan->chreset=0x08+shift & 0x0f;
  chan=chanb;
  if (verbose > 1) printf("Setcard Chan B Address 0x%x\n",chan);
  chan->decratio=0x7000 | (decimation-1);
  chan->cicscale=cicscale;
  chan->cfir=0x00 | coarse<<4;
  chan->finegain=finegain;
  chan->input= 0x01;
  chan->chreset=0x08+shift & 0x0f;


  if(RESAMP_BYPASS){
    global->nmult=0x46;
    global->filtslct=0x00;
    global->finalshft=0x15;
    global->ratio0=0x04000000;
    global->ratio1=0x04000000;
  } else {
    global->nmult=0x07;
    global->filtslct=0x50;
    global->finalshft=0x10 | finalshift;
    global->ratio0=(int)((((float)Fsamp/(4*(float)decimation))/
                          (float)freqout)*(float)0x04000000);
    global->ratio1=(int)((((float)Fsamp/(4*(float)decimation))/
                          (float)freqout)*(float)0x04000000);
  }



  if (verbose > 1) printf("SetCard A:: Rate: %lf matched: %d\n",chana->rate,chana->match);
  if (verbose > 1) printf("SetCard B:: Rate: %lf matched: %d\n",chanb->rate,chanb->match);
  if (verbose > 1) printf("SetCard C:: Rate: %lf matched: %d\n",chanc->rate,chanc->match);
  if (verbose > 1) printf("SetCard D:: Rate: %lf matched: %d\n",chand->rate,chand->match);

  /* CALCULATE PARAMETERS FOR CHANNELS C&D *********************************************/
  /* Channels C and D are set up for use with Todd Parris' meteor detection
     code.  Channel C is dedicated to input CH1, which is connected
     to the main array of the radar.  Channel D is dedicated to input
     CH2, which is connected to the secondary/interferometry array.
     These channels oversample the echoes with broader filters to allow
     edge detection of echoes from 'point' targets (such as meteor trails).
     Channels C and D are set just the same as channels A and B, except the
     output frequecny, and filter frequencies are multiplied by 'mosf' (meteor
     oversampling factor), and matched filtering is used.
  */
  matched=chanc->match;
  freqout=chanc->rate;
  if (verbose > 1) printf("SetCard FreqoutCD: %lf matched: %d\n",freqout,matched);

  if(RESAMP_BYPASS){
  	decimation=(int) ((float) Fsamp/(4*freqout));		// the factor of 10 allows the input data rate to the resampler, CFIR and PFIR to be fast enough to allow effective filter coeffs to be used
        ndelay=1;
  }
  else if(matched){
  	decimation=(int) ((float) Fsamp/(4*50000));
	ndelay=32;
        matched=1;
  }
  else{
  	decimation=(int) ((float) Fsamp/(6*freqout));		// the factor of 6 allows the input data rate to the resampler, CFIR and PFIR to be fast enough to allow effective filter coeffs to be used
        ndelay=32;
  }
  if (decimation>4096){
	 decimation=4096;
         //fprintf(stderr, "Decimation on channels C&D too great: \n");
  }
  if (decimation<8){
	 decimation=8;
         //fprintf(stderr, "Decimation on channels C&D too small: \n");
  }
  smpdlyF=0;
  Tdelay=0;
  if(matched) Tdelay+=(1/freqout);
  Tdelay+=((2.5*decimation)+(0.5*decimation*21)+(decimation*63)+(2*decimation*8)+40)/Fsamp;
  smpdlyF+=Tdelay*(float)freqout;
  smpdlyF+=2;
  *smpdlyCD=(int)(smpdlyF+0.5);


  if (decimation>3104){
    shift=0;
    bigscale=(int)((62-5*log10(decimation)/log10(2)-shift-scale)/6);
    if(bigscale>7) bigscale=7;
    if(bigscale<0) bigscale=0;
    cicscale=((bigscale<<3)+(scale)) & 0x3f;
  }
  else{
    shift=4;
    bigscale=(int)((62-5*log10(decimation)/log10(2)-shift-scale)/6);
    if(bigscale>7) bigscale=7;
    if(bigscale<0) bigscale=0;
    cicscale=((0x1<<6)+(bigscale<<3))+(scale) & 0x7f;
  }

  cicgain=pow((float) decimation,5)*pow(2,shift+scale+6*bigscale-62);
  coarse=(int)(log10(1.0/cicgain)/log10(2));  
  if(coarse>7) coarse=7;
  if(coarse<0) coarse=0;

  CFIRfreq=(float)Fsamp/(float)decimation;
  PFIRfreq=CFIRfreq/2;
  resampfreq=ndelay*PFIRfreq/2;



  if(matched){
  	CFIRgain=setCFIR(cfircoeffsCD,CFIRfreq,50000,50000);
  	PFIRgain=setPFIR(pfircoeffsCD,PFIRfreq,freqout,freqout,1);
  	resampgain=setresamp(resampcoeffs,resampfreq,25000,25000,1,RESAMP_BYPASS);
  }
  else{
  	CFIRgain=setCFIR(cfircoeffsCD,CFIRfreq,freqout,freqout);
  	PFIRgain=setPFIR(pfircoeffsCD,PFIRfreq,freqout,freqout,0);
  	resampgain=setresamp(resampcoeffs,resampfreq,freqout,freqout,1,RESAMP_BYPASS);
  }

  resampgain=(resampgain/ndelay)*pow(2,finalshift);

  gain=cicgain * pow(2,coarse) * CFIRgain * PFIRgain * resampgain;      
  finegain=(int) (2*GAIN_ADJ*1024/gain); 				
  if (finegain>=16384) finegain=16384;					
  if (finegain<=1) finegain=1024;					

  /* Set the parameters to the values calculated */

  chan=chanc;
  if (verbose > 1) printf("Setcard Chan C Address 0x%x\n",chan);
  chan->decratio=0x7000 | (decimation-1);
  chan->cicscale=cicscale;
  chan->cfir=0x00 | coarse<<4;
  chan->finegain=finegain;
  chan->input= 0x00;
  chan->chreset=0x08+shift & 0x0f;
  chan=chand;
  if (verbose > 1) printf("Setcard Chan D Address 0x%x\n",chan);
  chan->decratio=0x7000 | (decimation-1);
  chan->cicscale=cicscale;
  chan->cfir=0x00 | coarse<<4;
  chan->finegain=finegain;
  chan->input= 0x01;
  chan->chreset=0x08+shift & 0x0f;


  /* This routine sets the sampler appropriately if the reampler is to be
     bypassed (see page 15 in the GC4016 manual).  IF THE RESAMPLER IS 
     BYPASSED, THEN THE OUTPUT DATA RATE IS EQUAL TO FCLOCK/(4*DECIMATION).
     IF IT IS NOT BYPASSES, THEN THE OUTPUT DATA RATE IS EQUAL TO 'freqout'.
  */
  /* This routine sets the sampler appropriately if the reampler is to be
     bypassed (see page 15 in the GC4016 manual).  IF THE RESAMPLER IS 
     BYPASSED, THEN THE OUTPUT DATA RATE IS EQUAL TO FCLOCK/(4*DECIMATION).
     IF IT IS NOT BYPASSES, THEN THE OUTPUT DATA RATE IS EQUAL TO 'freqout'.
  */

  if(RESAMP_BYPASS){
    global->ratio2=0x04000000;
    global->ratio3=0x04000000;
  } else {
    global->ratio2=(int)((((float)Fsamp/(4*(float)decimation))/
                        (float)(freqout))*(float)0x04000000);
    global->ratio3=(int)((((float)Fsamp/(4*(float)decimation))/
                        (float)(freqout))*(float)0x04000000);
  }

}

