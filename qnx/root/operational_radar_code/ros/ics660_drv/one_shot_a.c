/* function to generate a one-shot pulse from chip 1 */
#include <stdio.h>
#include <math.h>
#include  <hw/pci.h>
#include <stdint.h>
#include <time.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include "ics660b.h"

void one_shot_a(uint32_t mod_control_reg, struct dc60m_registers dc60m_regs){
  struct timespec sl_time,hold_time;

  sl_time.tv_sec = (time_t)0;
  sl_time.tv_nsec = (long)500000;

  select_chip(mod_control_reg,(uint32_t)0x01);
  
  *(uint32_t *)dc60m_regs.sync_mode = (uint32_t)0xe5;
  printf("ONE_SHOT_A sync_mode: %x   %x\n",*(uint32_t *)dc60m_regs.sync_mode,dc60m_regs.sync_mode);
  nanosleep(&sl_time,&hold_time);
  *(uint32_t *)dc60m_regs.sync_mode = (uint32_t)0x65;
}
