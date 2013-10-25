/* Program ics660_xmt */
#include <stdio.h>
#include <stdint.h>
#include "ics660b.h"

void assign_register_addresses(uint32_t mem0, struct ics660_registers *ics660_regs, uint32_t m2base, struct dc60m_registers *dc60m_regs){
  int i;

  ics660_regs->board_reset = mem0 + BOARD_RESET;
  ics660_regs->dac_reset = mem0 + DAC_RESET;
  ics660_regs->status = mem0 + STATUS;
  ics660_regs->interrupt_mask = mem0 + INTERRUPT_MASK;
  ics660_regs->control = mem0 + CONTROL;
  ics660_regs->trans_length = mem0 + TRANS_LENGTH;
  ics660_regs->bank_length = mem0 + BANK_LENGTH;
  ics660_regs->fpdp_frame_length = mem0 + FPDP_FRAME_LENGTH;
  ics660_regs->persistence = mem0 + PERSISTENCE;
  ics660_regs->bank_width = mem0 + BANK_WIDTH;

  dc60m_regs->sync_mode = m2base + SYNC_MODE;
  dc60m_regs->interpolation_mode = m2base + INTERPOLATION_MODE;
  dc60m_regs->interpolation_gain = m2base + INTERPOLATION_GAIN;
  dc60m_regs->interpolation_byte0 = m2base + INTERPOLATION_BYTE0;
  dc60m_regs->interpolation_byte1 = m2base + INTERPOLATION_BYTE1;
  dc60m_regs->reset = m2base + RESET;
  dc60m_regs->counter_byte0 = m2base + COUNTER_BYTE0;
  dc60m_regs->counter_byte1 = m2base + COUNTER_BYTE1;
  dc60m_regs->channel_a_sync = m2base + CHANNEL_A_SYNC;
  dc60m_regs->channel_b_sync = m2base + CHANNEL_B_SYNC;
  dc60m_regs->channel_c_sync = m2base + CHANNEL_C_SYNC;
  dc60m_regs->channel_d_sync = m2base + CHANNEL_D_SYNC;
  dc60m_regs->channel_flush_mode = m2base + CHANNEL_FLUSH_MODE;
  dc60m_regs->miscelaneous = m2base + MISCELLANEOUS;
  dc60m_regs->stat = m2base + STATUS_REG;
  dc60m_regs->page = m2base + PAGE;
  for(i=0; i<16; i++) dc60m_regs->input_registers[i] = m2base + INPUT_REGISTER_0 + i*4;
}
