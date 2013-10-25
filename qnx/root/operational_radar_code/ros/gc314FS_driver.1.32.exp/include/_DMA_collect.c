#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/neutrino.h>
#include "_regs_PLX9656.h"
#include "_regs_GC314FS.h"
#include "_prog_conventions.h"
#include "../../include/gc314FS.h"
extern int version;
extern int channel_flag[4];

int check_fifo_lvl(unsigned int BASE1, int channel){
	struct timespec		start, stop, sleep;
	int			count,status1,status2,status3, temp, stat;
        double		elapsed=0;
	sleep.tv_sec=0;
	sleep.tv_nsec=500000;
	temp=clock_gettime(CLOCK_REALTIME, &start);
	stat=0;
	status1=0;
        count=0;
/*
		if(channel==CHANNEL_A) status1=*((uint32*)(BASE1+GC314FS_R1ACSR));
		if(channel==CHANNEL_B) status1=*((uint32*)(BASE1+GC314FS_R1BCSR));
		if(channel==CHANNEL_C) status1=*((uint32*)(BASE1+GC314FS_R1CCSR));
		if(channel==CHANNEL_D) status1=*((uint32*)(BASE1+GC314FS_R3DCSR));
		if( (status1 & 0x00020000) == 0x00020000){
			fprintf(stderr,"FIFO EMPTY\n");
		}
*/
}
int wait_on_fifo_lvl(unsigned int BASE1, int channel){
	struct timespec		start, stop, sleep;
	int			count,status1,status2,status3, temp, stat;
        double		elapsed=0;
	sleep.tv_sec=0;
	sleep.tv_nsec=500000;
	temp=clock_gettime(CLOCK_REALTIME, &start);
	stat=0;
	status1=0;
        count=0;
		//if(channel==CHANNEL_A) lvl=*((uint32*)(BASE1+GC314FS_R3ALVL));
		//if(channel==CHANNEL_B) lvl=*((uint32*)(BASE1+GC314FS_R3BLVL));
		//if(channel==CHANNEL_C) lvl=*((uint32*)(BASE1+GC314FS_R3CLVL));
		//if(channel==CHANNEL_D) lvl=*((uint32*)(BASE1+GC314FS_R3DLVL));
		//if(channel==CHANNEL_A) status1=*((uint32*)(BASE1+GC314FS_R3ACSR));
		//if(channel==CHANNEL_B) status1=*((uint32*)(BASE1+GC314FS_R3BCSR));
		//if(channel==CHANNEL_C) status1=*((uint32*)(BASE1+GC314FS_R3CCSR));
/*
		if(channel==CHANNEL_D) {
                  status1=*((uint32*)(BASE1+GC314FS_R3DCSR));
	 	  fprintf(stderr,"wait_on_fifo Initial status: %x chan: %d count: %d\n",status1,channel,count);
                  fflush(stderr);                 
		  if( (status1 & 0x00080000) == 0x00080000){
			fprintf(stderr,"wait_on_fifo Initial FIFO OVERFLOW! %x chan: %d count: %d\n",status1,channel,count);
                        fflush(stderr);                 
			stat=-1;
		  } 
		  if( (status1 & 0x00040000) == 0x00040000){
			fprintf(stderr,"wait_on_fifo Initial FIFO FULL! %x chan: %d count: %d\n",status1,channel,count);
                        fflush(stderr);                 
			stat=-1;
		  }
		  if( (status1 & 0x00020000) == 0x00020000){
			fprintf(stderr,"wait_on_fifo Initial FIFO LVL Reached! %x chan: %d count: %d\n",status1,channel,count);
                        fflush(stderr);                 
		  } 
		}
*/
	while(1){
		if(channel==CHANNEL_A) status1=*((uint32*)(BASE1+GC314FS_R3ACSR));
		if(channel==CHANNEL_B) status1=*((uint32*)(BASE1+GC314FS_R3BCSR));
		if(channel==CHANNEL_C) status1=*((uint32*)(BASE1+GC314FS_R3CCSR));
		if(channel==CHANNEL_D) {
                  status1=*((uint32*)(BASE1+GC314FS_R3DCSR));
		}
                if(channel_flag[channel]==0) {
                  channel_flag[channel]=1; 
		}
/*
		if(channel==CHANNEL_A) status2=*((uint32*)(BASE1+GC314FS_R2ACSR));
		if(channel==CHANNEL_B) status2=*((uint32*)(BASE1+GC314FS_R2BCSR));
		if(channel==CHANNEL_C) status2=*((uint32*)(BASE1+GC314FS_R2CCSR));
		if(channel==CHANNEL_D) status2=*((uint32*)(BASE1+GC314FS_R2DCSR));
		if(channel==CHANNEL_A) status3=*((uint32*)(BASE1+GC314FS_R3ACSR));
		if(channel==CHANNEL_B) status3=*((uint32*)(BASE1+GC314FS_R3BCSR));
		if(channel==CHANNEL_C) status3=*((uint32*)(BASE1+GC314FS_R3CCSR));
		if(channel==CHANNEL_D) status3=*((uint32*)(BASE1+GC314FS_R3DCSR));
*/
		//status|=*((uint32*)(BASE1+GC314FS_R1BCSR));
		//status|=*((uint32*)(BASE1+GC314FS_R1CCSR));
		temp=clock_gettime(CLOCK_REALTIME, &stop);
		if( (status1 & 0x00080000) == 0x00080000){
			fprintf(stderr,"FIFO OVERFLOW2! %x count: %d stop: %d %d\n",status1,count,stop.tv_sec,stop.tv_nsec);
                        fflush(stderr);                 
			stat=-1;
		} 
		if( (status1 & 0x00020000) == 0x00020000){
			break;
 
		}
                elapsed=stop.tv_sec-start.tv_sec;
                elapsed*=1E9;
                elapsed+=stop.tv_nsec-start.tv_nsec;
		if ( elapsed > DMA_NSEC_TIMEOUT ){
                        stat=-1;
			fprintf(stderr, "FIFO never reached desired level LVL! channel=%d count: %d nsecs: %g stop: %d %d\n"
                        ,channel,count,elapsed,stop.tv_sec,stop.tv_nsec);
			fprintf(stderr,"PISR status=%x\n", *((uint32*)(BASE1+GC314FS_PISR)) );
			fprintf(stderr,"R3xCSR  status=%x\n", status1 );
			//fprintf(stderr,"R2xCSR  status=%x\n", status2 );
			//fprintf(stderr,"R3xCSR  status=%x\n", status3 );
			//fprintf(stderr,"R1xLVL  lvl=%d\n", lvl );
                        if ( (*((uint32*)(BASE1+GC314FS_CPLL)) & 0x00000004) != 0x00000004 ){
			        fprintf(stderr, "  PLL CLOCK LOSS LOCK\n");
                        }

                        fflush(stderr);
			break;
		}
		else{
			temp=clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
		}
			
		count++;
	}
	return stat;
}

