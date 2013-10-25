#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "_prog_conventions.h"
#include "_regs_GC314FS.h"
#include "../../include/gc314FS.h"

int _config_GC314FS(char *BASE1, int clock_source, int cardnumber){

	int	temp;
	struct	timespec sleep;
        int version=1;
        int value;
	sleep.tv_sec=0;
	sleep.tv_nsec=10000;

	// set to big-endian mode
	*((uint32*)(BASE1+GC314FS_GCSR))	&= 0xffffffef;
	// set to little-endian mode
	//*((uint32*)(BASE1+GC314FS_GCSR))	|= 0x00000010;
	// enable A/Ds for all three inputs
	*((uint32*)(BASE1+GC314FS_GCSR))	&= 0xfffff8ff;
	// set term count to prevent PCI lockup
	*((uint32*)(BASE1+GC314FS_TERMCNT))	 = 3000;
	// set Frame more to include CHID and frame counter
	*((uint32*)(BASE1+GC314FS_GCSR))				|=0x00000040;
	// enable local bus termination
	*((uint32*)(BASE1+GC314FS_GCSR))	|= 0x00000020;
	// enable external trigger to SYNC 1
	if((cardnumber==SYNC_MASTER || SYNC_MASTER==-1))	*((uint32*)(BASE1+GC314FS_GCSR))	|= 0x00800000;
	else 			*((uint32*)(BASE1+GC314FS_GCSR))	&= 0xff7fffff;
	//*((uint32*)(BASE1+GC314FS_GCSR))	|= 0x00800000;
	// set as sync bus master
	if((cardnumber==SYNC_MASTER) || (SYNC_MASTER==-1))	*((uint32*)(BASE1+GC314FS_GCSR))	|= 0x01000000;
	//if((cardnumber==SYNC_MASTER) )	*((uint32*)(BASE1+GC314FS_GCSR))	|= 0x01000000;
	else			*((uint32*)(BASE1+GC314FS_GCSR))	&= 0xfeffffff;
        //Set the ADC sensitivity to +13 dbm  dither enabled
        value=0x15;
	*((uint32*)(BASE1+GC314FS_ADC))	= value;
	temp=*((uint32*)(BASE1+GC314FS_ADC));
        if(value==temp) {
          version=316;
          fprintf(stderr,"This appears to be a GC316\n");
        } else {
          version=314;
          fprintf(stderr,"This appears to be a GC314\n");
        }
        fprintf(stderr,"ADC Register Value 0x%x\n",temp);
	// set GC4016 sample size to 16 bits
	*((uint32*)(BASE1+GC314FS_GCSR))	&= 0xfff7ffff;
	// set IF inputs to drive the GC4016s
	*((uint32*)(BASE1+GC314FS_IWBCSR))	&= 0xffffffdf;
	// disable all 3 receiver IF outputs  and disable test sine wave
	*((uint32*)(BASE1+GC314FS_GCSR))	|= 0x00000f00;
	// clear all FIFOs
		// set syn masks to clear on sync1
		//*((uint32*)(BASE1+GC314FS_IWBSM))	= 0x00000220;
		*((uint32*)(BASE1+GC314FS_R1ASM))	= 0x000002a0;
		*((uint32*)(BASE1+GC314FS_R1BSM))	= 0x000002a0;
		*((uint32*)(BASE1+GC314FS_R1CSM))	= 0x000002a0;
		*((uint32*)(BASE1+GC314FS_R1DSM))	= 0x000002a0;
		*((uint32*)(BASE1+GC314FS_R2ASM))	= 0x000002a0;
		*((uint32*)(BASE1+GC314FS_R2BSM))	= 0x000002a0;
		*((uint32*)(BASE1+GC314FS_R2CSM))	= 0x000002a0;
		*((uint32*)(BASE1+GC314FS_R2DSM))	= 0x000002a0;
		*((uint32*)(BASE1+GC314FS_R3ASM))	= 0x000002a0;
		*((uint32*)(BASE1+GC314FS_R3BSM))	= 0x000002a0;
		*((uint32*)(BASE1+GC314FS_R3CSM))	= 0x000002a0;
		*((uint32*)(BASE1+GC314FS_R3DSM))	= 0x000002a0;
		// provide sync to clear fifos	
		//*((uint32*)(BASE1+GC314FS_GCSR))	|= 0x00400000;
		//usleep(10);
		/*  SYNC1_ON: */
        	*((uint32*)(BASE1+GC314FS_GCSR)) |=0x00200000;
       	 	usleep(20);     
		/*  SYNC1_OFF: */
        	*((uint32*)(BASE1+GC314FS_GCSR)) &=0xffdfffff;
        	usleep(20);     

	// set and enable clock sources
	if (clock_source==CLOCK_INTERNAL){
		// enable onboard oscillator
		*((uint32*)(BASE1+GC314FS_GCSR))	|= 0x00000004;
		sleep.tv_nsec=20000000;	
		clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
		// select internal sample clock
		*((uint32*)(BASE1+GC314FS_GCSR))	&= 0xfffffff7;
	}
	else if (clock_source==CLOCK_EXTERNAL){
		// disable internal oscillator
		*((uint32*)(BASE1+GC314FS_GCSR))	&= 0xfffffffb;
		sleep.tv_nsec=20000000;	
		clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
		// select external sample clock
		*((uint32*)(BASE1+GC314FS_GCSR))	|= 0x00000008;	
	}
	else{
		fprintf(stderr,"	Clock source not properly defined");
		return -1;
	}
	// enalbe all 3 receiver IF outputs
	*((uint32*)(BASE1+GC314FS_GCSR))				&=0xfffff8ff;
	temp=*((uint32*)(BASE1+GC314FS_GCSR));
        fprintf(stderr,"GCSR Register Value at end of config: %x\n",temp);
	return version;
}
