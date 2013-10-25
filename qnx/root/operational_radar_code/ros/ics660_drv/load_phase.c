/* Function to load the phase of a channel 

   arguments are the 

   dc60m_regs - the register locations
   channel - the channel number (1-4)
   fr - the input frequency in Hz
   ph - the input phase in radians
*/
#include <stdio.h>
#include <math.h>
#include  <hw/pci.h>
#include <stdint.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include "ics660b.h"

void load_phase(struct dc60m_registers dc60m_regs, int channel, double ph)
{

  double p_scale,pi;
  double frac,intg;
  uint32_t phase;
  int channel_register[]={0,8,0,8};
  int channel_pages[]={0,0,1,1};
  int i;
  
  p_scale = pow(2.,16.);
  pi = acos(-1.);

  frac=modf(ph/(2.*pi),&intg);
  ph=frac*2.*pi;
  if( ph<0 ){ ph += 2.*pi;}

  phase = (uint32_t)(p_scale*ph/(2.*pi));

  //printf("ph: %lf  phase %d\n",ph,phase);
  *(uint32_t *)dc60m_regs.page = (uint32_t)channel_pages[channel-1];

  for( i=4; i<6; i++)
    *(uint32_t *)dc60m_regs.input_registers[channel_register[channel-1]+i] = 
      ((uint32_t)((phase >> (i-4)*8) & 0x00ff));
  
}
