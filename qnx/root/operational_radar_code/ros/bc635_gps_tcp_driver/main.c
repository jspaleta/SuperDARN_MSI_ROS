#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/types.h>
#ifdef __QNX__
  #include <hw/pci.h>
  #include <hw/inout.h>
  #include <sys/neutrino.h>
  #include <sys/iofunc.h>
  #include <sys/dispatch.h>
#endif
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "control_program.h"
#include "global_server_variables.h"
#include "include/registers.h"
#include "include/_refresh_state.h"
#include "utils.h"
#include "bc635_registers.h"
#define SYNTH_CLOCK 1E6

int	sock,msgsock,sockaccepted=0;
unsigned char	 *DPram=NULL;
struct DEV_reg *DEVreg=NULL;
struct DP_ram_ofs DPram_ofs;
int IRQ;
int tc_count=0;
int intid;
int configured=0,locked=0;
struct 	sigevent interruptevent;
struct  GPSStatus displaystat;
struct	timespec timecompare,event;
int 	timecompareupdate=0,eventupdate=0;
pthread_t int_thread_id, refresh_thread_id;
int verbose=10;
int wait_for_lock=0;
pthread_mutex_t gps_state_lock;

int set_time_compare(int mask,struct timespec *nexttime);
int set_time_compare_register(int mask,struct timespec *nexttime);

