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

int gc314WaitForData(int filedes, int channel){
	
	int retval;
	int status=0;
        int chan=0;
        chan=channel;
#ifdef __QNX__
	
	retval = devctl(filedes, GC314_WAIT_FOR_DATA, &chan, sizeof(channel), &status);
	if(retval != EOK){
		fprintf(stderr,"Error in gc314fs device descriptor %d: %s\n", filedes, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(status != GC314_WAIT_FOR_DATA){
		fprintf(stderr,"!!!!Error updating channel on device descriptor %d.  Error %d\n", filedes, status);
		if( status == GC314_WRONG_CHANNEL ) fprintf(stderr, "   WRONG CHANNEL\n");
		exit(EXIT_FAILURE);
	}
#endif
        return chan;
}
