#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "_prog_conventions.h"
#include "_regs_GC314FS.h"
#include "_regs_GC4016.h"
#include "_global.h"
#include "../../include/gc314FS.h"

//int _set_RDA(char *BASE1, int print, int samples_to_collect, int fifo_lvl, unsigned long *GC4016, int *resampcoeffs, unsigned int *resampratio){
int _set_RDA(char *BASE1, int print, unsigned int samples_to_collect, int skip_samples, int fifo_lvl, int channel){

	struct		timespec start, stop;

	unsigned int	*to_write;	
    /* PROGRAM GC4016-1 *****************************************************************/
	clock_gettime(CLOCK_REALTIME, &start);
	//printf("channel %d skip %d samples\n", channel, skip_samples);
        //skip_samples=0;
if(channel==CHANNEL_A){
	write16(BASE1, GC314FS_R1ACSR,					0x0002);
	write16(BASE1, GC314FS_R2ACSR,					0x0002);
	write16(BASE1, GC314FS_R3ACSR,					0x0002);

	write32(BASE1, GC314FS_R1ALVL,					fifo_lvl);
	write32(BASE1, GC314FS_R2ALVL,					fifo_lvl);
	write32(BASE1, GC314FS_R3ALVL,					fifo_lvl);

	write32(BASE1, GC314FS_R1ADWS,					samples_to_collect);
	write32(BASE1, GC314FS_R2ADWS,					samples_to_collect);
	write32(BASE1, GC314FS_R3ADWS,					samples_to_collect);

	write32(BASE1, GC314FS_R1ASKIP,					skip_samples);
	write32(BASE1, GC314FS_R2ASKIP,					skip_samples);
	write32(BASE1, GC314FS_R3ASKIP,					skip_samples);

	write32(BASE1, GC314FS_R1AFS,					samples_to_collect);
	write32(BASE1, GC314FS_R2AFS,					samples_to_collect);
	write32(BASE1, GC314FS_R3AFS,					samples_to_collect);

	//write32(BASE1, GC314FS_R1AID,		1100<<16);
	//write32(BASE1, GC314FS_R2AID,		2100<<16);
	//write32(BASE1, GC314FS_R3AID,		3100<<16);

	//write32(BASE1, GC314FS_R1ASM,		0x2a8);
	//write32(BASE1, GC314FS_R2ASM,		0x2a8);
	//write32(BASE1, GC314FS_R3ASM,		0x2a8);
	write32(BASE1, GC314FS_R1ASM,		0x8);
	write32(BASE1, GC314FS_R2ASM,		0x8);
	write32(BASE1, GC314FS_R3ASM,		0x8);
}
else if(channel==CHANNEL_B){
	write16(BASE1, GC314FS_R1BCSR,					0x0002);
	write16(BASE1, GC314FS_R2BCSR,					0x0002);
	write16(BASE1, GC314FS_R3BCSR,					0x0002);

	write32(BASE1, GC314FS_R1BLVL,					fifo_lvl);
	write32(BASE1, GC314FS_R2BLVL,					fifo_lvl);
	write32(BASE1, GC314FS_R3BLVL,					fifo_lvl);

	write32(BASE1, GC314FS_R1BDWS,					samples_to_collect);
	write32(BASE1, GC314FS_R2BDWS,					samples_to_collect);
	write32(BASE1, GC314FS_R3BDWS,					samples_to_collect);

	write32(BASE1, GC314FS_R1BSKIP,					skip_samples);
	write32(BASE1, GC314FS_R2BSKIP,					skip_samples);
	write32(BASE1, GC314FS_R3BSKIP,					skip_samples);

	write32(BASE1, GC314FS_R1BFS,					samples_to_collect);
	write32(BASE1, GC314FS_R2BFS,					samples_to_collect);
	write32(BASE1, GC314FS_R3BFS,					samples_to_collect);

	//write32(BASE1, GC314FS_R1BID,		1200<<16);
	//write32(BASE1, GC314FS_R2BID,		2200<<16);
	//write32(BASE1, GC314FS_R3BID,		3200<<16);

	write32(BASE1, GC314FS_R1BSM,		0x8);
	write32(BASE1, GC314FS_R2BSM,		0x8);
	write32(BASE1, GC314FS_R3BSM,		0x8);
}
else if(channel==CHANNEL_C){
	write16(BASE1, GC314FS_R1CCSR,					0x0002);
	write16(BASE1, GC314FS_R2CCSR,					0x0002);
	write16(BASE1, GC314FS_R3CCSR,					0x0002);

	write32(BASE1, GC314FS_R1CLVL,					fifo_lvl);
	write32(BASE1, GC314FS_R2CLVL,					fifo_lvl);
	write32(BASE1, GC314FS_R3CLVL,					fifo_lvl);

	write32(BASE1, GC314FS_R1CDWS,					samples_to_collect);
	write32(BASE1, GC314FS_R2CDWS,					samples_to_collect);
	write32(BASE1, GC314FS_R3CDWS,					samples_to_collect);

	write32(BASE1, GC314FS_R1CSKIP,					skip_samples);
	write32(BASE1, GC314FS_R2CSKIP,					skip_samples);
	write32(BASE1, GC314FS_R3CSKIP,					skip_samples);

	write32(BASE1, GC314FS_R1CFS,					samples_to_collect);
	write32(BASE1, GC314FS_R2CFS,					samples_to_collect);
	write32(BASE1, GC314FS_R3CFS,					samples_to_collect);

	//write32(BASE1, GC314FS_R1CID,		1300<<16);
	//write32(BASE1, GC314FS_R2CID,		2300<<16);
	//write32(BASE1, GC314FS_R3CID,		3300<<16);

	write32(BASE1, GC314FS_R1CSM,		0x8);
	write32(BASE1, GC314FS_R2CSM,		0x8);
	write32(BASE1, GC314FS_R3CSM,		0x8);
}
else if(channel==CHANNEL_D){
	write16(BASE1, GC314FS_R1DCSR,					0x0002);
	write16(BASE1, GC314FS_R2DCSR,					0x0002);
	write16(BASE1, GC314FS_R3DCSR,					0x0002);

	write32(BASE1, GC314FS_R1DLVL,					fifo_lvl);
	write32(BASE1, GC314FS_R2DLVL,					fifo_lvl);
	write32(BASE1, GC314FS_R3DLVL,					fifo_lvl);

	write32(BASE1, GC314FS_R1DDWS,					samples_to_collect);
	write32(BASE1, GC314FS_R2DDWS,					samples_to_collect);
	write32(BASE1, GC314FS_R3DDWS,					samples_to_collect);

	write32(BASE1, GC314FS_R1DSKIP,					skip_samples);
	write32(BASE1, GC314FS_R2DSKIP,					skip_samples);
	write32(BASE1, GC314FS_R3DSKIP,					skip_samples);

	write32(BASE1, GC314FS_R1DFS,					samples_to_collect);
	write32(BASE1, GC314FS_R2DFS,					samples_to_collect);
	write32(BASE1, GC314FS_R3DFS,					samples_to_collect);

	//write32(BASE1, GC314FS_R1DID,		1400<<16);
	//write32(BASE1, GC314FS_R2DID,		2400<<16);
	//write32(BASE1, GC314FS_R3DID,		3400<<16);

	write32(BASE1, GC314FS_R1DSM,		0x8);
	write32(BASE1, GC314FS_R2DSM,		0x8);
	write32(BASE1, GC314FS_R3DSM,		0x8);
}
	return 1;

}
