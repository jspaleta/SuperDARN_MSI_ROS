#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/neutrino.h>
#include "_regs_PLX9656.h"
#include "_regs_GC314FS.h"
#include "_prog_conventions.h"

int _DMA_transfer_sg(unsigned int BASE, unsigned int BASE1, unsigned int fifo_offset,
	   	     unsigned int *physical_address, int offset, int size, int DMA_switch, int DMA_channel){

	struct _clockperiod	new, old;
	struct timespec		start, stop, sleep;
	int			temp, addr1, addr2, addr3, addr4;

	if(DMA_switch==0){
		addr1=*(physical_address)+offset;
		addr2=*(physical_address+1)+offset;
		addr3=*(physical_address+2)+offset;
	}
	else{
		addr1=*(physical_address+3)+offset;
		addr2=*(physical_address+4)+offset;
		addr3=*(physical_address+5)+offset;
	}

	
	// get clock resolution
	sleep.tv_sec=0;
	sleep.tv_nsec=1;
	// get the time at the start of the DMA transfer
	clock_gettime(CLOCK_REALTIME, &start);
	// initialize and start the DMA transfer
	if(DMA_channel==0){
		*((uint08*)(BASE+PLX9656_DMACSR0))=	0x8;
		*((uint08*)(BASE+PLX9656_DMACSR0))=	0x0;
		//*((uint32*)(BASE+PLX9656_DMAMODE0))=	0x2ec3;
		//*((uint32*)(BASE+PLX9656_DMAMODE0))=	0x2ac3; 
		//*((uint32*)(BASE+PLX9656_DMAMODE0))=	0xa43;  //32 bit, wait state=0, Input enable, burst disabled, local burst disabled, SG mode, interrupt disabled, local addressing held constant  500samples=2.9ms
		//*((uint32*)(BASE+PLX9656_DMAMODE0))=	0xac3;  //32 bit, wait state=0, Input enable, burst enabled, local burst disabled, SG mode, interrupt disabled, local addressing held constant  500samples=2.9ms
		// RTP: this should be best DMA settings
		*((uint32*)(BASE+PLX9656_DMAMODE0))=	0xa43;  //32 bit, wait state=0, Input enable, burst enabled, local burst enabled, SG mode, interrupt disabled, local addressing held constant  500samples=2.9ms
		// RTP: this was used when troubleshooting DMA problems
		//*((uint32*)(BASE+PLX9656_DMAMODE0))=	0xad3;  //32 bit, wait state=0, Input enable, burst enabled, local burst enabled, SG mode, interrupt disabled, local addressing held constant  500samples=2.9ms
		*((uint32*)(BASE1+GC314FS_PCI_ADD_0))=	addr1;
		*((uint32*)(BASE1+GC314FS_LOC_ADD_0))=	fifo_offset;
		*((uint32*)(BASE1+GC314FS_TRANS_0))=	size<<2;
		*((uint32*)(BASE1+GC314FS_POINT_0))=	0x2018;
		*((uint32*)(BASE1+GC314FS_PCI_ADD_0+0x10))=	addr2;
		*((uint32*)(BASE1+GC314FS_LOC_ADD_0+0x10))=	fifo_offset+0x10000;
		*((uint32*)(BASE1+GC314FS_TRANS_0+0x10))=	size<<2;
		*((uint32*)(BASE1+GC314FS_POINT_0+0x10))=	0x2028;
		*((uint32*)(BASE1+GC314FS_PCI_ADD_0+0x20))=	addr3;
		*((uint32*)(BASE1+GC314FS_LOC_ADD_0+0x20))=	fifo_offset+0x20000;
		*((uint32*)(BASE1+GC314FS_TRANS_0+0x20))=	size<<2;
		*((uint32*)(BASE1+GC314FS_POINT_0+0x20))=	0x203a;
		*((uint32*)(BASE+PLX9656_DMADPR0))=	0x2008;
		*((uint08*)(BASE+PLX9656_DMACSR0))=	0x1;
		*((uint08*)(BASE+PLX9656_DMACSR0))=	0x3;
		while ( (*((uint08*)(BASE+PLX9656_DMACSR0)) & 0x10) != 0x10 ){
			// see how long we have been waiting for the DMA x-fer
			clock_gettime(CLOCK_REALTIME, &stop);
			// if longer than 2 seconds, abort the x-fer
			if ( (stop.tv_sec-start.tv_sec) > DMA_TIMEOUT ){
				// abort DMA x-fer
				*((uint08*)(BASE+PLX9656_DMACSR0))=	0x0;
				//*((uint08*)(BASE+PLX9656_CNTRL))=	0x10000;
				*((uint08*)(BASE+PLX9656_DMACSR0))=	0x4;
				fprintf(stderr, "DMA transfer not able to complete!\n");
                                fflush(stderr);
				return -1;
			}
			// sleep for 10 us
			temp=clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
		}
	}
	if(DMA_channel==1){
		*((uint08*)(BASE+PLX9656_DMACSR1))=	0x8;
		*((uint08*)(BASE+PLX9656_DMACSR1))=	0x0;
		//*((uint32*)(BASE+PLX9656_DMAMODE1))=	0x2ec3;
		//*((uint32*)(BASE+PLX9656_DMAMODE1))=	0x2ac3; 
		//*((uint32*)(BASE+PLX9656_DMAMODE1))=	0xa43;  //32 bit, wait state=0, Input enable, burst disabled, local burst disabled, SG mode, interrupt disabled, local addressing held constant  500samples=2.9ms
		//*((uint32*)(BASE+PLX9656_DMAMODE1))=	0xac3;  //32 bit, wait state=0, Input enable, burst enabled, local burst disabled, SG mode, interrupt disabled, local addressing held constant  500samples=2.9ms
		// RTP: this should be best DMA settings
		*((uint32*)(BASE+PLX9656_DMAMODE1))=	0xa43;  //32 bit, wait state=0, Input enable, burst enabled, local burst enabled, SG mode, interrupt disabled, local addressing held constant  500samples=2.9ms
		// RTP: this was used when troubleshooting DMA problems
		//*((uint32*)(BASE+PLX9656_DMAMODE1))=	0xad3;  //32 bit, wait state=0, Input enable, burst enabled, local burst enabled, SG mode, interrupt disabled, local addressing held constant  500samples=2.9ms
		*((uint32*)(BASE1+GC314FS_PCI_ADD_0+0x30))=	addr1;
		*((uint32*)(BASE1+GC314FS_LOC_ADD_0+0x30))=	fifo_offset;
		*((uint32*)(BASE1+GC314FS_TRANS_0+0x30))=	size<<2;
		*((uint32*)(BASE1+GC314FS_POINT_0+0x30))=	0x2048;
		*((uint32*)(BASE1+GC314FS_PCI_ADD_0+0x40))=	addr2;
		*((uint32*)(BASE1+GC314FS_LOC_ADD_0+0x40))=	fifo_offset+0x10000;
		*((uint32*)(BASE1+GC314FS_TRANS_0+0x40))=	size<<2;
		*((uint32*)(BASE1+GC314FS_POINT_0+0x40))=	0x2058;
		*((uint32*)(BASE1+GC314FS_PCI_ADD_0+0x50))=	addr3;
		*((uint32*)(BASE1+GC314FS_LOC_ADD_0+0x50))=	fifo_offset+0x20000;
		*((uint32*)(BASE1+GC314FS_TRANS_0+0x50))=	size<<2;
		*((uint32*)(BASE1+GC314FS_POINT_0+0x50))=	0x206a;
		*((uint32*)(BASE+PLX9656_DMADPR1))=	0x2038;
		*((uint08*)(BASE+PLX9656_DMACSR1))=	0x1;
		*((uint08*)(BASE+PLX9656_DMACSR1))=	0x3;
		while ( (*((uint08*)(BASE+PLX9656_DMACSR1)) & 0x10) != 0x10 ){
			// see how long we have been waiting for the DMA x-fer
			clock_gettime(CLOCK_REALTIME, &stop);
			// if longer than 2 seconds, abort the x-fer
			if ( (stop.tv_sec-start.tv_sec) > DMA_TIMEOUT ){
				// abort DMA x-fer
				*((uint08*)(BASE+PLX9656_DMACSR1))=	0x0;
				//*((uint08*)(BASE+PLX9656_CNTRL))=	0x10000;
				*((uint08*)(BASE+PLX9656_DMACSR1))=	0x4;
				fprintf(stderr, "DMA transfer not able to complete!\n");
                                fflush(stderr);
				return -1;
			}
			// sleep for 10 us
			temp=clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
		}
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	//fprintf(stderr,"Time for DMA x-fer of %d samples is %d nsecs\n", size, stop.tv_nsec-start.tv_nsec);
        //fflush(stderr);
	return 0;
}
