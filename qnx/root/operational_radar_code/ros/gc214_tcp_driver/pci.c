#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#ifdef __QNX__
  #include <hw/pci.h>
  #include <hw/inout.h>
  #include <sys/neutrino.h>
  #include <sys/mman.h>
#endif
#include "registers.h"
#include "_prog_conventions.h"

#define VENDOR_ID 0x10b5
#define DEVICE_ID 0x9080


extern int verbose;

int initPCI9080(int *ad_irq,unsigned int *BASEA,unsigned int *BASEB,unsigned int *BASEC,int pci_index,int print){
  volatile unsigned char	*BASE0, *BASE1;
  unsigned int *mmap_io_ptr;
  unsigned lastbus, version, hardware, bus, device;
  long unsigned temp,temp_out,temp_in, localbase;

  long write_PCI9080_PCICR=0x107;
  long write_PCI9080_CNTRL=0x1007e;
  
  int pci_handle=-1 ;
  unsigned int 	BASEIO;

  unsigned flags;
#ifdef __QNX__
  struct _pci_config_regs pci_reg;

    /* CONNECT TO PCI SERVER */
	pci_handle=pci_attach(flags);
	if (pci_handle == -1){
		perror("Cannot attach to PCI system");
		return -1;
	}

    /* CHECK FOR PCI BUS */
	temp=pci_present(&lastbus, &version, &hardware);
	if(temp != PCI_SUCCESS){
		perror("Cannot find PCI BIOS");
		return -1;
	}

    /* FIND DEVICE */
	temp=pci_find_device(DEVICE_ID, VENDOR_ID, pci_index, &bus, &device);
	if(temp != PCI_SUCCESS){
		perror("Cannot find GC314-PCI/FS Digital Receiver Card");
		return -1;
	}
	
    /* READ THE DEVICE PCI CONFIGURATION */
	temp=pci_read_config32(bus, device, 0, 16, (char *)&pci_reg);
	if(temp != PCI_SUCCESS){
		perror("Cannot read from configuration space of the GC314 PCI/FS digital receiver card");
		return -1;
	}
	BASEIO=(int)pci_reg.Base_Address_Regs[1]-1;
//  *BASE1=(long) pci_reg.Base_Address_Regs[0];
//  *BASE2=(long) pci_reg.Base_Address_Regs[1]-1;
//  localbase=(long) pci_reg.Base_Address_Regs[1]-1;
//  *BASE3=(long) pci_reg.Base_Address_Regs[2];

    /* ALLOW I/O ACCESS ON THIS THREAD */
	temp=ThreadCtl(_NTO_TCTL_IO,0);
	if (temp==-1){
		perror("Unable to attach I/O privileges to thread");
		return -1;
	}

    /* MAP THE IO SPACE TO BE ABLE TO R/W TO PCI IO */
	mmap_io_ptr=(unsigned int *)mmap_device_io(256, pci_reg.Base_Address_Regs[1]);
	if ((int)mmap_io_ptr == MAP_DEVICE_FAILED){
		perror("Device I/O mapping failed");
		return -1;
	}

 
    /* TRY TO MEMORY MAP TO THE DEVICE REGISTER SPACE */
	BASE0=mmap_device_memory(0, 256, PROT_EXEC|PROT_READ|PROT_WRITE|PROT_NOCACHE,0, pci_reg.Base_Address_Regs[0]);

	BASE1=mmap_device_memory(0, 1048576, PROT_EXEC|PROT_READ|PROT_WRITE|PROT_NOCACHE,0, pci_reg.Base_Address_Regs[2]);

	if ((BASE0 == MAP_FAILED) | (BASE1 == MAP_FAILED)){
		perror("Device Memory mapping failed");
		return -1;
	}

  /* PRINT PLX9656 PARAMETERS */
	if (print == 1){
		printf("	PCI DEVICE PARAMETERS:\n");
		printf("	  lastbus=		%d\n", lastbus);
		printf("	  version=		%d\n", version);
		printf("	  hardware=		%d\n", hardware);
		printf("	  bus=			%d\n", bus);
		printf("	  device=		%d\n", device);
		printf("	MEMORY ALLOCATION:\n");
		printf("	  MEM Base0=		0x%x\n", pci_reg.Base_Address_Regs[0]);
		printf("	  MEM Base1=		0x%x\n", pci_reg.Base_Address_Regs[2]);
		printf("	  IO Base Address=	0x%x\n", pci_reg.Base_Address_Regs[1]);
		printf("	  Mapped BASE0=		0x%x\n", BASE0);
		printf("	  Mapped BASE1=		0x%x\n", BASE1);
	}
 

   /* TRY TO READ PCI DEVICE PARAMETERS */

	pci_write_config16(bus, device, reg_PCI9080_PCICR, 1, &write_PCI9080_PCICR);

  /* PRINT PLX9656 REGISTERS */
	if (print == 1){
		printf("	PCI REGISTERS Initial Values:\n");
		printf("	  BIGEND=		0x%x\n", *((uint16*)(BASE0+reg_PCI9080_BIGEND)) );
		printf("	  CNTRL=		0x%x\n", *((uint32*)(BASE0+reg_PCI9080_CNTRL))  );
		printf("	  INTCSR=		0x%x\n", *((uint32*)(BASE0+reg_PCI9080_INTCSR)) );
	}

	*((uint16*)(BASE0+reg_PCI9080_BIGEND))=0x00;
	*((uint32*)(BASE0+reg_PCI9080_CNTRL))=write_PCI9080_CNTRL;
	temp_in=*((uint32*)(BASE0+reg_PCI9080_INTCSR));
	*((uint32*)(BASE0+reg_PCI9080_INTCSR))=temp_in | 0x00011100;

  /* PRINT PLX9656 REGISTERS */
	if (print == 1){
		printf("	PCI REGISTERS Requested,Final Values:\n");
		printf("	  16bit:BIGEND=		0x%x 0x%x\n", 0x00,*((uint16*)(BASE0+reg_PCI9080_BIGEND))              );
		printf("	  32bit:CNTRL=		0x%x 0x%x\n", write_PCI9080_CNTRL,*((uint32*)(BASE0+reg_PCI9080_CNTRL)));
		printf("	  32bit:INTCSR=		0x%x 0x%x\n", 0x00011100,*((uint32*)(BASE0+reg_PCI9080_INTCSR))        );
	}

 
	*BASEA=(int)BASE0;
	*BASEB=(int)BASE1;
	*BASEC=(unsigned int)pci_reg.Base_Address_Regs[2];
	*ad_irq=pci_reg.Interrupt_Line;

#endif
        return write_PCI9080_CNTRL;
}
