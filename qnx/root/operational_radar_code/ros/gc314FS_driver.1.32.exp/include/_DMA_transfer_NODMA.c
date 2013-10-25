#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/neutrino.h>
#include "_regs_PLX9656.h"
#include "_regs_GC314FS.h"
#include "_prog_conventions.h"

int _DMA_transfer_NODMA(unsigned int BASE, unsigned int BASE1, unsigned int fifo_offset, unsigned int physical_address, int interrupt, int size){

	struct _clockperiod	new, old;
	struct timespec		start, stop, sleep;
	int			temp, i;

	// get clock resolution
	sleep.tv_sec=0;
	sleep.tv_nsec=1;
	// get the time at the start of the DMA transfer
	clock_gettime(CLOCK_REALTIME, &start);

	for (i=0;i<size;i++){
		*((uint32*)(physical_address+4*i)) = *((uint32*)(BASE1+fifo_offset));	
	}	
        printf("In NODMA transfer\n");
	clock_gettime(CLOCK_REALTIME, &stop);
	//printf("1 Million reads takes %d ns\n", stop.tv_nsec-start.tv_nsec);	
	return 1;
}
