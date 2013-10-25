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

void gc314SetFilters(int filedes, int bandwidth, int channel, int match){
	
	int retval;
	int status=0;
	struct S_filter_data   to_send;
	
#ifdef __QNX__

	to_send.channel=channel;
	to_send.bandwidth=bandwidth;
	to_send.matched=match;

	retval = devctl(filedes, GC314_SET_FILTERS, &to_send, sizeof(to_send), &status);
	if(retval != EOK){
		fprintf(stderr,"Error in gc314fs device descriptor %d: %s\n", filedes, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(status != GC314_SET_FILTERS){
		fprintf(stderr,"!!!!Error setting channel A filters on device descriptor %d.  Error %d\n", filedes, status);
		if( status == GC314_WRONG_CHANNEL ) fprintf(stderr, "   WRONG CHANNEL\n");
		exit(EXIT_FAILURE);
	}
#endif
}
