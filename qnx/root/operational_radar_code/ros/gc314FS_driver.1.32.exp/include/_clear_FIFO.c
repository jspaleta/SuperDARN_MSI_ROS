#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "_prog_conventions.h"
#include "_regs_GC314FS.h"
#include "_regs_GC4016.h"
#include "_global.h"
#include "../../include/gc314FS.h"

int extern pci_index;

int _clear_FIFO(char *BASE1, int print, int channel){

	int		i=0,j,k, skip=26;
	int		test[3]={3,2,6};
	unsigned int	GC314FS_GCoffset[3]={0x800,0x1000,0x1800};
	unsigned int 	chan_control_map[4]={0x00,0x80,0x100,0x180};
	unsigned int	*coeffs;
	struct		timespec start, stop;
	int chid;
	unsigned int	*to_write;	
    /* PROGRAM GC4016-1 *****************************************************************/
	clock_gettime(CLOCK_REALTIME, &start);
	//printf("channel %d skip %d samples\n", channel, skip_samples);
        //skip_samples=0;
if(channel==CHANNEL_A){
//	fprintf(stderr,"R3ACSR: %x\n",*((uint32*)(BASE1+GC314FS_R3ACSR)));
//	fprintf(stderr,"R3ASM: %x\n",*((uint32*)(BASE1+GC314FS_R3ASM)));
//	if ( (*((uint32*)(BASE1+GC314FS_R3ACSR)) & 0x10000) == 0x10000 ) {
//          fprintf(stderr,"_clear_FIFO: Channel A input 3 : initial FIFO: empty\n");
//          fflush(stderr);
//        } else {
//          fprintf(stderr,"_clear_FIFO: Channel A input 3 : initial FIFO: not empty\n") ;
//          fflush(stderr);
//        }
	write16(BASE1, GC314FS_R1ACSR,					0x0004);
	write16(BASE1, GC314FS_R2ACSR,					0x0004);
	write16(BASE1, GC314FS_R3ACSR,					0x0004);

	chid=1100+pci_index;
        write32(BASE1, GC314FS_R1AID,           chid<<16);                     
	chid=2100+pci_index;
        write32(BASE1, GC314FS_R2AID,           chid<<16);                     
	chid=3100+pci_index;
        write32(BASE1, GC314FS_R3AID,           chid<<16);                     

	write32(BASE1, GC314FS_R1ASM,		0x150);
	write32(BASE1, GC314FS_R2ASM,		0x150);
	write32(BASE1, GC314FS_R3ASM,		0x150);
}
else if(channel==CHANNEL_B){
	write16(BASE1, GC314FS_R1BCSR,					0x0004);
	write16(BASE1, GC314FS_R2BCSR,					0x0004);
	write16(BASE1, GC314FS_R3BCSR,					0x0004);

	chid=1200+pci_index;
        write32(BASE1, GC314FS_R1BID,           chid<<16);                     
	chid=2200+pci_index;
        write32(BASE1, GC314FS_R2BID,           chid<<16);                     
	chid=3200+pci_index;
        write32(BASE1, GC314FS_R3BID,           chid<<16);                     

	write32(BASE1, GC314FS_R1BSM,		0x150);
	write32(BASE1, GC314FS_R2BSM,		0x150);
	write32(BASE1, GC314FS_R3BSM,		0x150);
}
else if(channel==CHANNEL_C){
	write16(BASE1, GC314FS_R1CCSR,					0x0004);
	write16(BASE1, GC314FS_R2CCSR,					0x0004);
	write16(BASE1, GC314FS_R3CCSR,					0x0004);

	chid=1300+pci_index;
        write32(BASE1, GC314FS_R1CID,           chid<<16);                     
	chid=2300+pci_index;
        write32(BASE1, GC314FS_R2CID,           chid<<16);                     
	chid=3300+pci_index;
        write32(BASE1, GC314FS_R3CID,           chid<<16);                     

	write32(BASE1, GC314FS_R1CSM,		0x150);
	write32(BASE1, GC314FS_R2CSM,		0x150);
	write32(BASE1, GC314FS_R3CSM,		0x150);
}
else if(channel==CHANNEL_D){
	write16(BASE1, GC314FS_R1DCSR,					0x0004);
	write16(BASE1, GC314FS_R2DCSR,					0x0004);
	write16(BASE1, GC314FS_R3DCSR,					0x0004);

	chid=1400+pci_index;
        write32(BASE1, GC314FS_R1DID,           chid<<16);                     
	chid=2400+pci_index;
        write32(BASE1, GC314FS_R2DID,           chid<<16);                     
	chid=3400+pci_index;
        write32(BASE1, GC314FS_R3DID,           chid<<16);                     

	write32(BASE1, GC314FS_R1DSM,		0x150);
	write32(BASE1, GC314FS_R2DSM,		0x150);
	write32(BASE1, GC314FS_R3DSM,		0x150);
}
/*  SYNC2_ON: */
        *((uint32*)(BASE1+GC314FS_GCSR)) |=0x00020000;
	usleep(20);	
/*  SYNC2_OFF: */
        *((uint32*)(BASE1+GC314FS_GCSR)) &=0xfffdffff;
	usleep(20);	
/*
if(channel==CHANNEL_A){
	fprintf(stderr,"R3ACSR: %x\n",*((uint32*)(BASE1+GC314FS_R3ACSR)));
	fprintf(stderr,"R3ASM: %x\n",*((uint32*)(BASE1+GC314FS_R3ASM)));
	if ( (*((uint32*)(BASE1+GC314FS_R3ACSR)) & 0x10000) == 0x10000 ) {
          fprintf(stderr,"_clear_FIFO: Channel A input 3 : final FIFO: empty\n");
          fflush(stderr);
        } else {
          fprintf(stderr,"_clear_FIFO: Channel A input 3 : final  FIFO: not empty\n") ;
          fflush(stderr);
        }
}
*/
	return 1;

}
