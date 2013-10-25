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

void gc314SetSamples(int filedes, int samples, int channel){
	
	int retval;
	int status=0;
	struct S_samples_data   to_send;
	
#ifdef __QNX__

	to_send.channel=channel;
	to_send.samples=samples;

	retval = devctl(filedes, GC314_SET_SAMPLES, &to_send, sizeof(to_send), &status);
	if(retval != EOK){
		fprintf(stderr,"Error in gc314fs device descriptor %d: %s\n", filedes, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(status != GC314_SET_SAMPLES){
		fprintf(stderr,"!!!!Error setting channel A frequency on device descriptor %d.  Error %d\n", filedes, status);
		if( status == GC314_WRONG_CHANNEL ) fprintf(stderr, "   WRONG CHANNEL\n");
		exit(EXIT_FAILURE);
	}
#endif
}
