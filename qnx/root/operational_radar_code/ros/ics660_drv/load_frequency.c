/* Function to load the frequency and phase of a channel 

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

void load_frequency(struct dc60m_registers dc60m_regs, int channel, double fr)
{
  double f_scale;
  uint32_t freq;
  int channel_register[]={0,8,0,8};
  int channel_pages[]={0,0,1,1};
  int i;
  
  f_scale = pow(2.,32.);

  freq = (uint32_t)(f_scale*fr/CLOCK_FREQ);

  *(uint32_t *)dc60m_regs.page = (uint32_t)channel_pages[channel-1];

  for( i=0; i<4; i++){
    *(uint32_t *)dc60m_regs.input_registers[channel_register[channel-1]+i] = 
      (uint32_t)(freq >> i*8);
  }
}