void graceful_cleanup(int signum)
{

  char path[256];
  sprintf(path,"%s:%d","bc365",0);
  close(msgsock);
  close(sock);
  unlink(path);
  fprintf(stdout,"Unlinking Unix Socket: %s\n",path);
  exit(0);

}
/*-MAIN--------------------------------------------------------------*/
 main(){
	int		 temp, pci_handle, i, frame_size, status, temp2, ch;
        char		 buffer[256];
        char		 lock_status;
        uint16		 *puint16;
        unsigned long	 n1,n2;
        double		 rate;
       
        uint16		 temp16;
        uint8		 temp8;
	unsigned int	 CLOCK_RES;
	struct		 timespec now,start_p;
	struct		 timespec new, old;
	int		 gpssecond,gpsnsecond;
        struct timeval tv;
        struct DriverMsg msg;
	// socket and message passing variables
	char	datacode;
	int	rval,buf; 
        int     first_gps_lock;
        fd_set rfds,efds;

        signal(SIGINT, graceful_cleanup);

        pthread_mutex_init(&gps_state_lock, NULL);

    /* SET THE SYSTEM CLOCK RESOLUTION AND GET THE START TIME OF THIS PROCESS */
	new.tv_nsec=10000;
//	sleep.tv_sec=0;
//	sleep.tv_nsec=10000;
#ifdef __QNX__
/*
	temp=ClockPeriod(CLOCK_REALTIME,&new,0,0);
	if(temp==-1){
		perror("Unable to change system clock resolution");
	}
	temp=clock_gettime(CLOCK_REALTIME, &start_p);
	if(temp==-1){
		perror("Unable to read sytem time");
	}
	temp=ClockPeriod(CLOCK_REALTIME,0,&old,0);
	CLOCK_RES=old.tv_nsec;
	if (verbose > 0) {
          fprintf(stderr,"CLOCK_RES: %d\n", CLOCK_RES);
          fflush(stderr); 
        }
*/
#endif

    /* OPEN THE PLX9656 AND GET LOCAL BASE ADDRESSES */
//	clock_gettime(CLOCK_REALTIME, &start);
	temp=_open_PCI(&DEVreg, &DPram, &pci_handle, &IRQ);
	if(temp==-1){
		fprintf(stderr, "	PLX9695 configuration failed\n");
                fflush(stderr);
                configured=0;
	}
	else{
		if (verbose > -1) fprintf(stderr,"IRQ is %d\n",IRQ);
                fflush(stderr);
                configured=1;
	}
     //INITIALIZE CARD
        if(configured) {
          fprintf(stderr,"DEVreg %p  DPram %p\n",DEVreg,DPram);
          fflush(stderr);
          DPram_ofs.input=DPram+0x102;
          DPram_ofs.output=DPram+0x82;
          DPram_ofs.GPS=DPram+0x02;
          DPram_ofs.year=DPram+0x00;


          fprintf(stderr,"Model: ");
          buffer[0] = DATUM_GET_DATA_CMD;
          buffer[1] = DATUM_GET_DATA_TFPMODEL;
          write_dpram_data(DEVreg, DPram_ofs.input, &buffer[0], 2 );
          temp=*((uint8*)(DPram_ofs.output));
          for(i=1;i<=8;i++) {
            temp=*((uint8*)(DPram_ofs.output+i));
            fprintf(stderr,"%c",(char)temp);
          }
          fprintf(stderr,"\n");

          fprintf(stderr,"Part: ");
          buffer[0] = DATUM_GET_DATA_CMD;
          buffer[1] = DATUM_GET_DATA_DTFW;
          write_dpram_data(DEVreg, DPram_ofs.input, &buffer[0], 2 );
          temp=*((uint8*)(DPram_ofs.output));
          for(i=1;i<=6;i++) {
            temp=*((uint8*)(DPram_ofs.output+i));
            fprintf(stderr,"%c",(char)temp);
          }
          fprintf(stderr,"\n");

          fprintf(stderr,"S/N: ");
          buffer[0] = DATUM_GET_DATA_CMD;
          buffer[1] = DATUM_GET_DATA_SERIAL;
          write_dpram_data(DEVreg, DPram_ofs.input, &buffer[0], 2 );
          temp=*((uint8*)(DPram_ofs.output));
          temp=*((uint32*)(DPram_ofs.output+1));
          fprintf(stderr,"0x%x",temp);
          fprintf(stderr,"\n"); 



          fprintf(stderr,"Soft Reset\n");
          buffer[0] = DATUM_SOFT_RESET_CMD;
          write_dpram_data(DEVreg, DPram_ofs.input, &buffer[0], 1 );

          fprintf(stderr,"Set Time Mode: GPS\n");
          buffer[0] = DATUM_SET_TIMING_MODE_CMD;
          buffer[1] = DATUM_MODE_GPS;
          write_dpram_data(DEVreg, DPram_ofs.input, &buffer[0], 2 );

          fprintf(stderr,"Disable Periodic Output\n");
          buffer[0] = DATUM_ENABLE_PER_DDS_CMD;
          buffer[1] = DATUM_DISABLE_PER;
          write_dpram_data(DEVreg, DPram_ofs.input, &buffer[0], 2 );

          fprintf(stderr,"Select Periodic Output\n");
          buffer[0] = DATUM_SELECT_PER_DDS_CMD;
          buffer[1] = DATUM_SELECT_PER;
          write_dpram_data(DEVreg, DPram_ofs.input, &buffer[0], 2 );


          rate=0.0;  // 10 Hz
          if(rate!=0) {

          n1=2;
          n2= SYNTH_CLOCK/(n1*rate);
          if (n2 < 2) n2=2; 
          while (n2 > 65535) {
            n1++;
            n2=1E6/(n1*rate);
            if(n1 > 65535) {
              n1=65535;
              n2=65535;
              break;
            }
          }

          rate=SYNTH_CLOCK/(n1*n2);  // 10 Hz
          n1=htons(n1); //Ensure MSB ordering
          n2=htons(n2); //Ensure MSB ordering
          fprintf(stderr,"Set Periodic Output Rate %lf Hz :",rate);
          buffer[0] = DATUM_SET_PERIODIC_CMD;
          buffer[1] = DATUM_NO_SYNC_PPS;
          puint16=(uint16*)&buffer[2];
          *puint16 = (uint16) n1;
          fprintf(stderr," %d : %d :: ",(int) *puint16, (int) ntohs(*puint16));
          puint16=(uint16*)&buffer[4];
          *puint16 = (uint16) n2;
          fprintf(stderr," %d : %d ",(int) *puint16, (int) ntohs(*puint16));
          fprintf(stderr,"\n"); 
          write_dpram_data(DEVreg, DPram_ofs.input, &buffer[0], 6 );

            fprintf(stderr,"Enable Periodic Output\n");
            buffer[0] = DATUM_ENABLE_PER_DDS_CMD;
            buffer[1] = DATUM_ENABLE_PER;
            write_dpram_data(DEVreg, DPram_ofs.input, &buffer[0], 2 );
          }
          fprintf(stderr,"Set Control Register : ");
/*  Control register: 
    bit 0:   Event1 lockout : 0 - disable 1 - enable
    bit 1:   Event1 source  : 0 - input   1 - Per/DDS
    bit 2:   Event1 edge    : 0 - rising  1 - falling
    bit 3:   Event 1 capture: 0 - disable 1 - enable
    bit 4:   Strobe output  : 0 - disable 1 - enable
    bit 5:   Strobe Mode    : 0 - Use sec 1 - Every sec
    bit 6-7: Freq Select    : 00 - 100 Mhz 01 - 5 Mhz  1x - 1Mhz 
    bit 8:   Event2 lockout : 0 - disable 1 - enable
    bit 9:   Event2 source  : 0 - input   1 - Per/DDS
    bit 10:  Event2 edge    : 0 - rising  1 - falling
    bit 11:  Not Used 
    bit 12:  Event3 lockout : 0 - disable 1 - enable
    bit 13:  Event3 source  : 0 - input   1 - Per/DDS
    bit 14:  Event3 edge    : 0 - rising  1 - falling
*/
          DEVreg->control=0x09;  // Enable Event1 Capture on external pin;  10 MHz output
          usleep(10);
          printbits(stderr,DEVreg->control); 
          fprintf(stderr,"\n"); 
 
          first_gps_lock=0;
          while(wait_for_lock) {
            if(!first_gps_lock && configured) {
                  pthread_mutex_lock(&gps_state_lock);
                  fprintf(stderr,"DEVreg INTSTAT:");
                  printbits(stderr,DEVreg->intstat); 
                  fprintf(stderr,"\n"); 
                  DEVreg->intstat=0x7F;
                  fprintf(stderr,"request Time\n");
  	          temp=DEVreg->timereq;
                  fprintf(stderr,"DEVreg TIME1:");
                  printbits(stderr,DEVreg->time1); 
                  fprintf(stderr," %lu\n",DEVreg->time1); 
                  fprintf(stderr,"DEVreg TIME0:");
                  printbits(stderr,DEVreg->time0); 
                  fprintf(stderr,"\n"); 
                  lock_status=(DEVreg->time0 & 0x7000000) >> 24;
                  fprintf(stderr,"Status: 0x%x : ",lock_status);
                  if (lock_status==0) {
                    fprintf(stderr,"Good");
                    first_gps_lock=1;
                  } else {
                    fprintf(stderr,"Bad: "); 
                    if ((lock_status & 1)==1) fprintf(stderr," Track ");
                    if ((lock_status & 2)==2) fprintf(stderr," Phase ");
                    if ((lock_status & 4)==4) fprintf(stderr," Freq ");
                  }
                  fprintf(stderr,"\n"); 

                  pthread_mutex_unlock(&gps_state_lock);
                  if(!first_gps_lock) {
                    fprintf(stderr,"No GPS Lock Yet\n");
                    fflush(stderr);
                    locked=0;
                    if(wait_for_lock) {
                      sleep(10);
                      continue; 
                    } else {
                      fprintf(stderr,"GPS Not Waiting for Lock, willing to talk to ROS now\n");
                      fflush(stderr);
                      first_gps_lock=1; 
                      locked=1;
                    }
                  } else {
                    fprintf(stderr,"GPS Lock, willing to talk to ROS now\n");
                    fflush(stderr);
                    locked=1;
                    wait_for_lock=0;
                  } 
            }
          }
        } else {
          fprintf(stderr,"Card not found"); 
          exit(-1);
        }




	//start thread to refresh display
        if (verbose > -1) {
          fprintf(stderr,"starting refresh thread\n");
          fflush(stderr);
        }
	pthread_create(&refresh_thread_id, NULL, refresh_state, NULL);
	temp=clock_gettime(CLOCK_REALTIME, &start_p);
	//open tcp socket	
	//sock=tcpsocket(GPS_HOST_PORT);
	sock=server_unixsocket("rosgps",0);
	listen(sock,5);
	while(1){
        
                rval=1;
		msgsock=accept(sock, 0, 0);
		if (verbose > 0) printf("accepting socket!!!!!\n");
		if( (msgsock==-1) ){
			perror("accept FAILED!");
			return EXIT_FAILURE;
		}
		else while (rval>=0){
                  FD_ZERO(&rfds);
                  FD_SET(msgsock, &rfds); //Add msgsock to the read watch
                  FD_ZERO(&efds);
                  FD_SET(msgsock, &efds);  //Add msgsock to the exception watch
		  if (verbose > 0) printf("%d Entering Select\n",msgsock);
                  rval = select(msgsock+1, &rfds, NULL, &efds, NULL);
		  if (verbose > 0) printf("%d Leaving Select %d\n",msgsock,rval);
                  if (FD_ISSET(msgsock,&efds)){
                    if (verbose > 0) printf("Exception on msgsock %d ...closing\n",msgsock);
                    break;
                  }
                  if (rval == -1) perror("select()");
                  rval=recv(msgsock, &buf, sizeof(int), MSG_PEEK); 
                  if (verbose>0) printf("%d PEEK Recv Msg %d\n",msgsock,rval);
		  if (rval==0) {
                    if (verbose > 0) printf("Remote Msgsock %d client disconnected ...closing\n",msgsock);
                    break;
                  } 
		  if (rval<0) {
                    if (verbose > 0) printf("Msgsock %d Error ...closing\n",msgsock);
                    break;
                  } 
                  if ( FD_ISSET(msgsock,&rfds) && rval>0 ) {
                    if (verbose>0) printf("Data is ready to be read\n");
		    if (verbose > 0) printf("%d Recv Msg\n",msgsock);
                    rval=recv_data(msgsock,&msg,sizeof(struct DriverMsg));
                    datacode=msg.type;
		    if (verbose > 0) printf("\nmsg code is %c\n", datacode);
                    locked|=displaystat.gps_lock;
		    switch( datacode ){
				case GPS_GET_SOFT_TIME:
                                        fprintf(stderr," Get SOFT TIME : Configured: %d Locked: %d\n",configured,locked); 
                                        fflush(stderr);
                                        gpssecond=0;
                                        gpsnsecond=0;
                                        msg.status=get_software_time(&gpssecond,&gpsnsecond,DEVreg,locked);
                                        if(verbose > -1 ) fprintf(stderr,"Software Time: %s\n",ctime(&gpssecond)); 
					rval=send_data(msgsock,&gpssecond, sizeof(int));
					rval=send_data(msgsock,&gpsnsecond, sizeof(int));
                                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
					break;
				case GPS_GET_EVENT_TIME:
                                        gpssecond=0;
                                        gpsnsecond=0;
                                        if (configured && locked ) {
                                         if(verbose > -1 ) fprintf(stderr," Get GPS  EVENT: Configured: %d Locked: %d\n",configured,locked); 
                                          msg.status=get_event_time(&gpssecond,&gpsnsecond,DEVreg,locked);
                                        }
                                        else {
                                         if( verbose > -1 ) fprintf(stderr," Get SOFT EVENT : Configured: %d Locked: %d\n",configured,locked); 
                                         msg.status=get_software_time(&gpssecond,&gpsnsecond,DEVreg,locked);
                                        }
                                        if(verbose > 1 ) fprintf(stderr," %s\n",ctime(&gpssecond)); 
                                        if(verbose > 1) fflush(stderr);
					rval=send_data(msgsock,&gpssecond, sizeof(int));
					rval=send_data(msgsock,&gpsnsecond, sizeof(int));
                                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
					break;
				case GPS_SET_TRIGGER_RATE:
  		                        if (verbose > 1) printf("Inside Set Rate: %c\n", datacode);
					rval=recv_data(msgsock,&displaystat.ratesynthrate,sizeof(int));
  		                        if (verbose > -1) printf("Set Rate: %d\n", displaystat.ratesynthrate);
					if (configured ) {
					  if(displaystat.ratesynthrate==0) {
          					fprintf(stderr,"Disable Periodic Output\n");
          					buffer[0] = DATUM_ENABLE_PER_DDS_CMD;
          					buffer[1] = DATUM_DISABLE_PER;
          					write_dpram_data(DEVreg, DPram_ofs.input, &buffer[0], 2 );
                                          } else {

          					rate=displaystat.ratesynthrate;  // Hz
          					n1=10;
          					n2=SYNTH_CLOCK/(n1*rate);
          					if (n2 < 2) n2=2; 
          					while (n2 > 65535) {
            						n1++;
            						n2=1E6/(n1*rate);
            						if(n1 > 65535) {
              							n1=65535;
              							n2=65535;
              							break;
            						}
          					}
						rate=1E8/(n1*n2);
          					n1=htons(n1); //Ensure MSB ordering
          					n2=htons(n2); //Ensure MSB ordering
          					fprintf(stderr,"Set Periodic Output Rate %d Hz :",rate);
          					buffer[0] = DATUM_SET_PERIODIC_CMD;
          					buffer[1] = DATUM_NO_SYNC_PPS;
          					puint16=(uint16*)&buffer[2];
          					*puint16 = (uint16) n1;
          					fprintf(stderr," %d",(int) *puint16);
          					puint16=(uint16*)&buffer[4];
          					*puint16 = (uint16) n2;
          					fprintf(stderr," %d",(int) *puint16);
          					fprintf(stderr,"\n"); 
          					write_dpram_data(DEVreg, DPram_ofs.input, &buffer[0], 6 );
          					fprintf(stderr,"Enable Periodic Output\n");
          					buffer[0] = DATUM_ENABLE_PER_DDS_CMD;
          					buffer[1] = DATUM_ENABLE_PER;
          					write_dpram_data(DEVreg, DPram_ofs.input, &buffer[0], 2 );
					  }
                                        }
                                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
  		                        if (verbose > -1) printf("Leaving Set Rate: %c\n", datacode);
					break;
				case GPS_GET_HDW_STATUS:
					//this function does not yet work
  		                        if (verbose > 0) printf("Inside Get HDW status: %c\n", datacode);
                                        pthread_mutex_lock(&gps_state_lock);
                                        get_state();
                                        pthread_mutex_unlock(&gps_state_lock);
                                        rval=send_data(msgsock, &displaystat, sizeof(struct GPSStatus));
                                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
					break;
				case GPS_TRIGGER_NOW:
					//this function does not yet work
					status=0;
					rval=send_data(msgsock,&status, sizeof(int));
					break;
				default:	
					if (verbose > 0) fprintf(stderr,"BAD CODE: %c : %d\n",datacode,datacode);
					break;
			}
                  }
		}
	}
	
	


    /* END TEST CODE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */






    /* CLOSE PCI device AND CLEAR ALL MEMORY MAPS */
	temp=_close_PCI(pci_handle, DEVreg, DPram);
	fprintf(stderr,"close pci device\n");

    /* READ SYSTEM TIME AND END OF PROCESS, AND PRINT TOTAL TIME TO RUN */
	if(temp==-1){
		perror("Unable to read sytem time");
	}

 }
