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
#include <ncurses.h>
#include <unistd.h>
#include "rtypes.h"
#include "_prog_conventions.h"

/*-GET_COMPARE_TIME-------------------------------------------------*/
int get_compare_time(int *sec, int *nsec, int BASE1){

	struct		 timespec systime;
	struct		 tm localtime,*syslocaltime;
	time_t 		 calandertime;
	int		 temp,temp138,temp13c;
	int		 year,month,day,hour,minute,second,nsecond,yday;
	FILE		 *fp;

	   // READ THE TIME
		//get year from system time
	      temp=clock_gettime(CLOCK_REALTIME,&systime);
              if(BASE1!=NULL) {
		//read time of last event 
		temp138=*((uint32_t*)(BASE1+0x138));
		temp13c=*((uint32_t*)(BASE1+0x13c));
		syslocaltime=gmtime(&systime.tv_sec);
		localtime.tm_year=syslocaltime->tm_year;
		year=localtime.tm_year;
		//day
		temp=temp13c;
		day= 100*((temp & 0xf000000) >> 24) + 10*((temp & 0xf00000) >> 20) + 1*((temp & 0xf0000)>>16) ;
		yday=day;
		//day+=1;
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
		temp=temp13c;
		hour= 10*((temp & 0xf000) >> 12) + 1*((temp & 0xf00) >> 8);
		localtime.tm_hour=hour;

		//minute
		minute= 10*((temp & 0xf0) >> 4) + 1*((temp & 0xf));
		localtime.tm_min=minute;
		
		//second
		temp=temp138;
		second= 10*((temp & 0xf0000000) >> 28) + 1*((temp & 0xf000000) >> 24);
		localtime.tm_sec=second;

		//nano second
		nsecond= 1000000*((temp & 0xf000) >> 12) + 100000*((temp & 0xf00) >> 8) + 10000*((temp & 0xf0) >> 4) + 1000*((temp & 0xf));
		nsecond += ( 100000000*((temp & 0xf00000) >> 20) + 10000000*((temp & 0xf0000) >> 16) );
	
		calandertime=mktime(&localtime);
		*sec=calandertime;
		*nsec=nsecond;
              } else {
                *sec=systime.tv_sec;
                *nsec=systime.tv_nsec; 
                calandertime=systime.tv_sec; 
             }
		return calandertime;
}

