/* Program ics660_xmt */
#include <stdio.h>
#include <math.h>
#include  <hw/pci.h>
#include <stdint.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include "ics660b.h"

void set_dc_ready(uint32_t mod_control_reg){
  volatile uint32_t reg_value;

  reg_value = *(uint32_t *)mod_control_reg;
  *(uint32_t *)mod_control_reg = (uint32_t)reg_value | (uint32_t)0x10;
  pdebug("SET DC READY %x  %x\n",mod_control_reg,*(uint32_t *)mod_control_reg);
}
