/* Program select_chip */
#include <stdio.h>
#include <math.h>
#include  <hw/pci.h>
#include "ics660b.h"
#include <stdint.h>
#include <sys/mman.h>
#include <hw/inout.h>

int select_chip(uint32_t mod_control_reg,uint32_t chip)
{
  uint32_t control_value = 0;

  control_value = *(uint32_t *)mod_control_reg;
  control_value = (control_value  & 0xfffffff8) | chip;
  *(uint32_t *)mod_control_reg = (uint32_t)control_value;
}
