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
	int			temp, int_id;
	unsigned int		temp_value;
	struct sigevent 	event, notify;
	int			timeout;


	timeout=10000;
	
	*((uint08*)(BASE+PLX9656_DMACSR1))=	0x8;
	SIGEV_INTR_INIT( &event ); 
	SIGEV_UNBLOCK_INIT( &notify );
	// attach an interrupt event to this function
	int_id=InterruptAttachEvent(interrupt, &event, 0);
	temp=InterruptUnmask(interrupt, -1);
	//printf("int_id=%d\n", int_id);
	// enable dma interrupts
	temp_value=*((uint32*)(BASE+PLX9656_INTCSR));
	*((uint32*)(BASE+PLX9656_INTCSR)) |= 0x00080100;
	// get clock resolution
	temp=ClockPeriod(CLOCK_REALTIME, 0, &old, 0);
	sleep.tv_sec=0;
	sleep.tv_nsec=old.nsec;
	sleep.tv_nsec=1;
	// get the time at the start of the DMA transfer
	clock_gettime(CLOCK_REALTIME, &start);
	// initialize and start the DMA transfer
	*((uint08*)(BASE+PLX9656_DMACSR1))=	0x8;
	*((uint08*)(BASE+PLX9656_DMACSR1))=	0x0;
	*((uint32*)(BASE+PLX9656_DMAMODE1))=	0x20dc3;
	*((uint32*)(BASE+PLX9656_DMAPADR1))=	physical_address;
	*((uint32*)(BASE+PLX9656_DMALADR1))=	fifo_offset;
	*((uint32*)(BASE+PLX9656_DMASIZ1))=	size;
	*((uint32*)(BASE+PLX9656_DMADPR1))=	0x8;
	*((uint08*)(BASE+PLX9656_DMACSR1))=	0x1;
	*((uint08*)(BASE+PLX9656_DMACSR1))=	0x3;
	while(1){
		temp=TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_INTR, &notify, &timeout, NULL);
		temp=clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
		temp=InterruptWait(0,NULL);
		if ( (*((uint08*)(BASE+PLX9656_DMACSR1)) & 0x10 ) == 0x10 ){
			InterruptDetach(int_id);
			return 1;
		}
		else printf("interrupt triggered, but not done!\n");
		clock_gettime(CLOCK_REALTIME, &stop);
		if ( (stop.tv_sec-start.tv_sec) > 1 ){
			InterruptDetach(int_id);
			fprintf(stderr, "DMA Transfer Failed!\n");
			return -1;
		}
	}
}
