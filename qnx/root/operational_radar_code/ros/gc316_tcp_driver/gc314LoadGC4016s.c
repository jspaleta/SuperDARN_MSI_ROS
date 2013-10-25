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

void gc314LoadGC4016s(int filedes){
	
	int retval;
	int status=0;
	int temp=0;
#ifdef __QNX__

	retval = devctl(filedes, GC314_LOAD_GC4016S, &temp, sizeof(temp), &status);
	if(retval != EOK){
		fprintf(stderr,"Error in gc314fs device descriptor %d: %s\n", filedes, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(status != GC314_LOAD_GC4016S){
		fprintf(stderr,"!!!!Error setting global reset on device descriptor %d.  Error %d\n", filedes, status);
		if( status == GC314_WRONG_CHANNEL ) fprintf(stderr, "   WRONG CHANNEL\n");
		exit(EXIT_FAILURE);
	}
#endif
}
