/* function to generate a one-shot pulse from chip 1 */
#include <stdio.h>
#include <math.h>
#include  <hw/pci.h>
#include <stdint.h>
#include <time.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include "ics660b.h"

void channel_flush(struct dc60m_registers dc60m_regs){
  struct timespec sl_time,hold_time;

  sl_time.tv_sec = (time_t)0;
  sl_time.tv_nsec = (long)500000;

  *(uint32_t *)dc60m_regs.channel_flush_mode = (uint32_t)0xff;
  fprintf(stderr,"CHANNEL_FLUSH  flushed %x\n",*(uint32_t *)dc60m_regs.channel_flush_mode);
  nanosleep(&sl_time,&hold_time);
  *(uint32_t *)dc60m_regs.channel_flush_mode = (uint32_t)0x55;
  fprintf(stderr,"CHANNEL_FLUSH  restored %x\n",*(uint32_t *)dc60m_regs.channel_flush_mode);
}
