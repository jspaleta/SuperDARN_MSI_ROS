/* function to initialize a DC-60-M daughter card */

#include <stdio.h>
#include <math.h>
#include  <hw/pci.h>
#include "ics660b.h"
#include <stdint.h>
#include <sys/mman.h>
#include <hw/inout.h>

#define round(x) ((double)(x)-floor((double)(x))<(double)0.5?floor(x):ceil(x))

int dc60m_init(uint32_t mod_control_reg, int interp_val, struct dc60m_registers dc60m_regs)
{
  uint32_t control_value = 0;
  uint32_t sync_mode_value = 0;
  uint32_t interp0 = 0;
  uint32_t interp1 = 0;
  uint32_t interpolation_gain = 0;
  uint32_t cic_interp = 0;
  uint32_t reset_value = 0;
  uint32_t counter_mode1 = 0xffff;
  uint32_t counter_mode2 = 0xffff;
  uint32_t channel_sync = 0;
  uint32_t channel_flush = 0;
  uint32_t miscellaneous = 0;
  uint32_t status_control = 0;
  uint32_t page = 0;
  uint32_t page_reg_value = 0;
  uint32_t chip = 0;
  int i,status;
  int BIG_SHIFT, SCALE;
  float cicgain;
  
  printf("in dc60m_init  %x  %d  %x\n", mod_control_reg, interp_val, dc60m_regs.sync_mode);  

  /* Set module control register */
  control_value = (uint32_t)0x08; //Selects PLL operating range
  if( interp_val < 11) control_value = control_value | (uint32_t)(((uint32_t)(interp_val % 7) & 0x03) << 5); //Sets special case of CIC interpolation when interp =8,9 or 10
  
  *(uint32_t *)mod_control_reg = (uint32_t)control_value;
  
  /* initilize 4116 chips sync modes and interpolation modes */

  set_dc60m_resets(mod_control_reg,dc60m_regs);

  for( chip=1; chip<=4; chip++){

    select_chip(mod_control_reg,chip);

    /* set page to 0 */
    *(uint32_t *)dc60m_regs.page = (uint32_t)0x0000;

    if( (status=init_gc4116_sync(dc60m_regs)) != GOOD ){
      perror("FAILED GC4116 INITIALIZATION\n");
    }
    
    /* calculate and set BIG_SHIFT and SCALE */
    BIG_SHIFT = (uint32_t)2;
    if( interp_val < 1449 ) BIG_SHIFT = (uint32_t)2; 
    if( interp_val < 182 ) BIG_SHIFT = (uint32_t)1; 
    if( interp_val < 23 ) BIG_SHIFT = (uint32_t)0; 
    
    SCALE = (uint32_t) floor(4.*log((double)interp_val)/log((double) 2.)) - 3. - 12.*BIG_SHIFT + 1;
    if( SCALE > 15 )SCALE=(uint32_t)15;

    cicgain = pow((double)interp_val,(double)4.)*pow((double)2.,(double)-(SCALE+12*BIG_SHIFT+3));
    printf("dc60m  interp: %d  BIG_SHIFT:  %x  SCALE: %x  cicgain:  %f\n",interp_val,BIG_SHIFT,SCALE,cicgain);
    
    BIG_SHIFT = (BIG_SHIFT << 4) & 0x00000030;
    printf("dc60m  interp: %d  BIG_SHIFT | SCALE:  %x\n",interp_val,BIG_SHIFT | SCALE);
    *(uint32_t *)dc60m_regs.interpolation_gain = (uint32_t)(BIG_SHIFT | SCALE);

    /* set the CIC interpolation */
    interp0 = 0x000000ff & (uint32_t)interp_val-1;
    *(uint32_t *)dc60m_regs.interpolation_byte0 = (uint32_t)interp0;
    
    interp1 = ((interp_val-1) >> 8) & 0x0000003f;
    *(uint32_t *)dc60m_regs.interpolation_byte1 = (uint32_t)interp1;
    
    /* set counter mode to ffff */
    *(uint32_t *)dc60m_regs.counter_byte0 = (uint32_t)0xffffffff; 
    *(uint32_t *)dc60m_regs.counter_byte1 = (uint32_t)0xffffffff; 
            
    /* initialize the status register to 00 */
    *(uint32_t *)dc60m_regs.stat = (uint32_t)0x00;
    
    /* set the paged registers */
    
    /* initialize all paged registers to 0 */
    for( page=0; page<=30; page++ ){
     *(uint32_t *)dc60m_regs.page = (uint32_t)page;
     for( i=0; i<16; i++ )*(uint32_t *)dc60m_regs.input_registers[i] = (uint32_t)0x00000000;
    }
    
    /* initialize gain settings for all channels */
    *(uint32_t *)dc60m_regs.page = (uint32_t)0x02; // Page 2
    for( i=0; i<4; i++ )*(uint32_t *)dc60m_regs.input_registers[i] = (uint32_t)((float)0x19*cicgain);
    
    /* initialize the I/O control page */
    *(uint32_t *)dc60m_regs.page = (uint32_t)0x0005;
    /* set the channel input mode register to serial packed */
    *(uint32_t *)dc60m_regs.input_registers[0] = (uint32_t)0x05;
    /* set the sum I/O mode register */
    *(uint32_t *)dc60m_regs.input_registers[3] = (uint32_t)0xb0;
    /* enable output */
    *(uint32_t *)dc60m_regs.input_registers[8] = (uint32_t)0x55; // I/O control page 

    /* set page back to 0 */
    *(uint32_t *)dc60m_regs.page = (uint32_t)0x0000;
    
  }

  select_chip(mod_control_reg,(uint32_t)0x00);

  return(1);
}
