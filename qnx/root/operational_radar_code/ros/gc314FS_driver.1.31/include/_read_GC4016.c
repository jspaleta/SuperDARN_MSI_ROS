#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "_prog_conventions.h"
#include "_regs_GC314FS.h"
#include "_regs_GC4016.h"

int _read_GC4016(char *BASE1){
	
	FILE	*fid;
	int 	temp, i;

	fid=fopen("GC4016_settings.out", "w");
	for (i=0;i<=0x07c;i+=0x04){
		fprintf(fid, "%000.3x	%00000000.8x", i, *((uint32*)(BASE1+0x800+i)));
		fprintf(fid, "	%00000000.8x", *((uint32*)(BASE1+0x1000+i)));
		fprintf(fid, "	%00000000.8x\n", *((uint32*)(BASE1+0x1800+i)));
	}
	fprintf(fid, "\n\n\n");
	i=0;
	for (i=0x200;i<=0x45c;i+=0x04){
		fprintf(fid, "%000.3x	%00000000.8x", i, *((uint32*)(BASE1+0x800+i)));
		fprintf(fid, "	%00000000.8x", *((uint32*)(BASE1+0x1000+i)));
		fprintf(fid, "	%00000000.8x\n", *((uint32*)(BASE1+0x1800+i)));
	}
	fclose(fid);
}
