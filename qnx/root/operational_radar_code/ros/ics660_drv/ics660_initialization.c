  /* Set control register */
  
  /*** Set DAC_ENABLE bit to disable */
  set_parameter(ics660_struct,DAC_ENABLE,DISABLE);

  /*** Select trigger source (internal/external) */
  set_parameter(ics660_struct,TRIGGER_SOURCE,EXTERNAL);

  /*** Select clock source (internal/external) */
  set_parameter(ics660_struct,CLOCK_SELECT,EXTERNAL);

  /*** Select operating mode (continuous/one-shot/loop/pulse) */
  set_parameter(ics660_struct,MODE,PULSE);

  /*** Select upconverter */
  set_parameter(ics660_struct,UPCONVERTER_SELECT,ENABLE);
  
  /*** Select input data path (FPDP ot PCI) */
  set_parameter(ics660_struct,DATA_SOURCE,PCI);
  
  /*** Set sync master/term bits as required */
  if(pci_ind == 0){ 
    set_parameter(ics660_struct,SYNC_MASTER,ENABLE);
  } else {
    set_parameter(ics660_struct,SYNC_MASTER,DISABLE);
  }
  
  /*** Set sync term on last board in chain ***/
  if(pci_ind==pci_max){
    set_parameter(ics660_struct,SYNC_TERM,ENABLE);
  } else {
    set_parameter(ics660_struct,SYNC_TERM,DISABLE);
  }
    
  /* Set PERSISTENCE register */
  *(uint32_t *)(ics660_regs.persistence) = 0x00000000;
  set_parameter(ics660_struct,PERSISTENCE_REG,(uint32_t)0);

  /* Set TRANSMIT_LENGTH register */
  set_parameter(ics660_struct,TRANS_LENGTH,signal_length);  

  /* Set BANK_LENGTH register */
  set_parameter(ics660_struct,BANK_LENGTH,signal_length);

  /* Set BANK_WIDTH register */
  set_parameter(ics660_struct,BANK_WIDTH,(uint32_t)0);


  /* Program write sequence generator */
 
  for( i=0; i<32; i++ ){
    wr_addr = (31 - i) % 16;
    bl_adv = ( i == 15 | i == 31 ) ? 1 : 0;
    seq_word = (bl_adv << 6 | wr_addr << 2) & 0x0000007c; 
    *(uint32_t *)(ics660_mem0 + sequencer_base + (uint32_t)(4*i)) 
      = (uint32_t)seq_word;
  }
 
  /* Configure upconverter */
  
  interp_val = (uint32_t)((state_time*CLOCK_FREQ)/4+0.499999);
//interp_val = (uint32_t)(50);
  status = dc60m_init(mod_control_reg,interp_val,dc60m_regs);
  
