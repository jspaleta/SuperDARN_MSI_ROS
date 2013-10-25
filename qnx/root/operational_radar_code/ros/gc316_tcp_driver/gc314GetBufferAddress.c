#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <sys/mman.h>
#ifdef __QNX__
  #include <devctl.h>
#endif
#include "gc314FS.h"

unsigned long gc314GetBufferAddress(unsigned int *physical_addr,int filedes, int chip, int channel){
	
	int retval;
	int status=0;
	struct S_buffer_address to_send;
	unsigned long	address;

	if( (chip<0) || (chip>2) ){
		fprintf(stderr, "Invalid chip number selected in gc314GetBufferAddress\n");
	}
	to_send.channel=channel;
	to_send.chip=chip;
	to_send.address=0;
#ifdef __QNX__
	
	retval = devctl(filedes, GC314_GET_BUFFER_ADDRESS, &to_send, sizeof(to_send), &status);
        *physical_addr=to_send.address;
	address = mmap( 0, DMA_BUF_SIZE, PROT_READ|PROT_NOCACHE, MAP_PHYS, NOFD, to_send.address);
        printf("gc314GetBufferAddress:: fd: %d chip: %d channel: %d phys_addr: %x virtual_addr: %x\n",filedes,chip,channel,*physical_addr,address);
	if(address == MAP_FAILED){
		fprintf(stderr, "*************************** mmap failed ********************** \n");
	}
	if(retval != EOK){
		fprintf(stderr,"Error in gc314fs device descriptor %d: %s\n", filedes, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(status != GC314_GET_BUFFER_ADDRESS){
		fprintf(stderr,"!!!!Error getting buffer addresses on device descriptor %d.  Error %d\n", filedes, status);
		if( status == GC314_WRONG_CHANNEL ) fprintf(stderr, "   WRONG CHANNEL\n");
		exit(EXIT_FAILURE);
	}
	return address;
#else
        return NULL;
#endif

}
