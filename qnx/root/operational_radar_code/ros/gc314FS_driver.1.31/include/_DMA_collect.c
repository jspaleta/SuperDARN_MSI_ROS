#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/neutrino.h>
#include "_regs_PLX9656.h"
#include "_regs_GC314FS.h"
#include "_prog_conventions.h"

extern int version;

int wait_on_fifo_lvl(unsigned int BASE1, int channel){
	struct timespec		start, stop, sleep;
	int			status, temp, stat;
        double			elapsed=0.0;
	sleep.tv_sec=0;
	sleep.tv_nsec=50000;
	temp=clock_gettime(CLOCK_REALTIME, &start);
	stat=0;
	status=0;
	while(1){
		if(channel==0) status=*((uint32*)(BASE1+GC314FS_R1ACSR));
		else if(channel==1) status=*((uint32*)(BASE1+GC314FS_R1BCSR));
		else if(channel==2) status=*((uint32*)(BASE1+GC314FS_R1CCSR));
		else if(channel==3) status=*((uint32*)(BASE1+GC314FS_R1DCSR));
		//status|=*((uint32*)(BASE1+GC314FS_R1BCSR));
		//status|=*((uint32*)(BASE1+GC314FS_R1CCSR));
		temp=clock_gettime(CLOCK_REALTIME, &stop);
		if( (status & 0x00080000) == 0x00080000){
			fprintf(stderr,"FIFO OVERFLOW2! %x\n",status);
                        fflush(stderr);                 
			stat=-1;
		} 
		if( (status & 0x00020000) == 0x00020000){
			break;
 
		}
                elapsed=stop.tv_sec-start.tv_sec;
                elapsed*=1E9;
                elapsed+=stop.tv_nsec-start.tv_nsec;
		if ( elapsed > DMA_NSEC_TIMEOUT ){
                        stat=-1;
			fprintf(stderr, "FIFO never reached desired level LVL! channel=%d %lf %lf\n",channel,elapsed,(double)DMA_NSEC_TIMEOUT);
                        fflush(stderr);
			break;
		}
		else{
			temp=clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
		}
			
	}
	return stat;
}

int wait_on_fifo_finish(unsigned int BASE1, int channel){
	struct timespec		start, stop, sleep;
	int			temp;
	unsigned short		status;
	FILE			*Fout;
        int			rstatus=0;
        unsigned long		elapsed;
        int			frame,frame_count=0;
        if (version==316) frame_count=0;
        if (version==314) frame_count=1;
	sleep.tv_sec=0;
	sleep.tv_nsec=50000;
	temp=clock_gettime(CLOCK_REALTIME, &start);
	status=0;
	while(1){
		if(channel==0) frame=*((uint32*)(BASE1+GC314FS_R1AID));
		else if(channel==1) frame=*((uint16*)(BASE1+GC314FS_R1BID));
		else if(channel==2) frame=*((uint16*)(BASE1+GC314FS_R1CID));
		else if(channel==3) frame=*((uint16*)(BASE1+GC314FS_R1DID));
		temp=clock_gettime(CLOCK_REALTIME, &stop);
                
		if( frame > frame_count ){
			//fprintf(stderr,"Yay! %d  %d\n", status, channel);
			status=*((uint32*)(BASE1+GC314FS_R1ACSR));
			if( (status & 0x00080000) == 0x00080000){
				fprintf(stderr,"FIFO OVERFLOW3!\n");
                                fflush(stderr); 
                                rstatus=-1;
			} else rstatus=0;
			break;
		}
                elapsed=stop.tv_sec-start.tv_sec;
                elapsed*=1E9;
                elapsed+=stop.tv_nsec-start.tv_nsec;
		if ( elapsed > DMA_NSEC_TIMEOUT ){
			fprintf(stderr, "FIFO never reached desired level FINISH!, channel=%d last frame: %d\n", channel,frame);
			fprintf(stderr,"PISR status=%x\n", *((uint32*)(BASE1+GC314FS_PISR)) );
                        fflush(stderr);
                        rstatus=-1;
			break;
		}
		else{
			temp=clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
		}
			
	}
	return rstatus;
}
