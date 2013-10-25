/* function ideal_filter_coef */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#define round(x) ((double)(x)-floor((double)(x))<(double)0.5?floor(x):ceil(x))

//#define SCALE 65536.
#define SCALE 32768.
#define PI acos((double)(-1.))

/* calculates the coefficients for an ideal band-pass filter centered on the carrier
   frequency, with a corner frequency f_c.

   ntaps - number of filter taps (63 for the GC4116 - symmetric 2*31 + 1
   f_in - the inuput data rate in hertz
   f_c - the corner frequency in hertz. 
*/
int ideal_filter_coef(int ntaps, int f_in, int f_c, int *coef)
{
  int i,isum_coef; 
  float bw, tau, scale, pi;
  float fcoef[63]={0},sum_coef;

  pi = PI;
  bw = (float)f_c/(float)f_in;

  //fprintf(stderr,"ICS660 - DRIVER - FILTER  bw: %f  f_c: %d  f_in: %d\n",bw,f_c,f_in);
  fcoef[0] = .50;
  sum_coef = fcoef[0];

  for( i=1; i<ntaps+1; i++){
    fcoef[i] =0.5*exp(-(double)((i)*(i))*bw);
    sum_coef += fcoef[i];
  }


  scale = (SCALE/(float)sum_coef);
  isum_coef = 0;
  for( i=0; i<ntaps; i++){
    *(coef+i) = (int)(scale*fcoef[i]);
    isum_coef += *(coef+i);
  }
  return(isum_coef);
}
