/* function to set a control parameter on the main board */
#include <stdio.h>
#include <math.h>
#include  <hw/pci.h>
#include <stdint.h>
#include <time.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include "ics660b.h"

void set_parameter(struct ics660b *ics660, uint32_t parameter, uint32_t value ){
  uint32_t temp;

  switch(parameter){

  case TRIGGER_SOURCE:
    {
      fflush(stdout);
      temp = *(uint32_t *)ics660->regs.control & ~TRIGGER_SOURCE;
      *(uint32_t *)ics660->regs.control = temp | value;
    }

  case CLOCK_SELECT:
    {
      temp = *(uint32_t *)ics660->regs.control & ~CLOCK_SELECT;
      value = (value << 1) & CLOCK_SELECT;
      *(uint32_t *)ics660->regs.control = temp | value;
    }

  case MODE:
    {
      temp = *(uint32_t *)ics660->regs.control & ~MODE;
      value = value & MODE;
      *(uint32_t *)ics660->regs.control = temp | value;
    }

  case UPCONVERTER_SELECT:
    {
      temp = *(uint32_t *)ics660->regs.control & ~UPCONVERTER_SELECT;
      value = (value << 4) & UPCONVERTER_SELECT;
      *(uint32_t *)ics660->regs.control = temp | value;
    }

  case DATA_SOURCE:
    {
      temp = *(uint32_t *)ics660->regs.control & ~DATA_SOURCE;
      value = (value << 5) & DATA_SOURCE;
      *(uint32_t *)ics660->regs.control = temp | value;
    }

  case FPDP_CLOCK_SEL:
    {
      temp = *(uint32_t *)ics660->regs.control & ~FPDP_CLOCK_SEL;
      value = (value << 6) & FPDP_CLOCK_SEL;
      *(uint32_t *)ics660->regs.control = temp | value;
    }

  case FPDP_TERM:
    {
      temp = *(uint32_t *)ics660->regs.control & ~FPDP_TERM;
      value = (value << 7) & FPDP_TERM;
      *(uint32_t *)ics660->regs.control = temp | value;
    }

  case SYNC_MASTER:
    {
      temp = *(uint32_t *)ics660->regs.control & ~SYNC_MASTER;
      value = (value << 8) & SYNC_MASTER;
      *(uint32_t *)ics660->regs.control = temp | value;
    }

  case SYNC_TERM:
    {
      temp = *(uint32_t *)ics660->regs.control & ~SYNC_TERM;
      value = (value << 9) & SYNC_TERM;
      *(uint32_t *)ics660->regs.control = temp | value;
    }

  case DAC_ENABLE:
    {
      temp = *(uint32_t *)ics660->regs.control & ~DAC_ENABLE;
      value = (value << 10) & DAC_ENABLE;
      *(uint32_t *)ics660->regs.control = temp | value;
    }

  case TRIGGER:
    {
      temp = *(uint32_t *)ics660->regs.control & ~TRIGGER;
      value = (value << 11) & TRIGGER;
      *(uint32_t *)ics660->regs.control = temp | value;
    }

  case BANK_WIDTH:
    {
      *(uint32_t *)ics660->regs.bank_width = value;
      break;
    }

  case BANK_LENGTH:
    {
      *(uint32_t *)ics660->regs.bank_length = value;
      break;
    }

  case PERSISTENCE_REG:
    {
      *(uint32_t *)ics660->regs.persistence = value;
      break;
    }
    
  case TRANS_LENGTH:
    {
      *(uint32_t *)ics660->regs.trans_length = value;
      break;
    }

  default:
    fprintf(stderr,"**** SET_PARAMETER invalid parameter %x ****\n",parameter);
    break;
  }
}