int wait_on_fifo_finish(unsigned int BASE1, int channel){
	struct timespec		start, stop, sleep;
	int			temp;
	unsigned short		status;
	FILE			*Fout;
        int			rstatus=0;
        double 		elapsed;
        int			frame,frame_count=0;
        if (version==316) frame_count=0;
        if (version==314) frame_count=1;
	sleep.tv_sec=0;
	sleep.tv_nsec=500000;
	temp=clock_gettime(CLOCK_REALTIME, &start);
	status=0;
	while(1){
		if(channel==CHANNEL_A) frame=*((uint32*)(BASE1+GC314FS_R1AID));
		if(channel==CHANNEL_B) frame=*((uint16*)(BASE1+GC314FS_R1BID));
		if(channel==CHANNEL_C) frame=*((uint16*)(BASE1+GC314FS_R1CID));
		if(channel==CHANNEL_D) frame=*((uint16*)(BASE1+GC314FS_R1DID));
		temp=clock_gettime(CLOCK_REALTIME, &stop);
                
		if( frame > frame_count ){
			//fprintf(stderr,"Yay! %d  %d\n", status, channel);
			if(channel==CHANNEL_A) status=*((uint32*)(BASE1+GC314FS_R1ACSR));
			if(channel==CHANNEL_B) status=*((uint32*)(BASE1+GC314FS_R1BCSR));
			if(channel==CHANNEL_C) status=*((uint32*)(BASE1+GC314FS_R1CCSR));
			if(channel==CHANNEL_D) status=*((uint32*)(BASE1+GC314FS_R1DCSR));
			if( (status & 0x00080000) == 0x00080000){
				fprintf(stderr,"FIFO OVERFLOW3! stop: %d %d\n",stop.tv_sec,stop.tv_nsec);
                                fflush(stderr); 
                                rstatus=-1;
			} else rstatus=0;
			break;
		}
                elapsed=stop.tv_sec-start.tv_sec;
                elapsed*=1E9;
                elapsed+=stop.tv_nsec-start.tv_nsec;
		if ( elapsed > DMA_NSEC_TIMEOUT ){
			fprintf(stderr, "FIFO never reached desired level FINISH!, channel=%d last frame: %d nsecs: %g stop: %d %d\n"
                          , channel,frame,elapsed,stop.tv_sec,stop.tv_nsec);
			fprintf(stderr,"PISR status=%x\n", *((uint32*)(BASE1+GC314FS_PISR)) );
			fprintf(stderr,"R1xCSR  status=%x\n", status );
                        if ( (*((uint32*)(BASE1+GC314FS_CPLL)) & 0x00000004) != 0x00000004 ){
                                fprintf(stderr, "  PLL CLOCK LOSS LOCK\n");
                        }

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
