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

void gc314SetSyncMask(int filedes, unsigned int state){
	
	int retval;
	int status=0;
	struct S_sync_mask_data   to_send;
	
#ifdef __QNX__

	to_send.state=state;

	retval = devctl(filedes, GC314_SET_SYNC_MASK, &to_send, sizeof(to_send), &status);
	if(retval != EOK){
		fprintf(stderr,"Error in gc314fs device descriptor %d: %s\n", filedes, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(status != GC314_SET_SYNC_MASK){
		fprintf(stderr,"!!!!Error setting global reset on device descriptor %d.  Error %d\n", filedes, status);
		if( status == GC314_WRONG_CHANNEL ) fprintf(stderr, "   WRONG CHANNEL\n");
		exit(EXIT_FAILURE);
	}
#endif
}
