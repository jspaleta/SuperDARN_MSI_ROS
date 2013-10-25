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
extern int verbose;
/*-GET_EVENT_TIME-------------------------------------------------*/
int get_event_time(int *sec, int *nsec, int BASE1){

	struct		 timespec sleep,tp;
	struct		 tm localtime;
	time_t 		 calandertime;
	int		 temp,temp174,temp178,temp17c;
	int		 year,month,day,hour,minute,second,nsecond;

	   // READ THE TIME
		//Check event status flag
              if (BASE1!=NULL) {
		if ((*((uint32_t*)(BASE1+0xfe))& 0x01) != 0x01) {
                  return -1;
                }
                if (verbose > 1 ) printf("Event status flag enabled\n");
		//read time of last event 
		temp174=*((uint32_t*)(BASE1+0x174));
		temp178=*((uint32_t*)(BASE1+0x178));
		temp17c=*((uint32_t*)(BASE1+0x17c));
		//clear event status flag so new event can be captured
		*((uint32_t*)(BASE1+0xf8))|=0x01;
                if (verbose > 1) printf("Clear event status flag\n");
		temp=temp17c;
		year= 1000*((temp & 0xf000) >> 12) + 100*((temp & 0xf00) >> 8) + 10*((temp & 0xf0) >> 4) + 1*((temp & 0xf));
		localtime.tm_year=year-1900;
		//day
		temp=temp178;
		day= 100*((temp & 0xf000000) >> 24) + 10*((temp & 0xf00000) >> 20) + 1*((temp & 0xf0000)>>16);
		if( (year%4) == 0){
			//leap year
			if( (day>=0) && (day <=31) ){
				//jan
				month=0;
				day-=0;
			}
			else if ( (day > 31) && (day <= 60) ){
				//feb
				month=1;
				day-=31;
			}
			else if ( (day > 60) && (day <= 91)){
				//march
				month=2;
				day-=60;
			}
			else if ( (day > 91) && (day <= 121)){
				//apr
				month=3;
				day-=91;
			}
			else if ( (day > 121) && (day <= 152)){
				//may
				month=4;
				day-=121;
			}
			else if ( (day > 152) && (day <= 182)){
				//jun
				month=5;
				day-=152;
			}
			else if ( (day > 182) && (day <= 213)){
				//jul
				month=6;
				day-=182;
			}
			else if ( (day > 213) && (day <= 244)){
				//aug
				month=7;
				day-=213;
			}
			else if ( (day > 244) && (day <= 274)){
				//sept
				month=8;
				day-=244;
			}
			else if ( (day > 274) && (day <= 305)){
				//oct
				month=9;
				day-=274;
			}
			else if ( (day > 305) && (day <= 335)){
				//nov
				month=10;
				day-=305;
			}
			else if ( (day > 335) && (day <= 366)){
				//dec
				month=11;
				day-=335;
			}
		}
		else{
			if( (day>=0) && (day <=31) ){
				//jan
				month=0;
				day-=0;
			}
			else if ( (day > 31) && (day <= 59)){
				//feb
				month=1;
				day-=31;
			}
			else if ( (day > 59) && (day <= 90)){
				//march
				month=2;
				day-=59;
			}
			else if ( (day > 90) & (day <= 120)){
				//apr
				month=3;
				day-=90;
			}
			else if ( (day > 120) && (day <= 151)){
				//may
				month=4;
				day-=120;
			}
			else if ( (day > 151) && (day <= 181)){
				//jun
				month=5;
				day-=151;
			}
			else if ( (day > 181) && (day <= 212)){
				//jul
				month=6;
				day-=181;
			}
			else if ( (day > 212) && (day <= 243)){
				//aug
				month=7;
				day-=212;
			}
			else if ( (day > 243) && (day <= 273)){
				//sept
				month=8;
				day-=243;
			}
			else if ( (day > 273) && (day <= 304)){
				//oct
				month=9;
				day-=273;
			}
			else if ( (day > 304) && (day <= 334)){
				//nov
				month=10;
				day-=304;
			}
			else if ( (day > 334) && (day <= 365)){
				//dec
				month=11;
				day-=334;
			}
		}
		localtime.tm_mday=day;
		localtime.tm_mon=month;
		localtime.tm_isdst=0;
		localtime.tm_gmtoff=0;

		//hour
		temp=temp178;
		hour= 10*((temp & 0xf000) >> 12) + 1*((temp & 0xf00) >> 8);
		localtime.tm_hour=hour;

		//minute
		minute= 10*((temp & 0xf0) >> 4) + 1*((temp & 0xf));
		localtime.tm_min=minute;
		
		//second
		temp=temp174;
		second= 10*((temp & 0xf0000000) >> 28) + 1*((temp & 0xf000000) >> 24);
		localtime.tm_sec=second;

		//nano second
		nsecond= 100*((temp17c & 0xf00000) >> 20) + 1000000*((temp & 0xf000) >> 12) + 100000*((temp & 0xf00) >> 8) + 10000*((temp & 0xf0) >> 4) + 1000*((temp & 0xf));
		nsecond += ( 100000000*((temp & 0xf00000) >> 20) + 10000000*((temp & 0xf0000) >> 16) );

		calandertime=mktime(&localtime);
		*sec=calandertime;
		*nsec=nsecond;
              } else {
                clock_gettime(CLOCK_REALTIME,&tp);
                *nsec=tp.tv_nsec;
                *sec=tp.tv_sec;
                calandertime=tp.tv_sec;
 
              }
 	      return calandertime;
}

