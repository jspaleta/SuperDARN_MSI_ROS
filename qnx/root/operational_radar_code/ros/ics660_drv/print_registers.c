#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include "ics660b.h"

void print_registers(struct dc60m_registers dc60m_regs){

  int i;

  printf(" sync_mode %x  %x\n",dc60m_regs.sync_mode, *(uint32_t *)(dc60m_regs.sync_mode));
  printf(" interpolation_mode %x  %x\n",dc60m_regs.interpolation_mode, *(uint32_t *)(dc60m_regs.interpolation_mode));
  printf(" interpolation_gain %x  %x\n",dc60m_regs.interpolation_gain, *(uint32_t *)(dc60m_regs.interpolation_gain));
  printf(" interpolation_byte0 %x  %x\n",dc60m_regs.interpolation_byte0, *(uint32_t *)dc60m_regs.interpolation_byte0);
  printf(" interpolation_byte1 %x  %x\n",dc60m_regs.interpolation_byte1, *(uint32_t *)dc60m_regs.interpolation_byte1);
  printf(" reset %x  %x\n",dc60m_regs.reset, *(uint32_t *)dc60m_regs.reset);
  printf(" counter_byte0 %x  %x\n",dc60m_regs.counter_byte0, *(uint32_t *)dc60m_regs.counter_byte0);
  printf(" counter_byte1 %x  %x\n",dc60m_regs.counter_byte1, *(uint32_t *)dc60m_regs.counter_byte1);
  printf(" channel_a_sync %x  %x\n",dc60m_regs.channel_a_sync, *(uint32_t *)dc60m_regs.channel_a_sync);
  printf(" channel_b_sync %x  %x\n",dc60m_regs.channel_b_sync, *(uint32_t *)dc60m_regs.channel_b_sync);
  printf(" channel_c_sync %x  %x\n",dc60m_regs.channel_c_sync, *(uint32_t *)dc60m_regs.channel_c_sync);
  printf(" channel_d_sync %x  %x\n",dc60m_regs.channel_d_sync, *(uint32_t *)dc60m_regs.channel_d_sync);
  printf(" channel_flush_mode %x  %x\n",dc60m_regs.channel_flush_mode, *(uint32_t *)dc60m_regs.channel_flush_mode);
  printf(" miscelaneous %x  %x\n",dc60m_regs.miscelaneous, *(uint32_t *)dc60m_regs.miscelaneous);
  printf(" status %x  %x\n",dc60m_regs.stat, *(uint32_t *)dc60m_regs.stat);
  printf(" page %x  %x\n",dc60m_regs.page, *(uint32_t *)dc60m_regs.page);

  *(uint32_t *)dc60m_regs.page = (uint32_t)0;
  for( i=0; i<16; i++ ) printf("page 0  %x register %d = %x\n",dc60m_regs.input_registers[i]-dc60m_regs.sync_mode,i,*(uint32_t *)dc60m_regs.input_registers[i]);
  *(uint32_t *)dc60m_regs.page = (uint32_t)1;
  for( i=0; i<16; i++ ) printf("page 1 register %d = %x\n",i,*(uint32_t *)dc60m_regs.input_registers[i]);
  *(uint32_t *)dc60m_regs.page = (uint32_t)2;
  for( i=0; i<16; i++ ) printf("page 2 register %d = %x\n",i,*(uint32_t *)dc60m_regs.input_registers[i]);
  *(uint32_t *)dc60m_regs.page = (uint32_t)5;
  for( i=0; i<16; i++ ) printf("page 5 register %d = %x\n",i,*(uint32_t *)dc60m_regs.input_registers[i]);
  *(uint32_t *)dc60m_regs.page = (uint32_t)16;
  for( i=0; i<16; i++ ) printf("page 16 register %d = %x  address = %x\n",i,*(uint32_t *)dc60m_regs.input_registers[i],dc60m_regs.input_registers[i]);
  *(uint32_t *)dc60m_regs.page = (uint32_t)17;
  for( i=0; i<16; i++ ) printf("page 17 register %d = %x  address = %x\n",i,*(uint32_t *)dc60m_regs.input_registers[i],dc60m_regs.input_registers[i]);
  *(uint32_t *)dc60m_regs.page = (uint32_t)18;
  for( i=0; i<16; i++ ) printf("page 18 register %d = %x  address = %x\n",i,*(uint32_t *)dc60m_regs.input_registers[i],dc60m_regs.input_registers[i]);
  *(uint32_t *)dc60m_regs.page = (uint32_t)19;
  for( i=0; i<16; i++ ) printf("page 19 register %d = %x  address = %x\n",i,*(uint32_t *)dc60m_regs.input_registers[i],dc60m_regs.input_registers[i]);

}
