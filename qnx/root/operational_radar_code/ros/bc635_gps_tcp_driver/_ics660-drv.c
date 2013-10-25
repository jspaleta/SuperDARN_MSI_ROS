#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <devctl.h>
#include <sys/iofunc.h>
#include <sys/iomsg.h>
#include <sys/dispatch.h>
#include "/root/beam_forming/ics660b.h"

int io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
int io_write (resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);
int io_devctl (resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);

volatile struct ics660b *ics660;

static resmgr_connect_funcs_t connect_funcs;
static resmgr_io_funcs_t io_funcs;
static iofunc_attr_t attr;

main( int argc, char **argv)
{
  resmgr_attr_t resmgr_attr;
  dispatch_t *dpp;
  dispatch_context_t *ctp;
  int id;
  int pci_id;
  char *device;

  /* check to see if device number is specified when invoked */
  if( argc == 1 ){
    fprintf(stderr, "%s: Invoke with device number (i.e. 1 = /dev/ics660-1)\n",argv[0]);
    return EXIT_FAILURE;
  }
  
  device = calloc((size_t) 64, sizeof(char));
  strcat(device,"/dev/ics660-");
  strcat(device,(const char *) argv[1]);
  printf("_ICS660_DRV: INITIALIZING DEVICE %s\n",device);
  
  sscanf(argv[1],"%d",&pci_id);
  printf("_ICS660_DRV: DEVICE ID %d\n",pci_id);

  ics660 = (struct ics660b *)ics660_drv_init((int) pci_id);

  /* initialize dispatch interface */
  if((dpp = dispatch_create()) == NULL){
    fprintf(stderr, "%s: Unable to allocate dispatch handle.\n", argv[0]);
    return EXIT_FAILURE;
  }
  /* initialize resource manager attributes */
  memset(&resmgr_attr, 0, sizeof resmgr_attr);
  resmgr_attr.nparts_max = 1;
  resmgr_attr.msg_max_size = 2048;

  /* initialize functions for handling messages */
  iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, 
		   _RESMGR_IO_NFUNCS, &io_funcs);
  io_funcs.read = io_read;
  io_funcs.write = io_write;
  io_funcs.devctl = io_devctl;

 /* initialize attribute structure used by the device */
  iofunc_attr_init(&attr, S_IFNAM | 0666, 0, 0);
  //attr.nbytes = strlen(buffer)+1;

  /* attach our device name */
  id = resmgr_attach(dpp,            // dispatch handle
		     &resmgr_attr,   // resource manager attrs
		     device,  // device name
		     _FTYPE_ANY,     // open type
		     0,              // flags
		     &connect_funcs, // connect routines
		     &io_funcs,      // I/O routines
		     &attr);         // handle
  if(id == -1){
    fprintf(stderr, "%s: Unable to attach name.\n",argv[0]);
    return EXIT_FAILURE;
  }
  /* allocate a context structure */
  ctp = dispatch_context_alloc(dpp);

  /* start the resource manager messager loop */
  while(1){
    if((ctp = dispatch_block(ctp)) == NULL) {
      fprintf(stderr,"block_error\n");
      return EXIT_FAILURE;
    }
    dispatch_handler(ctp);
  }
}


