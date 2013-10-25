#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#ifdef __QNX__
  #include <hw/pci.h>
  #include <hw/inout.h>
  #include <sys/neutrino.h>
  #include <sys/iofunc.h>
  #include <sys/dispatch.h>
#endif
#ifdef __linux__
  #include <linux/rtc.h>
  #include <sys/ioctl.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "include/registers.h"
#include "include/_refresh_state.h"
#include "control_program.h"
#include "global_server_variables.h"
#include "utils.h"
#include "include/bc635_registers.h"
#define MAX_SEC_ERR 0.05

extern struct DEV_reg	 *DEVreg; 
extern int IRQ;
extern int tc_count;
extern int intid;
extern int configured;
extern int locked;
extern struct 	sigevent interruptevent;
extern struct  GPSStatus displaystat;
extern struct	timespec timecompare,event;
extern int 	RateSynthInterrupt,EventInterrupt,TimeCompareInterrupt,timecompareupdate,eventupdate;
extern pthread_t int_thread_id,refresh_thread_id;
extern int verbose;
extern pthread_mutex_t gps_state_lock;

int set_time_compare(int mask,struct timespec *nexttime);
int set_time_compare_register(int mask,struct timespec *nexttime);

/*-GET_HDW_STAT------------------------------------------------------*/
int get_hdw_stat(){
	int temp=0;
	return temp;

}

/*-REFRESH_---------------------------------------------------*/	
void * refresh_state(void *arg){
  int temp;
  int refreshrate=1;
  while(1){	
    pthread_mutex_lock(&gps_state_lock);
    get_state();
    pthread_mutex_unlock(&gps_state_lock);
    if(locked) refreshrate=10;
    else refreshrate=1;
    sleep(refreshrate);
  }   
}

void * get_state(){
  int temp, int_state, lock_status;
  int gps_sec, gps_nsec;
  int event_sec, event_nsec;
  struct timespec now;
  float timediff;

  if(verbose > 1) {  
	fprintf(stderr,"Get state\n");
        int_state=DEVreg->intstat;
	fprintf(stderr,"  DEVreg INTSTAT: 0x%x : ", int_state);
        printbits(stderr,int_state);
        fprintf(stderr,"\n");
        
        fprintf(stderr,"  Request Time\n");
  }
        temp=DEVreg->timereq;
        lock_status=(DEVreg->time0 & 0x7000000) >> 24;
        temp=DEVreg->time1;
        gps_sec=temp;
        temp=DEVreg->time0;
        gps_nsec=(temp & 0xFFFFF)*1000;
        gps_nsec+=((temp >> 20) & 0xF)*100;
        displaystat.gpssecond=gps_sec;
        displaystat.gpsnsecond=gps_nsec;

  //get_event_time(&gps_sec,&gps_nsec,DEVreg,locked);
  if(verbose > 1) {  
        fprintf(stderr,"  Software Major Time: ");
        fprintf(stderr,"%s",ctime(&gps_sec));
        fprintf(stderr,"  Software Nanosecs: %09d\n", gps_nsec);
        if((int_state & 0x01) == 0x01) {
          temp=DEVreg->event1;
          event_sec=temp;
          temp=DEVreg->event0;
          event_nsec=(temp & 0xFFFFF)*1000;
          event_nsec+=((temp >> 20) & 0xF)*100;
          fprintf(stderr,"  Event Major Time: ");
          fprintf(stderr,"%s",ctime(&event_sec));
          fprintf(stderr,"  Event Nanosecs: %09d\n", event_nsec);
          //temp=DEVreg->unlock1;
        } else {
          fprintf(stderr,"  :::::No Event::::::\n");
        }
  }
  fprintf(stderr,"  Status: 0x%x : ",lock_status);

        if (lock_status==0) {
          if(verbose > -1) fprintf(stderr,"Good : ");
          displaystat.gps_lock=1;
          temp=clock_gettime(CLOCK_REALTIME,&now);
          displaystat.syssecond=now.tv_sec;
          displaystat.sysnsecond=now.tv_nsec;
                        //caclulate time difference
          timediff=(float)(displaystat.syssecond-displaystat.gpssecond)+((float)(displaystat.sysnsecond-displaystat.gpsnsecond))/1e9;
          if(fabs(timediff)>MAX_SEC_ERR ) {
            if( verbose > -1 ) fprintf(stderr,"Preparing to Update System Time: %lf\n",timediff);
            now.tv_sec=displaystat.gpssecond;
            now.tv_nsec=displaystat.gpsnsecond;

            clock_settime(CLOCK_REALTIME,&now);
            #ifdef __QNX__              
                  system("rtc -r 10 -s hw");
            #endif
            #ifdef __linux__
//                  printf("Setting hdw clock on linux\n");
//                  int fd = open("/dev/rtc", O_RDONLY);
//                  struct rtc_time  *rt=NULL;
//                  rt=(struct rtc_time*)gmtime(&now.tv_sec);              
//                  ioctl(fd, RTC_SET_TIME, rt);
//                  close(fd);
            #endif

          } else {
            if(verbose > -1) fprintf(stderr," System Time within allowed margin\n");
          }
        } else {
          if(verbose > -1) {  
            fprintf(stderr,"Bad: ");
            if ((lock_status & 1)==1) fprintf(stderr," Track ");
            if ((lock_status & 2)==2) fprintf(stderr," Phase ");
            if ((lock_status & 4)==4) fprintf(stderr," Freq ");
            fprintf(stderr,"\n");
	  }	
          displaystat.gps_lock=0;
        }
        locked|=displaystat.gps_lock;
        pthread_mutex_unlock(&gps_state_lock);
}
