#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#ifdef __QNX__
  #include <hw/pci.h>
  #include <hw/inout.h>
  #include <sys/neutrino.h>
#endif
#ifdef __linux__
  #include "uio_helper.h"
#endif
#include <sys/mman.h>
#include "rtypes.h"
#include "_prog_conventions.h"

#define VENDOR_ID 0x10b5
#define DEVICE_ID 0x9050
#define MODULE_NAME "GPS-PCI-2U"
extern int verbose;
unsigned int 	BASEIO;


int _open_PLX9050(unsigned int *BASEA, unsigned int *BASEB, int *pci_handle, unsigned int *mmap_io_ptr, int *interrupt_line){

	uint8_t	*BASE0=NULL, *BASE1=NULL;
	unsigned		flags, lastbus, version, hardware, bus, device;
	int			temp;
	unsigned		pci_index=0;
#ifdef __QNX__
	struct			_pci_config_regs pci_reg;

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
	mmap_io_ptr=(unsigned int *)mmap_device_io(256, pci_reg.Base_Address_Regs[1]);
	if ((int)mmap_io_ptr == MAP_DEVICE_FAILED){
		perror("Device I/O mapping failed");
		return -1;
	}

    /* TRY TO MEMORY MAP TO THE DEVICE REGISTER SPACE */
	BASE0=mmap_device_memory(0, 256, PROT_EXEC|PROT_READ|PROT_WRITE|PROT_NOCACHE, 0, pci_reg.Base_Address_Regs[0]);
	BASE1=mmap_device_memory(0, 1048576, PROT_EXEC|PROT_READ|PROT_WRITE|PROT_NOCACHE, 0, pci_reg.Base_Address_Regs[2]);
	if ((BASE0 == MAP_FAILED) | (BASE1 == MAP_FAILED)){
		perror("Device Memory mapping failed");
		return -1;
	}

    /* TRY TO READ PCI DEVICE PARAMETERS */
	//pci_write_config16(bus, device, PLX9656_PCIICR, 1, &vPLX9656_PCIICR);

	//*((uint16*)(BASE0+PLX9656_BIGEND))=vPLX9656_BIGEND;
	//*((uint16*)(BASE0+PLX9656_BIGEND))=0;
	*((uint32*)(BASE0+0x4c))=0x48;
	temp=*((uint32*)(BASE0+0x4c));
	if (verbose > 1) printf(" INTCSR is %x\n",temp);
	temp=in32(BASEIO+0x4c);
	if (verbose > 1) printf(" INTCSR is %x\n",temp);

    /* PRINT PLX9656 PARAMETERS */
	if (verbose > 1) {
		printf("	PCI DEVICE PARAMETERS:\n");
		printf("	  lastbus=		%d\n", lastbus);
		printf("	  version=		%d\n", version);
		printf("	  hardware=		%d\n", hardware);
		printf("	  bus=			%d\n", bus);
		printf("	  device=		%d\n", device);
		printf("	  interrupt=		%d\n", pci_reg.Interrupt_Line);
		printf("	MEMORY ALLOCATION:\n");
		printf("	  MEM Base0=		0x%x\n", pci_reg.Base_Address_Regs[0]);
		printf("	  MEM Base2=		0x%x\n", pci_reg.Base_Address_Regs[2]);
		printf("	  IO Base Address=	0x%x\n", pci_reg.Base_Address_Regs[1]);
		printf("	  Mapped BASE0=		0x%x\n", BASE0);
		printf("	  Mapped BASE2=		0x%x\n", BASE1);
	}

	*BASEA=(int)BASE0;
	*BASEB=(int)BASE1;
	*interrupt_line=pci_reg.Interrupt_Line;
	return 1;
#else
        int uio_filter=0,fd;
        void* addr=NULL;
        char devicename[32];
        struct uio_info_t *uio_device_list=NULL,*gps_device=NULL;
        struct uio_dev_attr_t *attr = NULL; 
        long unsigned int page_addr=NULL,page_offset=NULL;
        unsigned long page_num,page_size;
        uio_device_list= uio_find_devices(uio_filter);
        while (uio_device_list) {
                uio_get_all_info(uio_device_list);
//                show_uio_info(uio_device_list);
                printf("%d: %d : name=%s, version=%s, events=%d\n",
               uio_device_list->uio_num, strcmp(uio_device_list->name,MODULE_NAME), uio_device_list->name,uio_device_list->version, uio_device_list->event_count);
               if (strcmp(uio_device_list->name,MODULE_NAME)==0) {
                 printf("Found GPS device in list\n");
                 gps_device=uio_device_list;
               }
                uio_device_list = uio_device_list->next;
        }
        if (gps_device==NULL) {
          printf("No GPS device in list\n");
	  return -1;
        }
        uio_get_device_attributes(gps_device);
        attr=gps_device->dev_attrs;
        while (attr) {
              if(strcmp(attr->name,"irq")==0) {
                printf("\t%s=%s\n", attr->name, attr->value);
	        *interrupt_line=atoi(attr->value);
              }
              attr = attr->next;
        }
        sprintf(devicename,"/dev/uio%d",gps_device->uio_num);
        fd = open(devicename,O_RDWR|O_SYNC);
        if (fd < 0) {
                printf("Can't open device file %s\n",devicename);
                return -1;
        }
        printf("Opened Device File: %s\n",devicename);
        // Calculate the mmap page offset for BAS0
        page_size=getpagesize();
        page_num=gps_device->maps[0].addr/page_size;
        page_addr=page_num*page_size;
        page_offset=gps_device->maps[0].addr-page_addr;
        // mmap for BASE0: The PLX configuration space
        BASE0 = page_offset+mmap( NULL, gps_device->maps[0].size,PROT_READ |PROT_WRITE,MAP_SHARED,fd,0*getpagesize());
	printf("	  Mapped BASE0=		%p\n", BASE0);
        printf("            INTSCR=	%p\n",BASE0[0x4c]);
        //turn on interrupts
        BASE0[0x4c]=0x48;
        printf("            INTSCR=	%p\n",BASE0[0x4c]);
        // mmap for BASE0: The GPS configuration space
        page_num=gps_device->maps[1].addr/page_size;
        page_addr=page_num*page_size;
        page_offset=gps_device->maps[1].addr-page_addr;
        BASE1 = page_offset+mmap( NULL, gps_device->maps[1].size, PROT_READ|PROT_WRITE ,MAP_SHARED,fd,1*getpagesize());
	printf("	  Mapped BASE1=		%p\n", BASE1);
	printf("	  PAGE ADDR: %p OFFSET: %p\n", page_addr,page_offset);
        printf("            HDW_CONTROL=	0x%x\n",BASE1[0xF8]);
        printf("            HDW_STATUS=		0x%x\n",BASE1[0xFE]);
        close(fd);
        if ((BASE0 == MAP_FAILED) || (BASE1 == MAP_FAILED)) return -1;
	*BASEA=(int)BASE0;
	*BASEB=(int)BASE1;
        return 0;
#endif
}
