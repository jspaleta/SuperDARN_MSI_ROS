/* Program ics660_init */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include  <hw/pci.h>
#include <stdint.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include "ics660b.h"


struct ics660b *ics660_drv_init(int pci_ind)
{
  int page;
  long nchan=16,i;
  long status;
  long interp_val,channel;
  double state_time = STATE_TIME;
  double pi;
  struct pci_base ics660pci;
  uint32_t sequencer_base = 0x00100000;
  uint32_t *mod_control_reg, *m2base;
  uint32_t chip;
  long wr_addr,bl_adv,seq_word;
  int pci_max=3;
  struct ics660b *ics660_struct;
  volatile unsigned char *ics660_mem0;
  volatile unsigned char *ics660_mem1;
  struct ics660_registers ics660_regs;
  struct dc60m_registers dc60m_regs;
  struct timespec sl_time,hold_time;

  ics660_struct = (struct ics660b *)calloc((size_t) 1,(size_t) sizeof(struct ics660b));
  
  /* Initialize card to get base address */ 
  if( get_pci_base_addr(DEVICE_ID,VENDOR_ID,pci_ind,&ics660pci) == -1 )
    {
      perror("***** FAILED TO GET BASE ADDRESSES *****\n");
      fprintf(stderr,"error  %d  %d  %d\n",DEVICE_ID,VENDOR_ID,pci_ind);
      exit(0);
    }
  /* Memory map address space */
  
  if( (ics660_mem0 = mmap_device_memory( 0, (size_t) ICS660_MEM_SIZE,
						  PROT_EXEC|PROT_READ|PROT_WRITE|PROT_NOCACHE,
						  0,(uint32_t) ics660pci.base0))
      == MAP_FAILED)
    {
      perror("***** FAILED TO MEMORY MAP CARD MEMORY *****\n");
      exit (0);
    }
  
  if( (ics660_mem1 =  mmap_device_memory( 0, (size_t) ICS660_MEM_SIZE,
						   PROT_EXEC|PROT_READ|PROT_WRITE|PROT_NOCACHE,
						   0,(uint32_t) ics660pci.base1))
      == MAP_FAILED)
    {
      perror("***** FAILED TO MEMORY MAP CARD MEMORY *****\n");
      exit (0);
    }
  
  m2base = (uint32_t *)(ics660_mem0 + (uint32_t)M2BASE_OFFSET);
  mod_control_reg = (uint32_t *)(ics660_mem0 + (uint32_t)M2BASE2_OFFSET);
  assign_register_addresses(ics660_mem0,&ics660_regs,m2base,&dc60m_regs);

  ics660_struct->mem0 = ics660_mem0;
  ics660_struct->mem1 = ics660_mem1;
  ics660_struct->pci_io = ics660pci.io_base;
  ics660_struct->regs = ics660_regs;
  ics660_struct->dc60m_regs = dc60m_regs;
  ics660_struct->mod_control_reg = mod_control_reg;

  /* Write to board reset register */
  *(unsigned int*)(ics660_regs.board_reset) = RESET_VALUE;  
  *(unsigned int*)(ics660_regs.interrupt_mask) = (uint32_t)IMASK_VALUE;

  /*** Select clock source (internal/external) (for some reason this is necessary) */
  set_parameter(ics660_struct,CLOCK_SELECT,EXTERNAL);

  return ics660_struct;
}

