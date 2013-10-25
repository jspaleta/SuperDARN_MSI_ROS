/* Function to load the PFIR filter taps 

   arguments are the 

     channel - 1,2,3,or 4
     fc - the corner frequency of the filter (in base band)
     t_in - the state time increment (i.e. data input rate)

*/
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include "ics660b.h"

void load_filter_taps(struct dc60m_registers dc60m_regs, int channel, int fc, double t_in)
{
  int sum_coef,i,j,findex;
  int ntaps=NTAPS;
  int filter_coefs[NTAPS+1]={0},fi;
  uint32_t channel_pages[]={16,20,24,28};
  union coefs {
    unsigned short f_coef;
    struct {
      unsigned char low;
      unsigned char high;
    } bytes;
  } coef_value;

  fi = (int) (1./t_in);

  //fprintf(stderr,"ICS660 - DRIVER - LOAD_FILTER_TAPS - f_c %d  f_i %d  t_in %f\n", fc,fi,t_in);
  sum_coef = ideal_filter_coef(ntaps,fi,fc,filter_coefs);

  for( i=0; i<4; i++ ){
    *(uint32_t *)dc60m_regs.page = (uint32_t)(channel_pages[channel-1]+i);
    for(j=0; j<8; j++){
      findex = ntaps - j - i*8; // reverse order of taps placing end tap in first slot
      coef_value.f_coef = (unsigned short)(filter_coefs[findex]);

      findex = j + i*8;

      *(uint32_t *)dc60m_regs.input_registers[2*j] = (uint32_t)coef_value.bytes.low;
      *(uint32_t *)dc60m_regs.input_registers[2*j+1] = (uint32_t)coef_value.bytes.high;


    }
  }
}
