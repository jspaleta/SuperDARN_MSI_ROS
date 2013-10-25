#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "rtypes.h"
#include "_prog_conventions.h"
#include "bc635_registers.h"

extern pthread_mutex_t gps_state_lock;

/*-GET_SOFTWARE_TIME-------------------------------------------------*/
int get_software_time(int *sec, int *nsec, struct DEV_reg  *DEVreg, int locked){

	struct timespec tp;
	time_t          calandertime;
        int32   gps_nsec;
        int32   gps_sec;
        int32      temp;
	   // READ THE TIME
		// poll status reg an wait 10 us
              if ((DEVreg!=NULL) && locked) {
                  pthread_mutex_lock(&gps_state_lock);
                  temp=DEVreg->timereq;
                  temp=DEVreg->time1;
                  gps_sec=temp;
                  temp=DEVreg->time0;
		  gps_nsec=(temp & 0xFFFFF)*1000;
		  gps_nsec+=((temp >> 20) & 0xF)*100;
                  pthread_mutex_unlock(&gps_state_lock);                                                                              
                calandertime=gps_sec;
		*sec=gps_sec;
		*nsec=gps_nsec;
              } else {                                                                                     
                clock_gettime(CLOCK_REALTIME,&tp);                                              
                *nsec=tp.tv_nsec;                                                               
                *sec=tp.tv_sec;                                                                 
                calandertime=tp.tv_sec;
              }                                     
		return calandertime;

}

