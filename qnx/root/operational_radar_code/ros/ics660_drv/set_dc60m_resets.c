/* function to generate a one-shot pulse from chip 1 */
#include <stdio.h>
#include <math.h>
#include  <hw/pci.h>
#include <stdint.h>
#include <time.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include "ics660b.h"

void set_dc60m_resets(uint32_t mod_control_reg, struct dc60m_registers dc60m_regs)
{
  uint32_t chip;
  uint32_t control_value=0;

  for( chip=1; chip<=4; chip++){

    select_chip(mod_control_reg,chip);

    /* reset chip */
    *(uint32_t *)dc60m_regs.reset = 0xff;

    /* set the miscellaneous register  to have OS_MODE=1*/
    *(uint32_t *)dc60m_regs.miscelaneous = (uint32_t)0x00000080;

    if(chip <= 2 ){
      *(uint32_t *)dc60m_regs.sync_mode = (uint32_t) 0xe5;
    }else{
      *(uint32_t *)dc60m_regs.sync_mode = (uint32_t) 0x05;
    }

  }
  select_chip(mod_control_reg,(uint32_t)0x00);
}
