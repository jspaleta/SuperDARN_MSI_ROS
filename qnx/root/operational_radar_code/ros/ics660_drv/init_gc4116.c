/* function to initialize a GC4116 chip */

#include <stdio.h>
#include <math.h>
#include  <hw/pci.h>
#include "ics660b.h"
#include <stdint.h>
#include <sys/mman.h>
#include <hw/inout.h>

int init_gc4116_sync(struct dc60m_registers dc60m_regs){
  

  *(uint32_t *)dc60m_regs.interpolation_mode = (uint32_t)0x000000c4; // sets gain sync
  *(uint32_t *)dc60m_regs.channel_a_sync = (uint32_t)0x00000055; // sets freq_sync, phase_sync, nco_sync, and dither_sync
  *(uint32_t *)dc60m_regs.channel_b_sync = (uint32_t)0x00000055; // sets freq_sync, phase_sync, nco_sync, and dither_sync
  *(uint32_t *)dc60m_regs.channel_c_sync = (uint32_t)0x00000055; // sets freq_sync, phase_sync, nco_sync, and dither_sync
  *(uint32_t *)dc60m_regs.channel_d_sync = (uint32_t)0x00000055; // sets freq_sync, phase_sync, nco_sync, and dither_sync
  *(uint32_t *)dc60m_regs.channel_flush_mode = (uint32_t)0x00000055; //sets channels to flush on sync
  return(GOOD);
}

