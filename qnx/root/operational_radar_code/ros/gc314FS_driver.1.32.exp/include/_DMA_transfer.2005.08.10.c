#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/neutrino.h>
#include "_regs_PLX9656.h"
#include "_prog_conventions.h"

int _DMA_transfer(unsigned int BASE, unsigned int fifo_offset, unsigned int physical_address, int interrupt, int size){

	struct _clockperiod	new, old;
	struct timespec		start, stop, sleep;
	int			temp;

	// get clock resolution
	temp=ClockPeriod(CLOCK_REALTIME, 0, &old, 0);
	sleep.tv_sec=0;
	sleep.tv_nsec=old.nsec;
	sleep.tv_nsec=1;
	// get the time at the start of the DMA transfer
	clock_gettime(CLOCK_REALTIME, &start);
	// initialize and start the DMA transfer
	*((uint08*)(BASE+PLX9656_DMACSR1))=	0x808;
	*((uint08*)(BASE+PLX9656_DMACSR1))=	0x0;
	*((uint32*)(BASE+PLX9656_DMAMODE1))=	0x9c3;
	*((uint32*)(BASE+PLX9656_DMAPADR1))=	physical_address;
	*((uint32*)(BASE+PLX9656_DMALADR1))=	fifo_offset;
	*((uint32*)(BASE+PLX9656_DMASIZ1))=	size;
	*((uint32*)(BASE+PLX9656_DMADPR1))=	0x0;
	*((uint08*)(BASE+PLX9656_DMACSR1))=	0x1;
	*((uint08*)(BASE+PLX9656_DMACSR1))=	0x3;
	printf("blahlbaksjd\n");
	while ( (*((uint08*)(BASE+PLX9656_DMACSR1)) & 0x10) != 0x10 ){
		// see how long we have been waiting for the DMA x-fer
		clock_gettime(CLOCK_REALTIME, &stop);
		// if longer than 2 seconds, abort the x-fer
		if ( (stop.tv_sec-start.tv_sec) > 3 ){
			// abort DMA x-fer
			*((uint08*)(BASE+PLX9656_DMACSR1))=	0x0;
			*((uint08*)(BASE+PLX9656_CNTRL))=	0x10000;
			*((uint08*)(BASE+PLX9656_DMACSR1))=	0x4;
			fprintf(stderr, "DMA transfer not able to complete!\n");
			return -1;
		}
		// sleep for 10 us
		temp=clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
	}

	return 1;
}
