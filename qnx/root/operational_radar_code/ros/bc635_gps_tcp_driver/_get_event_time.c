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

/*-GET_EVENT_TIME-------------------------------------------------*/
int get_event_time(int *sec, int *nsec, struct DEV_reg  *DEVreg, int locked){

	struct timespec tp;
	time_t          calandertime;
        int32   event_nsec;
        int32   event_sec;
        int32      temp,intstat;
	   // READ THE TIME
		// poll status reg an wait 10 us
              if (DEVreg!=NULL) {
                  pthread_mutex_lock(&gps_state_lock);
                  intstat=DEVreg->intstat;
                  if((intstat & 0x1)==0x1) {               
                    //temp=DEVreg->eventreq;
                    temp=DEVreg->event1;
                    event_sec=temp;
                    temp=DEVreg->event0;
		    event_nsec=(temp & 0xFFFFF)*1000;
		    event_nsec+=((temp >> 20) & 0xF)*100;
                    pthread_mutex_unlock(&gps_state_lock);                                                                              
                    calandertime=event_sec;
		    *sec=event_sec;
		    *nsec=event_nsec;
                    fprintf(stderr,"Get Event Time :: sec: %d  nsec: %d\n",event_sec,event_nsec);
                    temp=DEVreg->unlock1;  // Clear the event occurance bit 
                  } else {
                    fprintf(stderr,"Event has not occurred: 0x%x\n",intstat);
                    clock_gettime(CLOCK_REALTIME,&tp);                                              
                    *nsec=tp.tv_nsec;                                                               
                    *sec=tp.tv_sec;                                                                 
                    calandertime=tp.tv_sec;
                  }
              } else {                                                                                     
                clock_gettime(CLOCK_REALTIME,&tp);                                              
                *nsec=tp.tv_nsec;                                                               
                *sec=tp.tv_sec;                                                                 
                calandertime=tp.tv_sec;
              }                                     
		return calandertime;

}

