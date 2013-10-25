/* Program get_pci_base_addr */
#include <stdio.h>
#include  <hw/pci.h>
#include "ics660b.h"

int get_pci_base_addr(int dev_id, int vendor_id, int pci_ind, struct pci_base *ics660_addr)
{
  struct _pci_config_regs pci_reg;
  unsigned lastbus,version,hardware,bus,device,flags;
  int i,j,handle;
  volatile unsigned char *ics660_io;
  uint32_t map0,map1;
  uint16_t pci_cmd;
  
  //fprintf(stderr,"IN GET_PCI_BASE_ADDR\n");
  if( (handle=pci_attach(flags)) == -1 )
    {
      perror("Could not attach to PCI\n");
      return -1;
    }
  //fprintf(stderr,"PCI_ATTACHED\n");
 
  fprintf(stderr,"GET_PCI %d %d %d\n",dev_id, vendor_id, pci_ind);
  if( pci_find_device(dev_id,vendor_id,pci_ind,&bus,&device) != PCI_SUCCESS)
    {
      perror("Cannot find PCI-synth\n");
      return -1;
    }
  //fprintf(stderr,"PCI DEVICE FOUND\n");
  
  if( pci_read_config32(bus,device,0,16,(char *)&pci_reg) != PCI_SUCCESS)
    {
      perror("Cannot read from configuration space of PCI-synth");
      return -1;
    }
  //fprintf(stderr,"READ CONFIGURATION\n");
  
  ics660_addr->io_base = pci_reg.Base_Address_Regs[0];
  ics660_addr->base0 = pci_reg.Base_Address_Regs[1];
  ics660_addr->base1 = pci_reg.Base_Address_Regs[2];
  
  
  //fprintf(stderr,"\nBASE ADDRESS REGS 0 %x \n",ics660_addr->io_base);
  //fprintf(stderr,"BASE ADDRESS REGS 1 %x \n",ics660_addr->base0);
  //fprintf(stderr,"BASE ADDRESS REGS 2 %x \n",ics660_addr->base1);
  

  /* SET PCI WRITE ENABLE */
  pci_read_config16(bus,device,0x04,1,(char *)&pci_cmd);
  //printf("PCI_CMD: %x\n",(unsigned int)pci_cmd);
  
  pci_cmd = pci_cmd | 0x0003;
  
  pci_write_config16(bus,device,0x04,1,(char *)&pci_cmd);
  
  pci_read_config16(bus,device,0x04,1,(char *)&pci_cmd);
  //printf("PCI_CMD: %x\n",(unsigned int)pci_cmd);
  
  
  pci_read_config32(bus,device,0x40,1,(char *)&map0);
  pci_read_config32(bus,device,0x44,1,(char *)&map1);
  
  //printf("MAP 0: %x\n",(unsigned int)map0);
  //printf("MAP 1: %x\n",(unsigned int)map1);
  
  map0 = map0 & 0x000ffcff;
  map1 = map1 & 0x000ffcff;
  
  map0 = map0 | (unsigned int)0xff000033;
  map1 = map1 | (unsigned int)0xffc00033;
  
  //printf("MAP 0: %x\n",(unsigned int)map0);
  //printf("MAP 1: %x\n",(unsigned int)map1);
  
  pci_write_config32(bus,device,0x40,1,(char *)&map0);
  pci_write_config32(bus,device,0x44,1,(char *)&map1);
  
  
  pci_read_config32(bus,device,0x40,1,(char *)&map0);
  pci_read_config32(bus,device,0x44,1,(char *)&map1);
  
  //printf("MAP 0: %x\n",(unsigned int)map0);
  //printf("MAP 1: %x\n",(unsigned int)map1);
  
  
  return 0;
}
