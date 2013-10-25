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

void gc314SetSync2(int filedes, int state){
	
	int retval;
	int status=0;
	struct S_sync1_data   to_send;
	
#ifdef __QNX__
#ifdef POOP
	to_send.state=state;

	retval = devctl(filedes, GC314_SET_SYNC2, &to_send, sizeof(to_send), &status);
	if(retval != EOK){
		fprintf(stderr,"Error in gc314fs device descriptor %d: %s\n", filedes, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(status != GC314_SET_SYNC2){
		fprintf(stderr,"!!!!Error setting global reset on device descriptor %d.  Error %d\n", filedes, status);
		if( status == GC314_WRONG_CHANNEL ) fprintf(stderr, "   WRONG CHANNEL\n");
		exit(EXIT_FAILURE);
	}
#endif
#endif
}