int
io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
  int nleft;
  int nbytes;
  int nparts;
  int status;
  uint32_t chip;
  char lbuff[64];
  uint32_t cval;

  if ((status = iofunc_read_verify(ctp, msg, ocb, NULL)) != EOK)
    return(status);

  if ((msg->i.xtype & _IO_XTYPE_NONE) != _IO_XTYPE_NONE)
    return (ENOSYS);

  
  /*
   * On all reads, calculate how many
   * bytes we can return to the client
   * based upon the number of bytes available (nleft)
   * and the client's buffer size
   */

  nleft = ocb->attr->nbytes - ocb->offset;
  nbytes = min(msg->i.nbytes, nleft);

  cval = *(uint32_t *)ics660->regs.control;
  fprintf(stderr,"io_read - cval %d\n",cval);
  sprintf(lbuff,"%x",cval);
  for( chip=1; chip<=4; chip++){
    select_chip(ics660->mod_control_reg,chip);
    printf("MAIN PRINT_REGISTER\n");
    print_registers(ics660->dc60m_regs);
  }      
  select_chip(ics660->mod_control_reg,(uint32_t)0x0);

  if (nbytes > 0){
    /* set up the return data IOV */
    //SETIOV( ctp->iov, buffer + ocb->offset, nbytes );
    SETIOV( ctp->iov, lbuff + ocb->offset, nbytes );

    /* set up the number of bytes (returned by client's read()) */
    _IO_SET_READ_NBYTES(ctp, nbytes);

    /*
     * advance the offset by the number of bytes 
     * returned to the client.
     */

    ocb->offset += nbytes;

    nparts = 1;
  } else {
    /*
     * they've asked for zero bytres or they've already 
     * read everything
     */
    _IO_SET_READ_NBYTES(ctp,0);

    nparts = 0;
  }

  /* mark the access time as invalid (we just accessed it) */
  if (msg->i.nbytes > 0)
    ocb->attr->flags |= IOFUNC_ATTR_ATIME;
 
  return (_RESMGR_NPARTS(nparts));
}

