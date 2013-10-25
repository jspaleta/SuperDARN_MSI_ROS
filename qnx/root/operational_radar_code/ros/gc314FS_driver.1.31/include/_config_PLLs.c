#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "_prog_conventions.h"
#include "_regs_GC314FS.h"

int _config_PLLs(char *BASE1, int clock_source, long clock_freq){

	struct		timespec start, stop, sleep;
	unsigned long	temp;

	sleep.tv_sec=0;
	sleep.tv_nsec=10000;
	
	if (clock_source==CLOCK_INTERNAL){
		clock_freq=40000000;
	}
	
	if (clock_source==CLOCK_INTERNAL){
		//release PLL reset
		*((uint32*)(BASE1+GC314FS_CPLL))	= 0x00000002;
		sleep.tv_nsec=20000000;	
		clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
		clock_gettime(CLOCK_REALTIME, &start);
		// program PLLs
		*((uint32*)(BASE1+GC314FS_PLL_M))=	0x0070000a;
		*((uint32*)(BASE1+GC314FS_PLL_G0))=	0x00201405;
		*((uint32*)(BASE1+GC314FS_PLL_E0))=	0x00201405;
		*((uint32*)(BASE1+GC314FS_PLL_E1))=	0x00201405;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
		*((uint32*)(BASE1+GC314FS_PLL_E2))=	0x00201405;
		*((uint32*)(BASE1+GC314FS_PLL_E3))=	0x00201405;
		// reprogram PLLs with new register values
		*((uint32*)(BASE1+GC314FS_CPLL))	|= 0x0008;
		// wait 200 ms for PLLs to lock
		sleep.tv_nsec=20000000;	
		clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
		clock_gettime(CLOCK_REALTIME, &start);
		while (1){
			clock_gettime(CLOCK_REALTIME, &stop);
			if ( stop.tv_sec-start.tv_sec > 2){
				fprintf(stderr, "	PLLs could not lock to internal clock\n");
				return -1;
			}
			if ( (*((uint32*)(BASE1+GC314FS_CPLL)) & 0x00000004) != 0x00000004 ){
				clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
			}
			else{
				break;
			}
		}	
	}
	else if (clock_source==CLOCK_EXTERNAL){
		// Check for existing PLL lock
		// if locked, do not change a thing
//		if ( ( *((uint32*)(BASE1+GC314FS_CPLL)) & 0x00000004 ) != 0x00000004 ){	
//	  	  fprintf(stderr,"	PLLs not locked to external clock\n");
//                  fflush(stderr);
              if(1) {  
	  	  fprintf(stderr,"	Force PLLs programming to external clock\n");
                  fflush(stderr);
		// enable PLL output
		  *((uint32*)(BASE1+GC314FS_CPLL))	|= 0x00000002;
		  sleep.tv_nsec=20000000;	
		  clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
		  clock_gettime(CLOCK_REALTIME, &start);
		// program PLLs
		  if ( (clock_freq>=40000000) & (clock_freq<50000000) ){
			*((uint32*)(BASE1+GC314FS_PLL_M))=	0x0070000a;
			*((uint32*)(BASE1+GC314FS_PLL_G0))=	0x00201405;
			*((uint32*)(BASE1+GC314FS_PLL_E0))=	0x00201405;
			*((uint32*)(BASE1+GC314FS_PLL_E1))=	0x00201405;
			*((uint32*)(BASE1+GC314FS_PLL_E2))=	0x00201405;
			*((uint32*)(BASE1+GC314FS_PLL_E3))=	0x00201405;
		  }
		  else if ( (clock_freq>=50000000) & (clock_freq<65000000) ){
			*((uint32*)(BASE1+GC314FS_PLL_M))=	0x00700008;
			*((uint32*)(BASE1+GC314FS_PLL_G0))=	0x00201004;
			*((uint32*)(BASE1+GC314FS_PLL_E0))=	0x00201004;
			*((uint32*)(BASE1+GC314FS_PLL_E1))=	0x00201004;
			*((uint32*)(BASE1+GC314FS_PLL_E2))=	0x00201004;
			*((uint32*)(BASE1+GC314FS_PLL_E3))=	0x00201004;
		  }
		  else if ( (clock_freq>=65000000) & (clock_freq<90000000) ){
			*((uint32*)(BASE1+GC314FS_PLL_M))=	0x00700006;
			*((uint32*)(BASE1+GC314FS_PLL_G0))=	0x00200c03;
			*((uint32*)(BASE1+GC314FS_PLL_E0))=	0x00200c03;
			*((uint32*)(BASE1+GC314FS_PLL_E1))=	0x00200c03;
			*((uint32*)(BASE1+GC314FS_PLL_E2))=	0x00200c03;
			*((uint32*)(BASE1+GC314FS_PLL_E3))=	0x00200c03;
		  }
		  else if ( (clock_freq>=90000000) & (clock_freq<=105000000) ){
			*((uint32*)(BASE1+GC314FS_PLL_M))=	0x00700004;
			*((uint32*)(BASE1+GC314FS_PLL_G0))=	0x00200802;
			*((uint32*)(BASE1+GC314FS_PLL_E0))=	0x00200802;
			*((uint32*)(BASE1+GC314FS_PLL_E1))=	0x00200802;
			*((uint32*)(BASE1+GC314FS_PLL_E2))=	0x00200802;
			*((uint32*)(BASE1+GC314FS_PLL_E3))=	0x00200802;
		  }
		  else if ( (clock_freq>105000000) ){
			fprintf(stderr,"	Clock frequency greater than 105 MHz is not acceptable\n");
                        fflush(stderr); 
			return -1;
		  }
		  else if ( (clock_freq<40000000) ){
			fprintf(stderr,"	Clock frequency of less than 40 MHz is not acceptable\n");
                        fflush(stderr);
			return -1;
		  }
		  else{
			fprintf(stderr,"	Clock value of %d is not properly defined\n", clock_freq);
                        fflush(stderr);
			return -1;
		  }
		// reprogram PLLs with new register values
		  *((uint32*)(BASE1+GC314FS_CPLL))	|= 0x0008;
		// wait 200 ms for PLLs to lock
		  sleep.tv_nsec=20000000;	
		  clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
		  clock_gettime(CLOCK_REALTIME, &start);
		  while (1){
			clock_gettime(CLOCK_REALTIME, &stop);
			if ( (stop.tv_sec-start.tv_sec) > 1 ){
				fprintf(stderr,"	PLLs could not lock to external clock\n");
                                fflush(stderr);
				return -1;
			}
			if ( (*((uint32*)(BASE1+GC314FS_CPLL)) & 0x00000004) != 0x00000004 ){
				clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
			}
			else{
				break;
			}
		  }
	  	  fprintf(stderr,"	PLLs programmed and locked to external clock\n");
                  fflush(stderr); 
		} else {
	  	  fprintf(stderr,"	PLLs initially locked to external clock\n");
                  fflush(stderr); 
                }
	}
	else{
		fprintf(stderr, "	Clock source not properly defined\n");
                fflush(stderr); 
		return -1;
	}
	temp=*((uint32*)(BASE1+GC314FS_SISR));
        usleep(100);
	if ( ( *((uint32*)(BASE1+GC314FS_CPLL)) & 0x00000004 ) != 0x00000004 ){	
  	  fprintf(stderr,"	PLLs not locked to external clock\n");
          fflush(stderr);
        }
	return 1;
}
