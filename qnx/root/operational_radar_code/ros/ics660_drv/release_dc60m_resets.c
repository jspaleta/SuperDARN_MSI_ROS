/* function to generate a one-shot pulse from chip 1 */
#include <stdio.h>
#include <math.h>
#include  <hw/pci.h>
#include <stdint.h>
#include <time.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include "ics660b.h"

void release_dc60m_resets(uint32_t mod_control_reg, struct dc60m_registers dc60m_regs)
{
  uint32_t chip;
  uint32_t control_value=0;

  struct timespec sl_time,hold_time;

  sl_time.tv_sec = (time_t)0;
  sl_time.tv_nsec = (long)500000;


  /* assert s1a bar on chip 1*/

  select_chip(mod_control_reg, (uint32_t)0x01);

  *(uint32_t *)dc60m_regs.miscelaneous = (uint32_t)0x80;

  *(uint32_t *)dc60m_regs.sync_mode = (uint32_t) 0xe5;

  for( chip=4; chip>=1; chip--){
    select_chip(mod_control_reg,chip);
    /* release reset */
    *(uint32_t *)dc60m_regs.reset = (uint32_t)0x10; 
  } 


  /* release s1a bar on chip 2*/
  
  select_chip(mod_control_reg, (uint32_t)0x02);
  *(uint32_t *)dc60m_regs.sync_mode = (uint32_t) 0x65;

  /* release s1a bar on chip 1*/
  
  select_chip(mod_control_reg, (uint32_t)0x01);
  *(uint32_t *)dc60m_regs.sync_mode = (uint32_t) 0x65;


  /* set oneshot mode to pulse */
  /* set the miscellaneous register  to have OS_MODE=0*/
  for( chip=4; chip>=1; chip--){
    select_chip(mod_control_reg,chip);
    *(uint32_t *)dc60m_regs.miscelaneous = (uint32_t)0x00000000;
  }
    select_chip(mod_control_reg,(uint32_t)0x5);
}