int
io_write( resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb)
{
  int status;
  char *buf;

  if((status = iofunc_write_verify(ctp, msg, ocb, NULL)) != EOK )
    return(status);

  if((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
    return(ENOSYS);

  _IO_SET_WRITE_NBYTES(ctp, msg->i.nbytes);

  buf = (char *)malloc(msg->i.nbytes + 1);
  if( buf == NULL)
    return(ENOMEM);

  /*
   * reread the data from the sender's message buffer.
   * We're not assuming that all of the data fit into the 
   * resource manager library's receive buffer.
   */
  if( (status = resmgr_msgread(ctp, buf, msg->i.nbytes, sizeof(msg->i))) == -1){ 
    return(ENOSYS);
  }
  fprintf(stderr,"_ics660-drvr:  bytes attemted: %d  bytes received %d\n",msg->i.nbytes,status);
  buf[msg->i.nbytes] = '\0'; //just in case text is not NULL terminated 
  memcpy((int *)ics660->mem1,buf,(size_t)msg->i.nbytes);
  free(buf);

  if(msg->i.nbytes > 0)
    ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

  return(_RESMGR_NPARTS(0));
}

int io_devctl (resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb){
  int nbytes, status, previous;
  uint32_t chip, page, reg;
  uint32_t value, temp, command, offset;
  uint32_t sequencer_base = 0x00100000;
  struct ICS660_FILTER filter_str;
  struct ICS660_FREQ freq_str;
  struct ICS660_PHASE phase_str;
  struct ICS660_sequencer seq_data;
  union {
    data_t data;
    uint32_t data32;
    int int_data;
    struct ICS660_FILTER filter_str;
    struct ICS660_FREQ freq_str;
    struct ICS660_PHASE phase_str;
    struct dc60m_p_val dc60m_p;
    struct dc60m_pg_reg pg_reg;
    struct ICS660_sequencer seq_data;
  } *rx_data;

  pdebug("IO_DEVCTL - entry -\n");

  if((status = iofunc_devctl_default(ctp, msg, ocb)) != _RESMGR_DEFAULT) 
    return(status);

  rx_data = _DEVCTL_DATA(msg->i);
  //pdebug("IO_DEVCTL - rx_data %d filter_str %d\n",rx_data,sizeof(rx_data->filter_str));
  //pdebug("IO_DEVCTL - msg %d  msg.i %d \n",msg,msg->i);

  command = get_device_command(msg->i.dcmd);
  pdebug("IO_DEVCTL - command %x  %x\n",msg->i.dcmd,command);

  switch(msg->i.dcmd){
    
  case ICS660_BOARD_RESET:
    pdebug("IO_DEVCTL - BOARD_RESET  %x\n",value);
    *(unsigned int*)(ics660->regs.board_reset) = RESET_VALUE;
    break;

  case ICS660_CLEAR_IMASK:
    pdebug("IO_DEVCTL - CLEAR_IMASK %x\n",value);
    *(unsigned int*)(ics660->regs.interrupt_mask) = (uint32_t)IMASK_VALUE;
    break;

  case ICS660_TRIGGER_SOURCE:
    value = rx_data->data32;
    pdebug("IO_DEVCTL - TRIGGER_SOURCE %x\n",value);
    temp = *(uint32_t *)ics660->regs.control & ~TRIGGER_SOURCE;
    *(uint32_t *)ics660->regs.control = temp | value;
    break;
    
  case ICS660_CLOCK_SELECT:
    value = rx_data->data32;
    temp = *(uint32_t *)ics660->regs.control & ~CLOCK_SELECT;
    value = (value << 1) & CLOCK_SELECT;
    pdebug("IO_DEVCTL - CLOCK_SELECT %x\n",value);
    *(uint32_t *)ics660->regs.control = temp | value;
    break;

  case ICS660_MODE:
    value = rx_data->data32;
    temp = *(uint32_t *)ics660->regs.control & ~MODE;
    value = value & MODE;
    pdebug("IO_DEVCTL - ICS660_MODE %x\n",value);
    *(uint32_t *)ics660->regs.control = temp | value;
    break;

  case ICS660_UPCONVERTER_SELECT:
    value = rx_data->data32;
    temp = *(uint32_t *)ics660->regs.control & ~UPCONVERTER_SELECT;
    value = (value << 4) & UPCONVERTER_SELECT;
    pdebug("IO_DEVCTL - UPCONVERTER_SELECT  %x\n",value);
    *(uint32_t *)ics660->regs.control = temp | value;
    break;

  case ICS660_DATA_SOURCE:
    value = rx_data->data32;
    temp = *(uint32_t *)ics660->regs.control & ~DATA_SOURCE;
    value = (value << 5) & DATA_SOURCE;
    pdebug("IO_DEVCTL - DATA_SOURCE %x\n",value);
    *(uint32_t *)ics660->regs.control = temp | value;
    break;

  case ICS660_FPDP_CLOCK_SEL:
    value = rx_data->data32;
    temp = *(uint32_t *)ics660->regs.control & ~FPDP_CLOCK_SEL;
    value = (value << 6) & FPDP_CLOCK_SEL;
    pdebug("IO_DEVCTL - FPDP_CLOCK_SEL %x\n",value);
    *(uint32_t *)ics660->regs.control = temp | value;
    break;

  case ICS660_FPDP_TERM:
    value = rx_data->data32;
    temp = *(uint32_t *)ics660->regs.control & ~FPDP_TERM;
    value = (value << 7) & FPDP_TERM;
    pdebug("IO_DEVCTL - FPDP_TERM %x\n",value);
    *(uint32_t *)ics660->regs.control = temp | value;
    break;

  case ICS660_SYNC_MASTER:
    value = rx_data->data32;
    temp = *(uint32_t *)ics660->regs.control & ~SYNC_MASTER;
    value = (value << 8) & SYNC_MASTER;
    pdebug("IO_DEVCTL - SYNC_MASTER %x\n",value);
    *(uint32_t *)ics660->regs.control = temp | value;
    break;

  case ICS660_SYNC_TERM:
    value = rx_data->data32;
    temp = *(uint32_t *)ics660->regs.control & ~SYNC_TERM;
    value = (value << 9) & SYNC_TERM;
    pdebug("IO_DEVCTL - SYNC_TERM %x\n",value);
    *(uint32_t *)ics660->regs.control = temp | value;
    break;

  case ICS660_DAC_ENABLE:
    value = rx_data->data32;
    temp = *(uint32_t *)ics660->regs.control & ~DAC_ENABLE;
    value = (value << 10) & DAC_ENABLE;
    pdebug("IO_DEVCTL - DAC_ENABLE %x\n",value);    
    *(uint32_t *)ics660->regs.control = temp | value;
    break;

  case ICS660_DAC_RESET:
    value = rx_data->data32;
    pdebug("IO_DEVCTL - DAC_RESET %x\n",value);
    *(uint32_t *)ics660->regs.dac_reset =  value;
    break;

  case ICS660_TRIGGER:
    value = rx_data->data32;
    temp = *(uint32_t *)ics660->regs.control & ~TRIGGER;
    value = (value << 11) & TRIGGER;
    pdebug("IO_DEVCTL - TRIGGER  %x\n",value);
    *(uint32_t *)ics660->regs.control = temp | value;
    break;

  case ICS660_BANK_WIDTH:
    value = rx_data->int_data;
    pdebug("IO_DEVCTL - BANK_WIDTH %d\n",value);
    *(uint32_t *)ics660->regs.bank_width = (uint32_t) value;
    break;
    
  case ICS660_BANK_LENGTH:
    value = rx_data->int_data;
    pdebug("IO_DEVCTL - BANK_LENGTH %d\n",value);
    *(uint32_t *)ics660->regs.bank_length = (uint32_t) value;
    break;

  case ICS660_PERSISTENCE_REG:
    value = rx_data->int_data;
    pdebug("IO_DEVCTL - PERSISTENCE %d\n",value);
    *(uint32_t *)ics660->regs.persistence = (uint32_t) value;
    break;
    
  case ICS660_TRANS_LENGTH:
    value = rx_data->int_data;
    pdebug("IO_DEVCTL - TRANSMIT_LENGTH: %d\n",value);
    *(uint32_t *)ics660->regs.trans_length = (uint32_t) value;
    break;

  case ICS660_WRITE_SEQUENCER:
    pdebug("IO_DEVCTL - WRITE_SEQUENCER offset: %x  value: %x\n",(uint32_t)rx_data->seq_data.offset,(uint32_t)rx_data->seq_data.value );
    seq_data.offset = rx_data->seq_data.offset;
    seq_data.value  = rx_data->seq_data.value;
    *(uint32_t *)(ics660->mem0 + sequencer_base + (uint32_t)seq_data.offset) 
    = (uint32_t)seq_data.value;
    break;

  case ICS660_LOAD_FILTER:
    pdebug("_ICS660_DRV: LOAD_FILTER\n");
    filter_str.chip = rx_data->filter_str.chip;
    filter_str.channel = rx_data->filter_str.channel;
    filter_str.f_corner = rx_data->filter_str.f_corner;
    filter_str.state_time = rx_data->filter_str.state_time;
    pdebug("_ICS660_DRV: LOAD_FILTER - \n  CHIP %d\n  CHANNEL %d\n  F_CORNER %d\n  STATE_TIME %f\n",filter_str.chip,filter_str.channel,filter_str.f_corner,filter_str.state_time);
    select_chip(ics660->mod_control_reg,filter_str.chip);
    load_filter_taps(ics660->dc60m_regs,filter_str.channel,filter_str.f_corner,
		     filter_str.state_time);
    select_chip(ics660->mod_control_reg,0);
    break;

  case ICS660_LOAD_FREQUENCY:
    pdebug("_ICS660_DRV: LOAD_FREQUENCY\n");
    freq_str.chip = rx_data->freq_str.chip;
    freq_str.channel = rx_data->freq_str.channel;
    freq_str.freq = rx_data->freq_str.freq;
    select_chip(ics660->mod_control_reg,freq_str.chip);
    pdebug("_ICS660_DRV: LOAD_FREQ - \n  CHIP %d\n  CHANNEL %d\n  FREQ %f\n",freq_str.chip,freq_str.channel,freq_str.freq);
    load_frequency(ics660->dc60m_regs,freq_str.channel,freq_str.freq);
    break;

  case ICS660_LOAD_PHASE:
    pdebug("_ICS660_DRV: LOAD_PHASE\n");
    phase_str.chip = rx_data->phase_str.chip;
    phase_str.channel = rx_data->phase_str.channel;
    phase_str.phase = rx_data->phase_str.phase;
    select_chip(ics660->mod_control_reg,phase_str.chip);
    load_phase(ics660->dc60m_regs,phase_str.channel,phase_str.phase);
    break;

  case ICS660_SET_DC_READY:
    pdebug("IO_DEVCTL - SET_DC_READY %x\n",value);
    set_dc_ready(ics660->mod_control_reg);
    break;

  case ICS660_RELEASE_RESETS:
    pdebug("IO_DEVCTL - RELEASE_RESETS %x\n",value);
    release_dc60m_resets(ics660->mod_control_reg,ics660->dc60m_regs);
    break;

  case ICS660_SET_CHIP:
    chip = rx_data->data32;
    select_chip(ics660->mod_control_reg,chip);
    break;

  case ICS660_SET_PAGE:
    chip = rx_data->data32;
    select_chip(ics660->mod_control_reg,chip);
    break;

  case ICS660_MODULE_CONTROL_REG:
    value = rx_data->data32;
    pdebug("IO_DEVCTL - MODULE_CONTROL_REG %d\n",value);
    *(uint32_t *)ics660->mod_control_reg = (uint32_t)value;
    break;

  case ICS660_DC60M_RESET:
    chip = rx_data->dc60m_p.chip;
    value = rx_data->dc60m_p.value;
    select_chip(ics660->mod_control_reg,chip);
    pdebug("IO_DEVCTL - DC60M_RESET - chip %d value %x\n",chip,value);
    *(uint32_t *)ics660->dc60m_regs.reset = (uint32_t) value;
    break;

  case ICS660_MISCELANEOUS:
    chip = rx_data->dc60m_p.chip;
    value = rx_data->dc60m_p.value;
    select_chip(ics660->mod_control_reg,chip);
    pdebug("IO_DEVCTL - MISCELANEOUS - chip %d value %x\n",chip,value);
    *(uint32_t *)ics660->dc60m_regs.miscelaneous = (uint32_t)value;
    break;

  case ICS660_SYNC_MODE:
    chip = rx_data->dc60m_p.chip;
    value = rx_data->dc60m_p.value;
    select_chip(ics660->mod_control_reg,chip);
    pdebug("IO_DEVCTL - SYNC_MODE - chip %d value %x\n",chip,value);
   *(uint32_t *)ics660->dc60m_regs.sync_mode = (uint32_t) value;
    break;

  case ICS660_INTERPOLATION_MODE:
    chip = rx_data->dc60m_p.chip;
    value = rx_data->dc60m_p.value;
    select_chip(ics660->mod_control_reg,chip);
    pdebug("IO_DEVCTL - INTERPOLATION_MODE - chip %d value %x\n",chip,value);
    *(uint32_t *)ics660->dc60m_regs.interpolation_mode = (uint32_t) value;
    break;

  case ICS660_CHANNEL_A_SYNC:
    chip = rx_data->dc60m_p.chip;
    value = rx_data->dc60m_p.value;
    select_chip(ics660->mod_control_reg,chip);
    pdebug("IO_DEVCTL - CHANNEL_A_SYNC - chip %d value %x\n",chip,value);
    *(uint32_t *)ics660->dc60m_regs.channel_a_sync = (uint32_t) value;
    break;

  case ICS660_CHANNEL_B_SYNC:
    chip = rx_data->dc60m_p.chip;
    value = rx_data->dc60m_p.value;
    select_chip(ics660->mod_control_reg,chip);
    pdebug("IO_DEVCTL - CHANNEL_B_SYNC - chip %d value %x\n",chip,value);
    *(uint32_t *)ics660->dc60m_regs.channel_b_sync = (uint32_t) value;
    break;

  case ICS660_CHANNEL_C_SYNC:
    chip = rx_data->dc60m_p.chip;
    value = rx_data->dc60m_p.value;
    select_chip(ics660->mod_control_reg,chip);
    pdebug("IO_DEVCTL - CHANNEL_C_SYNC - chip %d value %x\n",chip,value);
    *(uint32_t *)ics660->dc60m_regs.channel_c_sync = (uint32_t) value;
    break;

  case ICS660_CHANNEL_D_SYNC:
    chip = rx_data->dc60m_p.chip;
    value = rx_data->dc60m_p.value;
    select_chip(ics660->mod_control_reg,chip);
    pdebug("IO_DEVCTL - CHANNEL_D_SYNC - chip %d value %x\n",chip,value);
    *(uint32_t *)ics660->dc60m_regs.channel_d_sync = (uint32_t) value;
    break;

  case ICS660_CHANNEL_FLUSH_MODE:
    chip = rx_data->dc60m_p.chip;
    value = rx_data->dc60m_p.value;
    select_chip(ics660->mod_control_reg,chip);
    pdebug("IO_DEVCTL - CHANNEL_FLUSH_MODE - chip %d value %x\n",chip,value);
    *(uint32_t *)ics660->dc60m_regs.channel_flush_mode = (uint32_t) value;
    break;

  case ICS660_INTERPOLATION_GAIN:
    chip = rx_data->dc60m_p.chip;
    value = rx_data->dc60m_p.value;
    select_chip(ics660->mod_control_reg,chip);
    pdebug("IO_DEVCTL - INTERPOLATION_GAIN - chip %d value %x\n",chip,value);
    *(uint32_t *)ics660->dc60m_regs.interpolation_gain = (uint32_t) value;
    break;

  case ICS660_INTERPOLATION_BYTE0:
    chip = rx_data->dc60m_p.chip;
    value = rx_data->dc60m_p.value;
    select_chip(ics660->mod_control_reg,chip);
    pdebug("IO_DEVCTL - INTERPOLATION_BYTE0 - chip %d value %x\n",chip,value);
    *(uint32_t *)ics660->dc60m_regs.interpolation_byte0 = (uint32_t) value;
    break;

  case ICS660_INTERPOLATION_BYTE1:
    chip = rx_data->dc60m_p.chip;
    value = rx_data->dc60m_p.value;
    select_chip(ics660->mod_control_reg,chip);
    pdebug("IO_DEVCTL - INTERPOLATION_BYTE1 - chip %d value %x\n",chip,value);
    *(uint32_t *)ics660->dc60m_regs.interpolation_byte1 = (uint32_t) value;
    break;

  case ICS660_COUNTER_BYTE0:
    chip = rx_data->dc60m_p.chip;
    value = rx_data->dc60m_p.value;
    select_chip(ics660->mod_control_reg,chip);
    pdebug("IO_DEVCTL - COUNTER_BYTE0 - chip %d value %x\n",chip,value);
    *(uint32_t *)ics660->dc60m_regs.counter_byte0 = (uint32_t) value;
    break;

  case ICS660_COUNTER_BYTE1:
    chip = rx_data->dc60m_p.chip;
    value = rx_data->dc60m_p.value;
    select_chip(ics660->mod_control_reg,chip);
    pdebug("IO_DEVCTL - COUNTER_BYTE1 - chip %d value %x\n",chip,value);
    *(uint32_t *)ics660->dc60m_regs.counter_byte1 = (uint32_t) value;
    break;

  case ICS660_DC_STAT:
    chip = rx_data->dc60m_p.chip;
    value = rx_data->dc60m_p.value;
    select_chip(ics660->mod_control_reg,chip);
    pdebug("IO_DEVCTL - STAT - chip %d value %x\n",chip,value);
    *(uint32_t *)ics660->dc60m_regs.stat = (uint32_t) value;
    break;

  case ICS660_SET_PAGE_REG:
    page = rx_data->pg_reg.page;
    chip = rx_data->pg_reg.chip;
    reg = rx_data->pg_reg.reg;
    value = rx_data->pg_reg.value;
    select_chip(ics660->mod_control_reg,chip);
    pdebug("IO_DEVCTL - PAGE_REG - chip %d page %d reg %d value %x\n",chip,page,reg,value);
    *(uint32_t *)ics660->dc60m_regs.page = page;
    *(uint32_t *)ics660->dc60m_regs.input_registers[(int)reg] = (uint32_t) value;
    break;


  default:
    pdebug("IO_DEVCTL - ERROR - no matching case %d\n",value);
    return(ENOSYS);
  }

  memset(&msg->o, 0, sizeof(msg->o));
  
  msg->o.ret_val = status;

  msg->o.nbytes = nbytes;
  return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
}
