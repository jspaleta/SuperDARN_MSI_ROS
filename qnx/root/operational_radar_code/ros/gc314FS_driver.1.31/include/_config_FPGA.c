#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "_prog_conventions.h"
#include "_regs_GC314FS.h"
#define image0_address	0x00000000
#define image1_address	0x00200000
#define image_length	0x00200000

int _config_FPGA(char *BASE1, int write_FPGA, int flash_image, int print){

	int		temp=0;
	unsigned int	address;
	struct		timespec sleep;

    /* VERIFY IMAGE AND SET ADDRESS */
	if (flash_image==0){
		address=image0_address;
	}
	else if (flash_image==1){
		address=image1_address;
	}
	else{
		fprintf(stderr,"INVALID IMAGE NUMBER\n");
		return -1;
	}
    /* WRITE FPGA WITH FLASH IMAGE */
	if (write_FPGA==1){
		if (print==1){
			printf("RECONFIGURING FPGA WITH IMAGE %d\n", flash_image);
		}
		*((uint32*)(BASE1+GC314FS_SF_FLA))=(0x80000000 | address);
		sleep.tv_sec=1;
		sleep.tv_nsec=0;
		clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
	}

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
