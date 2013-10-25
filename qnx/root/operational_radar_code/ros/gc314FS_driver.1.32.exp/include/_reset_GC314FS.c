#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "_prog_conventions.h"
#include "_regs_GC314FS.h"

int _reset_GC314FS(char *BASE1){

	int temp;


//	*((uint32*)(BASE1))			= 0x00006041;
//	*((uint32*)(BASE1))			= 0x00000041;
//	*((uint32*)(BASE1+0x104))		= 0x80000000;
//	*((uint32*)(BASE1+0x104))		= 0x00000000;
//	*((uint32*)(BASE1+0x108))		= 0x00000000;
//	*((uint32*)(BASE1+0x100))		= 0x01000814;
//	*((uint32*)(BASE1+0x170))		= 0x00000006;
//	*((uint32*)(BASE1+0x100))		= 0x01000814;


	*((uint32*)(BASE1+GC314FS_PIMR))	 = 0x00000000;
	*((uint32*)(BASE1+GC314FS_SIMR))	 = 0x00000000;
//	temp=*((uint32*)(BASE1+GC314FS_PISR));
//	temp=*((uint32*)(BASE1+GC314FS_SISR));


//	*((uint32*)(BASE1+GC314FS_PIMR))		= 0x80000000;  // bit 31 reserved

/* GCSR
 * x0010 : Little Endian DMA 
 * x0f00 : Disable Chan 1-3 A/D IF Outputs
 */    
	*((uint32*)(BASE1+GC314FS_GCSR))		= 0x0f10;

	*((uint32*)(BASE1+GC314FS_GCSR))	|= 0x00000080; //FPGA Reset

	*((uint32*)(BASE1+GC314FS_CPLL))		= 0x02; // Allow PLL to lock


	return 1;
}
