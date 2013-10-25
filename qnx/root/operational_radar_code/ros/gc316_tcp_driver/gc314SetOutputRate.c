#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef __QNX__
  #include <devctl.h>
#endif
#include "gc314FS.h"

void gc314SetOutputRate(int filedes, double rate, int channel){
	
	int retval;
	int status=0;
	struct S_output_rate_data   to_send;
	
#ifdef __QNX__

	to_send.channel=channel;
	to_send.output_rate=rate;

	//printf("gc314SetOutputRate channel=%d\n", channel);
	retval = devctl(filedes, GC314_SET_OUTPUT_RATE, &to_send, sizeof(to_send), &status);
	if(retval != EOK){
		fprintf(stderr,"Error in gc314fs device descriptor %d: %s\n", filedes, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(status != GC314_SET_OUTPUT_RATE){
		fprintf(stderr,"!!!!Error setting channel A output rate on device descriptor %d.  Error %d\n", filedes, status);
		if( status == GC314_WRONG_CHANNEL ) fprintf(stderr, "   WRONG CHANNEL\n");
		exit(EXIT_FAILURE);
	}
#endif
}
