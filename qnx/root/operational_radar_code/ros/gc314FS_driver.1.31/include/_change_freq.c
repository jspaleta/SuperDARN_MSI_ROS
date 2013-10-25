#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "_prog_conventions.h"
#include "_regs_GC314FS.h"
#include "_regs_GC4016.h"
#include "_global.h"


int _change_freq(char *BASE, int channel, int freq){

	int FREQ, i;
	unsigned int	GC314FS_GCoffset[3]={0x800,0x1000,0x1800};
	unsigned int 	chan_control_map[4]={0x00,0x80,0x100,0x180};

	FREQ=4294967296*freq/FCLOCK;	

	// write the frequency registers	
	for (i=0;i<3;i++){
		write16(BASE, GC314FS_GCoffset[i]+chan_control_map[channel]+GC4016_FREQ_LSB,	(short)(FREQ & 0x0000ffff));
		write16(BASE, GC314FS_GCoffset[i]+chan_control_map[channel]+GC4016_FREQ_MSB,	(short)( (FREQ>>16) & 0x0000ffff));
	}
	//restart data collection
	// set sync2 data aquisition one time sync
	write32(BASE, GC314FS_IWBSM,		0x152);
	write32(BASE, GC314FS_R1ASM,		0x152);
	write32(BASE, GC314FS_R1BSM,		0x152);
	write32(BASE, GC314FS_R1CSM,		0x152);
	write32(BASE, GC314FS_R1DSM,		0x152);
	write32(BASE, GC314FS_R2ASM,		0x152);
	write32(BASE, GC314FS_R2BSM,		0x152);
	write32(BASE, GC314FS_R2CSM,		0x152);
	write32(BASE, GC314FS_R2DSM,		0x152);
	write32(BASE, GC314FS_R3ASM,		0x152);
	write32(BASE, GC314FS_R3BSM,		0x152);
	write32(BASE, GC314FS_R3CSM,		0x152);
	write32(BASE, GC314FS_R3DSM,		0x152);
	// set global sync mask to clear RTS
	write32(BASE, GC314FS_GSM,		0x00010000);
	// set Frame more to include CHID and frame counter
	*((uint32*)(BASE+GC314FS_GCSR))				|=0x00000040;

	// trigger sync to to enable RDA system
	*((uint32*)(BASE+GC314FS_GCSR))				|=0x00040000;

	return 1;	
}
