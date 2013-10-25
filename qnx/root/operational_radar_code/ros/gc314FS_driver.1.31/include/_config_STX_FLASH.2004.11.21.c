#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "_prog_conventions.h"
#include "_regs_GC314FS.h"

int _config_STX_FLASH(char *BASE1, int write_FPGA, int print){

	int temp=0;

    /* CHECK FOR FPGA CONFIGURATION SUCCESS */
	if ( (*((uint32*)(BASE1+GC314FS_SF_CSR)) & 0x00000001)==0){
		fprintf(stderr, "FPGA not properly configured");
		return -1;
	}
    /* RELEASE FPGA FROM TRI-STATE */
	*((uint32*)(BASE1+GC314FS_SF_CSR))=(*((uint32*)(BASE1+GC314FS_SF_CSR)) | 0x00000010);
	*((uint32*)(BASE1+GC314FS_SF_CSR))=(*((uint32*)(BASE1+GC314FS_SF_CSR)) | 0x00000020);
	if ( (*((uint32*)(BASE1+GC314FS_SF_CSR)) & 0x00000001)==0){
		fprintf(stderr, "FPGA not properly configured");
		temp=1;
		return -1;
	}
    /* PRINT STATUS OF FPGA AND STX_FLASH */
	if (print==1){
		printf("	STX_FLASH PARAMETERS:\n");
		if (temp==0){
			printf("	  FPGA configured properly\n");
		}
		printf("	  GC314FS_SF_CSR=	0x%x\n", *((uint32*)(BASE1+GC314FS_SF_CSR)));
		printf("	  FLASH VERSION=	0x%x\n", *((uint32*)(BASE1+GC314FS_SF_VER)));
	}
	return 1;
}
