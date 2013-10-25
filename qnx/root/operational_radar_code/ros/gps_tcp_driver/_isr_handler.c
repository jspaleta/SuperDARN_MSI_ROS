#include <stdlib.h>
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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "include/registers.h"
//#include "include/_prog_conventions.h"
#include "include/_isr_handler.h"
#include "include/_refresh_state.h"
//#include "include/_display_structure.h"
#include "control_program.h"
#include "global_server_variables.h"
#include "utils.h"

extern unsigned char	 *BASE0, *BASE1;
extern int IRQ;
extern int tc_count;
extern int intid;
extern int configured;
extern struct 	sigevent interruptevent;
extern struct  GPSStatus displaystat;
extern struct	timespec timecompare,event;
extern int 	RateSynthInterrupt,EventInterrupt,TimeCompareInterrupt,timecompareupdate,eventupdate;
extern pthread_t int_thread_id,refresh_thread_id;
extern int verbose;

int set_time_compare(int mask,struct timespec *nexttime);
int set_time_compare_register(int mask,struct timespec *nexttime);

const struct sigevent* isr_handler(void *arg, int id){

// This is the intterupt handler for the GPS-PCI
// card.  In this software only the time compare
// interrupt and the event capture interrupts are
// enabled.  This handler monitors for these two
// interrupts.

		//check to see if interrupt is from event capture
		if( ( *((uint08*)(BASE1+0xfe)) & 0x01 ) == 0x01 ){
			//clear event interrupt
			*((uint08*)(BASE1+0xf8))|=0x01;
			//set flag for event interrupt
			EventInterrupt=1;
		} 
		//check to see if interrupt is from rate synthesizer
		//(which is GPS trigger output)
		if( ( *((uint08*)(BASE1+0xfe)) & 0x08 ) == 0x08 ){
			//clear time compare interrupt
			*((uint08*)(BASE1+0xf8))|=0x40;
			//set flag for time compare interrupt
			RateSynthInterrupt=1;
		}
		//check to see if interrupt is from time compare
		//(which is GPS trigger output)
		if( ( *((uint08*)(BASE1+0xfe)) & 0x02 ) == 0x02 ){
			//clear time compare interrupt
			*((uint08*)(BASE1+0xf8))|=0x02;
			//set flag for time compare interrupt
			TimeCompareInterrupt=1;
		}
		//if a time compare or event capture intterupt has
		//occured, then signal the thread monitoring the
		//interrupts (int_thread)
		if( TimeCompareInterrupt || RateSynthInterrupt || EventInterrupt ){
			return(&interruptevent);
		}
		//if the time compare or event capture intterupts
		//did not occure, then some other device triggered
		//the interrupts 
		else{
			return(NULL);
		}
}
void * int_thread(void *arg){
// This is the interrupt monitoring thread for the GPC-PCI
// driver.  This thread continially wait for a time compare
// or event capture interrupt, then implements the necessary
// code to respond to those interrupts

	struct	timespec start, stop, sleep, now;
	struct	tm* localtime;
	time_t 	calandertime;
	int	gpssecond, gpsnsecond, temp;
	int	lastsecond=0,lastnsecond=0;
	float	second;
	const struct sched_param threadparameters;
	pthread_t	threadid;

	//fill in 'event' structure to signal an interrupt
printf("In int thread");
#ifdef __QNX__
	memset(&interruptevent, 0, sizeof(interruptevent));
	interruptevent.sigev_notify = SIGEV_INTR;
	//give this thread I/O capcabilities so it can monitor hardware
	ThreadCtl(_NTO_TCTL_IO, NULL);
	//get thread ID
	//threadid=pthread_self(void);
	//get priority
	//temp=pthread_getschedparam(threadid,&temp,&threadparameters);
	//threadparameters.sched_priority=15;
	//attach this thread to the interrupt handler
	intid=InterruptAttach(IRQ, isr_handler, NULL, &BASE1, 4);
	while(1){
		//wait for interrupt
		InterruptWait(NULL, NULL);
		//once interrupt occurs, unmask interrupts
		InterruptUnmask(IRQ, intid);
		//if time compare interrupt, then do this...
		if(RateSynthInterrupt){
			//if oneshot, disable the rate synthesizer
			//clear the time compare interrupt flag
			temp=get_software_time(&gpssecond,&gpsnsecond,BASE1);
			displaystat.nextcomparesec=gpssecond;
			displaystat.nextcomparensec=gpsnsecond;
			RateSynthInterrupt=0;
			displaystat.timecompareupdate=1;
		}
		//if event capture interrupt, then do this...
		if(EventInterrupt){
			//clear the event capture interrupt flag
			EventInterrupt=0;
			//get the time of the event which triggered the
			//interrupt
			temp=get_event_time(&gpssecond,&gpsnsecond,BASE1);
			event.tv_sec=gpssecond;
			event.tv_nsec=gpsnsecond;
			//set a flag so the screen will be updated to 
			//display the time of the event
			eventupdate=1;
		}
		if(TimeCompareInterrupt){
			//clear the event capture interrupt flag
			TimeCompareInterrupt=0;
			displaystat.oneshot=0;
			//get the time of the event which triggered the
			//interrupt
			temp=get_compare_time(&gpssecond,&gpsnsecond,BASE1);
			//set a flag so the screen will be updated to 
			//display the time of the event
			displaystat.nextcomparesec=gpssecond;
			displaystat.nextcomparensec=gpsnsecond;
			displaystat.timecompareupdate=1;
		}
	}
#endif
}

