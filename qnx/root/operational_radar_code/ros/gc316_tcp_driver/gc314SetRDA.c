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

void gc314SetRDA(int filedes, int channel){
	
	int retval;
	int status=0;
	struct S_RDA_data RDA_data;
#ifdef __QNX__

	RDA_data.channel=channel;
	retval = devctl(filedes, GC314_SET_RDA, &RDA_data, sizeof(RDA_data), &status);
	if(retval != EOK){
		fprintf(stderr,"Error in gc314fs device descriptor %d: %s\n", filedes, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(status != GC314_SET_RDA){
		fprintf(stderr,"!!!!Error setting global reset on device descriptor %d.  Error %d\n", filedes, status);
		if( status == GC314_WRONG_CHANNEL ) fprintf(stderr, "   WRONG CHANNEL\n");
		exit(EXIT_FAILURE);
	}
#endif
}
