#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <hw/pci.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#include <stdint.h>
#include "_regs_PLX9080.h"
#include "_prog_conventions.h"

#define VENDOR_ID 0x10b5
#define DEVICE_ID 0x9080

unsigned int 	BASEIO;
int		vPLX9080_PCIICR=		0x107;
int		vPLX9080_BIGEND=		0xc0;
int		vPLX9080_CNTRL=			0x7e;

int _open_PLX9080(unsigned int *BASEA, unsigned int *BASEB, unsigned int *BASEB_phys, int *pci_handle, unsigned int *mmap_io_ptr, int *interrupt_line, int print){

	volatile unsigned char	*BASE0, *BASE1;
	unsigned		flags, lastbus, version, hardware, bus, device;
	int			temp;
	unsigned		pci_index=0;
	struct			_pci_config_regs pci_reg;
	unsigned int		*mmap_ptr;

    /* CONNECT TO PCI SERVER */
	*pci_handle=pci_attach(flags);
	if (*pci_handle == -1){
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

    /* ALLOW I/O ACCESS ON THIS THREAD */
	temp=ThreadCtl(_NTO_TCTL_IO,0);
	if (temp==-1){
		perror("Unable to attach I/O privileges to thread");
		return -1;
	}

    /* MAP THE IO SPACE TO BE ABLE TO R/W TO PCI IO */
	mmap_ptr=(unsigned int *)mmap_device_io(256, pci_reg.Base_Address_Regs[2]-1);
	if ((int)mmap_io_ptr == MAP_DEVICE_FAILED){
		perror("Device I/O mapping failed");
		return -1;
	}

    /* TRY TO MEMORY MAP TO THE DEVICE REGISTER SPACE */
	BASE0=mmap_device_memory(0, 1048576, PROT_EXEC|PROT_READ|PROT_WRITE|PROT_NOCACHE, 0, pci_reg.Base_Address_Regs[0]);

	//BASE0=mmap_device_memory(0, 256, PROT_EXEC|PROT_READ|PROT_WRITE|PROT_NOCACHE, 0, pci_reg.Base_Address_Regs[2]-1);

	if ((BASE0 == MAP_FAILED) | (BASE1 == MAP_FAILED)){
		perror("Device Memory mapping failed");
		return -1;
	}

    /* TRY TO READ PCI DEVICE PARAMETERS */
	pci_write_config16(bus, device, PLX9080_PCIICR, 1, &vPLX9080_PCIICR);

	*((uint16*)(BASE0+PLX9080_BIGEND))=vPLX9080_BIGEND;
	*((uint16*)(BASE0+PLX9080_BIGEND))=0;
	*((uint32*)(BASE0+PLX9080_CNTRL))=vPLX9080_CNTRL;

    /* PRINT PLX9080 PARAMETERS */
	if (print == 1){
		printf("	PCI DEVICE PARAMETERS:\n");
		printf("	  lastbus=		%d\n", lastbus);
		printf("	  version=		%d\n", version);
		printf("	  hardware=		%d\n", hardware);
		printf("	  bus=			%d\n", bus);
		printf("	  device=		%d\n", device);
		printf("	MEMORY ALLOCATION:\n");
		printf("	  MEM Base (pci config)=	0x%x\n", pci_reg.Base_Address_Regs[0]);
		printf("	  IO Base1 (card ctrl)=		0x%x\n", pci_reg.Base_Address_Regs[2]);
		printf("	  IO Base2=			0x%x\n", pci_reg.Base_Address_Regs[1]);
		printf("	  Mapped BASE0=			0x%x\n", BASE0);
		printf("	  Mapped BASE1=			0x%x\n", BASE1);
	}
	
	*mmap_io_ptr=mmap_ptr;
	pci_read_config16(bus, device, 0x18, 1, &vPLX9080_PCIICR);
	printf("PCR 0x18 is %x\n", vPLX9080_PCIICR);
	printf("mmap_io_ptr is %x\n", *mmap_io_ptr);
	*BASEA=(int)BASE0;
	//*BASEB=(int)BASE1;
	*BASEB=(unsigned int)pci_reg.Base_Address_Regs[1]-1;
	//*BASEB=mmap_io_ptr-1;
	*BASEB_phys=(unsigned int)pci_reg.Base_Address_Regs[0];
	*interrupt_line=pci_reg.Interrupt_Line;
	printf("BASE0 is %x\n", BASE0);
	printf("memory base + 0x00 is %x\n", *(uint32*)(BASE0));
	return 1;
}
