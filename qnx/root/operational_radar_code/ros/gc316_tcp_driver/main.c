#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <signal.h>
#include <math.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fftw3.h>
#ifdef __QNX__
  #include <sys/iofunc.h>
  #include <sys/dispatch.h>
  #include <devctl.h>
  #include <sched.h>
  #include <process.h>
  #include <sys/iomsg.h>
  #include <sys/uio.h>
  #include <sys/resmgr.h>
  #include <sys/neutrino.h>
  #include <hw/inout.h>
  #include <hw/pci.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
#endif
#include "control_program.h"
#include "global_server_variables.h"
#include "utils.h"
#include "beam_phase.h"
#include "gc314FS_defines.h"
#include "gc314FS.h"
#include "_prog_conventions.h"
#include "_regs_GC314FS.h"
#include "rtypes.h"
#define IMAGING 0 
#define BUFS 1
#define MAX_SAMPLES 262144 
#define CLR_SAMP_OFFSET 10
int verbose=0;
int write_clr_file=0; 
FILE *clr_data;

int sock,msgsock;
FILE 		 *gc314fs[MAX_CARDS];

unsigned int     virtual_addresses[MAX_CARDS][MAX_INPUTS][MAX_CHANNELS][BUFS];
unsigned int     physical_addresses[MAX_CARDS][MAX_INPUTS][MAX_CHANNELS][BUFS];
unsigned int     *main_test_data[MAX_RADARS][MAX_CHANNELS][BUFS],*back_test_data[MAX_RADARS][MAX_CHANNELS][BUFS],*aux_test_data[MAX_RADARS][MAX_CHANNELS][BUFS]; 

unsigned int     *summed_main_addresses[MAX_RADARS][MAX_CHANNELS][BUFS];
unsigned int     *summed_back_addresses[MAX_RADARS][MAX_CHANNELS][BUFS]; 

unsigned int     antennas[20][2] = {
                 /* card, input */
			/* main array */                 
			{0,0},
			{0,1},
			{0,2},
			{1,0},
			{1,1},
			{1,2},
			{2,0},
			{2,1},
			{2,2},
			{3,0},
			{3,1},
			{3,2},
			{4,0},
			{4,1},
			{4,2},
			{5,0},
			{5,1},
			/* back array */
			{5,2},
			{6,0},
			{6,1},
		};
double          time_delay_correction[20] = {
                  0.0, 0.0, 0.0, 10E-9, 10E-9, 10E-9, 0.0, 0.0, 0.0, 0.0,
                  0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                };
int 		use_flag[20] = {
     		  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1, 0, 0, 0, 1, 1,
                };

int armed=0,post_clr[4]={0,0,0,0};
int main_input=2;
int back_input=1;
int aux_input=0;
int write_out=1;
int write_clr_out=0;


void graceful_cleanup(int signum)
{
  char path[256];
  sprintf(path,"%s:%d","rosrecv",0);
  close(msgsock);
  close(sock);
  fprintf(stdout,"Unlinking Unix Socket: %s\n",path);
  unlink(path);
  exit(0);
}


void write_raw_files (int tfreq, int beam, int sample, int radar,int channel, int buffer) {

  FILE *ftest;
  char  data_file[255], data_file2[255], data_file3[255], err_file[255], strtemp[255], test_file[255];
  char   chan_str[12];
  struct timespec time_now;
  struct           tm* time_struct;
  int32 temp ,*tmp_ptr;
  int fd_1,i;
  chan_str[0]='\0';
  switch (channel) {
    case 0:
      strcat(chan_str, ".a");
      break;
    case 1:
      strcat(chan_str, ".b");
      break;
    case 2:
      strcat(chan_str, ".c");
      break;
    case 3:
      strcat(chan_str, ".d");
      break;
  }
  
  test_file[0]='\0';
  strcat(test_file,"/collect.now");
  strcat(test_file,chan_str);
  ftest=fopen(test_file, "r");
  if(ftest!=NULL){
    fclose(ftest);
    data_file[0]='\0';
    data_file2[0]='\0';
    clock_gettime(CLOCK_REALTIME, &time_now);
    time_struct=gmtime(&time_now.tv_sec);
    // data file directory
    
    strcat(data_file, "/data/image_samples/");
    // data file year
    temp=(int)time_struct->tm_year+1900;
    sprintf(strtemp,"%04d",temp);
    //ltoa(temp, strtemp, 10);
    strcat(data_file, strtemp);
    // data file month
    temp=(int)time_struct->tm_mon;
    sprintf(strtemp,"%02d",temp+1);
    //ltoa(temp+1,strtemp,10);
    //if(temp<10) strcat(data_file, "0");
    strcat(data_file, strtemp);
    // data file day
    temp=(int)time_struct->tm_mday;
    sprintf(strtemp,"%02d",temp);
    //ltoa(temp,strtemp,10);
    //if(temp<10) strcat(data_file, "0");
    strcat(data_file, strtemp);
    // data file hour
    temp=(int)time_struct->tm_hour;
    sprintf(strtemp,"%02d",temp);
    //ltoa(temp,strtemp,10);
    //if(temp<10) strcat(data_file, "0");
    strcat(data_file, strtemp);
    // data file 5 of minutes
    temp=(int)time_struct->tm_min;
    temp=((int)(temp/5))*5;
    sprintf(strtemp,"%02d",temp);
    //ltoa(temp,strtemp,10);
    strcat(data_file, strtemp);
    //if(temp<10) strcat(data_file, "0");
    // data file suffix
    strcat(data_file, ".");
    temp=radar;
    sprintf(strtemp,"%d",temp);
    //ltoa(temp,strtemp,10);
    strcat(data_file, strtemp);
    strcat(data_file, ".iraw");
    strcat(data_file, chan_str);
    fd_1=open(data_file, O_WRONLY|O_CREAT|O_NONBLOCK|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    temp=(int)time_struct->tm_year+1900;
    write(fd_1, &temp, sizeof(int));
    temp=(int)time_struct->tm_mon+1;
    write(fd_1, &temp, sizeof(int));
    temp=(int)time_struct->tm_mday;
    write(fd_1, &temp, sizeof(int));
    temp=(int)time_struct->tm_hour;
    write(fd_1, &temp, sizeof(int));
    temp=(int)time_struct->tm_min;
    write(fd_1, &temp, sizeof(int));
    temp=(int)time_struct->tm_sec;
    write(fd_1, &temp, sizeof(int));
    temp=(int)time_now.tv_nsec;
    write(fd_1, &temp, sizeof(int));
    write(fd_1, &beam, sizeof(beam));
    write(fd_1, &tfreq, sizeof(tfreq));
    write(fd_1, &post_clr[channel], sizeof(int));
    write(fd_1, &sample, sizeof(sample));
    if (IMAGING) {
      for(i=0;i<20;i++){
        tmp_ptr=(int*)virtual_addresses[antennas[i][0]][antennas[i][1]][channel][buffer];
        write(fd_1, tmp_ptr, 4*sample);
      }
    } else {
      for(i=0;i<3;i++) {
        tmp_ptr=(int*)virtual_addresses[radar][i][channel][buffer];
        write(fd_1, tmp_ptr, 4*sample);
      }
    }
    close(fd_1);
  }
};
 
void   chunk_copy(t_copy_chunk *arg) {
  int i,j;
  double d=ANTENNA_SEPARATION, pi=M_PI, delta, M, phi,phase,phase_correction;
  short   I, Inew, Iout, Q, Qnew, Qout;
  double  Itemp, Qtemp;
  unsigned int *tmpptr;
  unsigned int *main;



  delta=calculate_delta(arg->frequency,beamdirs_rad[arg->beamdir],d);

  for(i=0;i<arg->length;i++){
    Itemp=0.0;
    Qtemp=0.0;
    for(j=0;j<16;j++){
      if (use_flag[j]==1) {
        main=(unsigned int *)virtual_addresses[antennas[j][0]][antennas[j][1]][arg->channel][arg->buffer];
        I=(short)( main[arg->start+i] & 0x0000ffff );
        Q=(short)((main[arg->start+i] >> 16) & 0x0000ffff );
        M=sqrt( (double)(I*I+Q*Q) );
        phi=atan2((double)Q, (double)I);
        phase=delta*((double)j-7.5);
        phase_correction=arg->frequency*time_delay_correction[j]*2.*pi;
        Itemp+=M * cos( phi + phase + phase_correction);
        Qtemp+=M * sin( phi + phase + phase_correction);
      }
    }
    Iout=(short)(Itemp/16);
    Qout=(short)(Qtemp/16);
    tmpptr=summed_main_addresses[arg->radar][arg->channel][arg->buffer];
    tmpptr[arg->start+i]= ((short)Iout & 0x0000ffff) + ( ((short)Qout & 0x0000ffff) << 16);
    Itemp=0.0;
    Qtemp=0.0;
    for(j=0;j<4;j++){
      if (use_flag[j+16]==1) {
        main=(unsigned int *)virtual_addresses[antennas[j+16][0]][antennas[j+16][1]][arg->channel][arg->buffer];
        I=(short)( main[arg->start+i] & 0x0000ffff );
        Q=(short)((main[arg->start+i] >> 16) & 0x0000ffff );
        M=sqrt( (double)I*(double)I + (double)Q*(double)Q );
        phi=atan2((double)Q, (double)I);
        phase=delta*((double)j-1.5);
        phase_correction=arg->frequency*time_delay_correction[j+16]*2.*pi;
        Itemp+=M * cos( phi + phase + phase_correction);
        Qtemp+=M * sin( phi + phase + phase_correction);
      }
    }
    Iout=(short)(Itemp/4);
    Qout=(short)(Qtemp/4);
    tmpptr=summed_back_addresses[arg->radar][arg->channel][arg->buffer];
    tmpptr[arg->start+i]= ((short)Iout & 0x0000ffff) + ( ((short)Qout & 0x0000ffff) << 16);
  }
  pthread_exit(NULL);
}  
int add_phase(int frequency, int beamdir, int length, int radar,int channel, int buffer,int write){
        int     c,cc,rc,i,j;
        short I,Q;
        double M; 
        unsigned int *main;
        FILE *fp;
        struct timeval t0,t1,t2,t3;
        unsigned long elapsed;
        t_copy_chunk *chunks=NULL;
        int chunk_start,chunk_length,chunk_left;
        int num_threads=5;
        pthread_t threads[4];
        char filename[80];

        char   test_file[255];
        char   chan_str[12];
        FILE *ftest;

        if(chunks==NULL) 
          chunks = (t_copy_chunk *) malloc(sizeof(t_copy_chunk) * num_threads);
        gettimeofday(&t0,NULL);
        chunk_length=length/(num_threads-1);
        chunk_start=0;
        chunk_left=length; 
        cc=-1;
       for(c=0;c<num_threads;c++){
          if ( chunk_left < chunk_length ) {
            if (chunk_left <=0 ) {
              chunk_length=0;
            }  else chunk_length=chunk_left; 
          } 
          chunks[c].radar=radar;
          chunks[c].beamdir=beamdir;
          chunks[c].frequency=frequency;
          chunks[c].channel=channel;
          chunks[c].buffer=buffer;
          chunks[c].start=chunk_start;
          chunks[c].length=chunk_length;
          chunk_start+=chunk_length; 
          chunk_left-=chunk_length;
          if (chunks[c].length > 0) {
            cc++; 
            rc = pthread_create(&threads[cc], NULL, (void *) &chunk_copy, &chunks[c]);
          }
       }
 
       for(;cc>=0;cc--){
         pthread_join(threads[cc],NULL);
       }

       gettimeofday(&t1,NULL);
       elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
       elapsed+=(t1.tv_usec-t0.tv_usec);
       if (verbose > 1 ) printf("  Phase Add Elapsed Microseconds: %ld\n",elapsed);

       if(write) {
         chan_str[0]='\0';
         switch (channel) {
          case 0:
           strcat(chan_str, ".a");
           break;
          case 1:
           strcat(chan_str, ".b");
           break;
          case 2:
           strcat(chan_str, ".c");
           break;
          case 3:
           strcat(chan_str, ".d");
           break;
         }
  
         test_file[0]='\0';
         strcat(test_file,"/collect.ascii");
         strcat(test_file,chan_str);
         ftest=fopen(test_file, "r");
         if(ftest!=NULL){
           fclose(ftest);
           sprintf(filename,"/data/ascii_samples"); 
           strcat(filename,chan_str);
           fp=fopen(filename,"a+");
           for(i=0;i<length;i++) {
             fprintf(fp,"\t");
             if(i==0) {
               fprintf(fp,"--------\t");
               for(j=0;j<21;j++){
                 fprintf(fp,"%8d  %8d  %8.3lf \t",j,j,j);
               }
               fprintf(fp,"%8d  %8d  %8.3lf \n",21,21,21);
             }
             fprintf(fp,"%8d\t",i);
             for(j=0;j<16;j++){
               main=(unsigned int *)virtual_addresses[antennas[j][0]][antennas[j][1]][channel][buffer];
               I=(short)( main[i] & 0x0000ffff );
               Q=(short)((main[i] >> 16) & 0x0000ffff );
               M=sqrt( (double)(I*I+Q*Q) );
               if(use_flag[j]==1) 
                 fprintf(fp,"%8d  %8d  %8.3lf \t",I,Q,M);
               else 
                 fprintf(fp,"%8d* %8d* %8.3lf*\t",I,Q,M);
             }
             for(j=0;j<4;j++){
               main=(unsigned int *)virtual_addresses[antennas[j+16][0]][antennas[j+16][1]][channel][buffer];
               I=(short)( main[i] & 0x0000ffff );
               Q=(short)((main[i] >> 16) & 0x0000ffff );
               M=sqrt( (double)I*(double)I + (double)Q*(double)Q );
               if(use_flag[j+16]==1) 
                 fprintf(fp,"%8d  %8d  %8.3lf \t",I,Q,M);
               else 
                 fprintf(fp,"%8d* %8d* %8.3lf*\t",I,Q,M);
             }
             main=summed_main_addresses[radar][channel][buffer]; 
             I=(short)( main[i] & 0x0000ffff );
             Q=(short)((main[i] >> 16) & 0x0000ffff );
             M=sqrt( (double)I*(double)I + (double)Q*(double)Q );
             fprintf(fp,"%8d  %8d  %8.3lf \t",I,Q,M);
             main=summed_back_addresses[radar][channel][buffer]; 
             I=(short)( main[i] & 0x0000ffff );
             Q=(short)((main[i] >> 16) & 0x0000ffff );
             M=sqrt( (double)I*(double)I + (double)Q*(double)Q );
             fprintf(fp,"%8d  %8d  %8.3lf \n",I,Q,M);
           }
           fclose(fp);
         }
       }
       if(chunks!=NULL) {
          free(chunks);
          chunks=NULL;
       }
       return 1;
}

int main(int argc, char **argv){
	// socket and message passing variables
        struct  timeval tv;
	char	datacode;
        double phasediff;
	int	rval,nave;
        fd_set rfds,efds;
        int wait_status,status,configured=0;
        short I,Q;
	// counter and temporary variables
	int32	temp,buf,r,c,i,ii,j,n,b,N;
        unsigned int utemp;
        struct  DriverMsg msg;
        struct RXFESettings rf_settings;
        struct RXFESettings if_settings;
        uint32 ifmode=IF_ENABLED;
        struct CLRFreqPRM clrfreq_parameters;
        unsigned int *main;
        int  maxclients=MAX_RADARS*MAX_CHANNELS;
        int numclients=0;
        int ready_index[MAX_RADARS][MAX_CHANNELS];
        struct  ControlPRM  clients[maxclients],client;
        struct timeval tpre,tpost,t0,t1,t2,t3,t4,t5,t6;
        unsigned long elapsed;
        int card=0;
        char *driver;
        char shm_device[80];
        int shm_fd;
        int32 shm_memory=0;
	char cardnum[1];
        int frame_counter=1;
        unsigned int CLOCK_RES;
        int pci_handle, pci_handle_dio,IRQ;
        int32 samples;
        fftw_complex *in=NULL, *out=NULL;
        double *pwr=NULL,*pwr2=NULL;
        fftw_plan plan;
        int usable_bandwidth;
        double search_bandwidth,unusable_sideband;
        int centre,start,end;
        int max_retries=10,try=0;
        uint64_t main_address,back_address;
#ifdef __QNX__
	struct	 _clockperiod 	new, old;
#endif
	signal(SIGINT, graceful_cleanup);
        for (r=0;r<MAX_RADARS;r++){
          for (c=0;c<MAX_CHANNELS;c++){
            ready_index[r][c]=-1;
          }
        }
        ifmode=IF_ENABLED;
        if (verbose > 1) printf("RECV driver: IF Mode %d \n",ifmode);
#ifdef __QNX__
   /* SET UP COMMUNICATION TO GC314 DRIVERS */
        configured=1;
        if(IMAGING==1) {
          if ((MAX_CARDS*MAX_INPUTS) < (MAX_TRANSMITTERS+MAX_BACK_ARRAY)) {
	    fprintf(stderr, "Too few cards configured for imaging radar configuration\n");
            configured=0;
          }
          if (MAX_RADARS !=1 ) {
	    fprintf(stderr, "imaging configuration only supports one radar\n");
            configured=0;
          }
        } else {
          if (MAX_CARDS < MAX_RADARS) {
	    fprintf(stderr, "Too few cards configured for non-imaging radar configuration\n");
            configured=0;
          }
        }
        if (verbose > 0 ) printf("Opening block devices for each GC314 card\n");
	for(card=0;card<MAX_CARDS;card++){
		driver=calloc((size_t) 64, 1);
		strcat(driver,"/dev/gc314fs-");
		itoa(card,cardnum,10);
		strcat(driver,cardnum);
	  	gc314fs[card]=(FILE *)open(driver, O_RDWR);
		if( (int)(gc314fs[card]) < 0 ){
			fprintf(stderr, "Unable to open driver %s: %s\n", driver, strerror(errno));
                        configured=0;
		} else {
                  fprintf(stderr,"Opened driver %s: %d %d\n",driver,card,gc314fs[card]);
                }
		free(driver);
	}
#else
        configured=0;
#endif

//#ifdef __QNX__
    /* SET THE SYSTEM CLOCK RESOLUTION AND GET THE START TIME OF THIS PROCESS */
	// set the system clock resolution to 10 us
#ifdef __QNX__
/*
	new.nsec=10000;
	new.fract=0;
	temp=ClockPeriod(CLOCK_REALTIME,&new,0,0);
	if(temp==-1) 	perror("Unable to change system clock resolution");
*/
	temp=ClockPeriod(CLOCK_REALTIME,0,&old,0);
	if(temp==-1) 	perror("Unable to read sytem time");
	CLOCK_RES=old.nsec;
#endif
        if (configured) {
          if(IMAGING==1) {
            if (verbose > 1 ) printf("Setting up for Imaging Radar\n");
            for(card=0;card<MAX_CARDS;card++){                                           
              for(c=0;c<MAX_CHANNELS;c++){                                      
                for (i=0;i<MAX_INPUTS;i++) { 
  		  virtual_addresses[card][i][c][0]=
                    gc314GetBufferAddress(&physical_addresses[card][i][c][0],gc314fs[card],i,c);	
		}
              }
            } 
	    //printf("Filling Summed Arrays\n");
            shm_memory=1; //uses shm device nodes for summed data.
	    for(r=0;r<MAX_RADARS;r++){
	      for(c=0;c<MAX_CHANNELS;c++){
                  sprintf(shm_device,"/receiver_main_%d_%d_%d",r,c,0);
                  shm_unlink(shm_device);
                  shm_fd=shm_open(shm_device,O_RDWR|O_CREAT,S_IRUSR | S_IWUSR);
                  if (ftruncate(shm_fd, MAX_SAMPLES*4) == -1) fprintf(stderr,"ftruncate error\n");
                  summed_main_addresses[r][c][0]=mmap(0,MAX_SAMPLES*4,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
                  close(shm_fd);
                  sprintf(shm_device,"/receiver_back_%d_%d_%d",r,c,0);
                  shm_unlink(shm_device);
                  shm_fd=shm_open(shm_device,O_RDWR|O_CREAT,S_IRUSR | S_IWUSR);
                  ftruncate(shm_fd, MAX_SAMPLES*4);
                  summed_back_addresses[r][c][0]=mmap(0,MAX_SAMPLES*4,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
                  close(shm_fd);
                  for (i=0;i<MAX_SAMPLES;i++) {
                    summed_main_addresses[r][c][0][i]=i;	
		    summed_back_addresses[r][c][0][i]=i;	
                  } 
              }
            }
          } else {
            if (verbose > 1 ) printf("Setting up for Multi-site Radar\n");
            shm_memory=0;  //uses dma mapped memory not shm
	    for(r=0;r<MAX_RADARS;r++){
	      for(c=0;c<MAX_CHANNELS;c++){
                card=r;
                if (verbose > 0 ) printf("Get Buf Address card: %d  r:%d c:%d\n",card,r,c);
                for (i=0;i<MAX_INPUTS;i++) { 
  		  virtual_addresses[r][i][c][0]=
                    gc314GetBufferAddress(&physical_addresses[r][i][c][0],gc314fs[card],i,c);	
                }
              }
            } 
          }
        } else {
            if (verbose > 1 ) printf("Setting up for Testing\n");
	    //printf("Filling Test Data Arrays\n");
            shm_memory=1; // uses shm device nodes for test data
	    for(r=0;r<MAX_RADARS;r++){
	      for(c=0;c<MAX_CHANNELS;c++){
                  sprintf(shm_device,"/receiver_main_%d_%d_%d",r,c,0);
                  shm_unlink(shm_device);
                  shm_fd=shm_open(shm_device,O_RDWR|O_CREAT,S_IRUSR | S_IWUSR);
                  if (ftruncate(shm_fd, MAX_SAMPLES*4) == -1) fprintf(stderr,"ftruncate error\n");
                  main_test_data[r][c][0]=mmap(0,MAX_SAMPLES*4,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
                  close(shm_fd);
                  sprintf(shm_device,"/receiver_back_%d_%d_%d",r,c,0);
                  shm_unlink(shm_device);
                  shm_fd=shm_open(shm_device,O_RDWR|O_CREAT,S_IRUSR | S_IWUSR);
                  ftruncate(shm_fd, MAX_SAMPLES*4);
                  back_test_data[r][c][0]=mmap(0,MAX_SAMPLES*4,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
                  close(shm_fd);
                  sprintf(shm_device,"/receiver_aux_%d_%d_%d",r,c,0);
                  shm_fd=shm_open(shm_device,O_RDWR|O_CREAT,S_IRUSR | S_IWUSR);
                  ftruncate(shm_fd, MAX_SAMPLES*4);
                  aux_test_data[r][c][0]=mmap(0,MAX_SAMPLES*4,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
                  close(shm_fd);
                  for (i=0;i<MAX_SAMPLES;i++) {
                    main_test_data[r][c][0][i]=i;	
		    back_test_data[r][c][0][i]=i;	
		    aux_test_data[r][c][0][i]=i;	
                  } 
              }
            }
        }

        printf("Entering Main loop\n");
    // OPEN TCP SOCKET AND START ACCEPTING CONNECTIONS 
	//sock=tcpsocket(RECV_HOST_PORT);
	sock=server_unixsocket("rosrecv",0);
	listen(sock, 5);
	while (1) {
                fflush(stdout);
                fflush(stderr);
                rval=1;
		msgsock=accept(sock, 0, 0);
		if (verbose > 0) printf("accepting socket!!!!!\n");
		if( (msgsock==-1) ){
			perror("accept FAILED!");
			return EXIT_FAILURE;
		}
		else while (rval>=0){
                  /* Look for messages from external client process */
                  FD_ZERO(&rfds);
                  FD_SET(msgsock, &rfds); //Add msgsock to the read watch
                  FD_ZERO(&efds);
                  FD_SET(msgsock, &efds);  //Add msgsock to the exception watch
                  /* Wait up to five seconds. */
                  tv.tv_sec = 5;
                  tv.tv_usec = 0;
		  if (verbose > 2) printf("%d Entering Select\n",msgsock);
                  rval = select(msgsock+1, &rfds, NULL, &efds, &tv);
		  if (verbose > 2) printf("%d Leaving Select %d\n",msgsock,rval);
                  /* Don’t rely on the value of tv now! */
                  if (FD_ISSET(msgsock,&efds)){
                    if (verbose > 1) printf("Exception on msgsock %d ...closing\n",msgsock);
                    break;
                  }
                  if (rval == -1) perror("select()");
                  rval=recv(msgsock, &buf, sizeof(int), MSG_PEEK); 
                  if (verbose>2) printf("%d PEEK Recv Msg %d\n",msgsock,rval);
		  if (rval==0) {
                    if (verbose > 1) printf("Remote Msgsock %d client disconnected ...closing\n",msgsock);
                    break;
                  } 
		  if (rval<0) {
                    if (verbose > 0) printf("Msgsock %d Error ...closing\n",msgsock);
                    break;
                  } 
                  if ( FD_ISSET(msgsock,&rfds) && rval>0 ) {
                    if (verbose>2) printf("Data is ready to be read\n");
		    if (verbose > 2) printf("%d Recv Msg\n",msgsock);
                    rval=recv_data(msgsock,&msg,sizeof(struct DriverMsg));
                    datacode=msg.type;
		    if (verbose > 2) printf("\nmsg code is %c\n", datacode);
                    //system("date -t > /tmp/recv_cmd_time");
		    switch( datacode ){
                      case RECV_CtrlProg_END:
                        if (verbose > 1) printf("RECV driver: Closing a control program\n");
                        rval=recv_data(msgsock,&client,sizeof(struct ControlPRM));
                        r=client.radar-1;
                        c=client.channel-1;
                        if(IMAGING) {
                          for(card=0;card<MAX_CARDS;card++) gc314ChannelAbort(gc314fs[card], c);
                        } else {
                          card=r;
                          gc314ChannelAbort(gc314fs[card], c);
                        }
                        break;
                      case RECV_RXFE_SETTINGS:
                        if (verbose > 1) printf("RECV driver: Configuring for IF Mode\n");
                        rval=recv_data(msgsock,&ifmode,sizeof(ifmode)); 
                        if (verbose > 1) printf("RECV driver: IF Mode %d \n",ifmode);
                        rval=recv_data(msgsock,&rf_settings,sizeof(struct RXFESettings)); 
                        rval=recv_data(msgsock,&if_settings,sizeof(struct RXFESettings)); 
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
                       
                        break;


		      case RECV_CtrlProg_READY:
                        gettimeofday(&t0,NULL);
		        if (verbose > 1) printf("\nAsking to set up receiver for client that is ready\n");	
		        rval=recv_data(msgsock,&client,sizeof(struct ControlPRM));
                        r=client.radar-1; 
                        c=client.channel-1; 
                        if(ifmode) {
                          client.rfreq=(-SAMPLE_FREQ+IF_FREQ); 
                          if(verbose > 1 ) printf("Setting IF recv freq: %d %d %d\n",SAMPLE_FREQ,IF_FREQ,client.rfreq);  
                        }
                        if ((ready_index[r][c]>=0) && (ready_index[r][c] <maxclients) ) {
                          clients[ready_index[r][c]]=client;
                        } else {
                          clients[numclients]=client;
                          ready_index[r][c]=numclients;
                          numclients++;
                        }
			if (verbose > 1) printf("  Radar: %d, Channel: %d Beamnum: %d Status %d\n",
			  client.radar,client.channel,client.rbeam,msg.status);	
                        if (numclients >= maxclients) msg.status=-2;
		        if (verbose > 1) printf("\nclient ready done\n");	
                        numclients=numclients % maxclients;
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
                        gettimeofday(&t1,NULL);
                        elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
                        elapsed+=(t1.tv_usec-t0.tv_usec);
                        if (verbose > 1) printf("  Receiver Client Ready Elapsed Microseconds: %ld\n",elapsed);
                        if (verbose > 1)  printf("Ending Client Ready Setup\n");
                        break; 

		      case RECV_PRETRIGGER:
                        gettimeofday(&t0,NULL);
			if(verbose > 1 ) printf("Setup Receiver Card for next trigger\n");	
                        if (configured) {
                          if(IMAGING){ 
			    if(verbose > 0 ) printf("Set up card parameters for Imaging:\n");	
                            for(card=0;card<MAX_CARDS;card++) { 
                              gc314SetExternalTrigger(gc314fs[card], GC314_OFF);
			      for(i=0;i<numclients;i++){
                                r=clients[i].radar-1; 
                                c=clients[i].channel-1; 
                                b=0; 
			        if(verbose > 1 ) fprintf(stdout,"  Card %d client: %d r: %d c: %d\n",card,i,r,c);	
			        if(verbose > 1 ) fprintf(stdout,"    rfreq: %d filters %d %d\n", (int) clients[i].rfreq,(int) clients[i].filter_bandwidth,clients[i].match_filter);	
                                gc314SetFilters(gc314fs[card], (int) clients[i].filter_bandwidth, c, clients[i].match_filter);  
                                gc314SetOutputRate(gc314fs[card], (double) clients[i].baseband_samplerate, c);  
                                gc314SetFrequency(gc314fs[card], clients[i].rfreq*1000, c); 
                                gc314UpdateChannel(gc314fs[card], c); 
                                gc314SetSamples(gc314fs[card], clients[i].number_of_samples+RECV_SAMPLE_HEADER, c); 
                              } //end of client loop
                            }  // end of card loop 
                            //for(card=0;card<MAX_CARDS;card++) gc314SetSyncMask(gc314fs[card], SYNC1_SIA_ONETIME);
			    for(card=0;card<MAX_CARDS;card++) gc314SetSyncMask(gc314fs[card], SYNC1_SIA_HOLD);                          
                            gc314SetSync1(gc314fs[SYNC_MASTER], GC314_ON); 
			    for(card=0;card<MAX_CARDS;card++) gc314SetGlobalReset(gc314fs[card], GC314_ON);
			    for(card=0;card<MAX_CARDS;card++) gc314LoadGC4016s(gc314fs[card]);
			    for(card=0;card<MAX_CARDS;card++) gc314SetGlobalReset(gc314fs[card], GC314_OFF);
                            gc314SetSync1(gc314fs[SYNC_MASTER], GC314_OFF); 
                            for(card=0;card<MAX_CARDS;card++) {
			      for(i=0;i<numclients;i++){
                                r=clients[i].radar-1; 
                                c=clients[i].channel-1; 
                                b=0; 
                                gc314SetRDA(gc314fs[card], c); 
                              }
                            } 
                            //for(card=0;card<MAX_CARDS;card++) gc314SetSyncMask(gc314fs[card], SYNC1_SIA_ONETIME);
			    for(card=0;card<MAX_CARDS;card++) gc314SetSyncMask(gc314fs[card], SYNC1_SIA_HOLD);                          
                            for(card=0;card<MAX_CARDS;card++) {
			      for(i=0;i<numclients;i++){
                                r=clients[i].radar-1; 
                                c=clients[i].channel-1; 
                                b=0; 
                                gc314ChannelOn(gc314fs[card], c);
                              } 
                            }
                            for(card=0;card<MAX_CARDS;card++) {
                              gc314SetSyncMask(gc314fs[card], SYNC1_RTSC_CLEAR);
                              gc314StartCollection(gc314fs[card]);
                            }
                            gc314SetExternalTrigger(gc314fs[SYNC_MASTER], GC314_ON);
                          } else { 
                            for(card=0;card<MAX_CARDS;card++) 
                              gc314SetExternalTrigger(gc314fs[card], GC314_OFF);
			    for(i=0;i<numclients;i++){
                              r=clients[i].radar-1; 
                              c=clients[i].channel-1; 
                              card=r;
                              b=0; 
			      if(verbose > 1 ) fprintf(stdout,"  Non-Imaging client: %d r: %d c: %d\n",i,r,c);	
			      if(verbose > 1 ) fprintf(stdout,"    rfreq: %d filters %d %d\n", (int) clients[i].rfreq,(int) clients[i].filter_bandwidth,clients[i].match_filter);	
			      gc314SetFilters(gc314fs[card], (int) clients[i].filter_bandwidth, c, clients[i].match_filter);
			      gc314SetOutputRate(gc314fs[card], (double) clients[i].baseband_samplerate, c);
			      gc314SetFrequency(gc314fs[card], clients[i].rfreq*1000, c);
			      gc314UpdateChannel(gc314fs[card], c);
			      gc314SetSamples(gc314fs[card], clients[i].number_of_samples+RECV_SAMPLE_HEADER, c);
                            }
                            for(card=0;card<MAX_CARDS;card++) 
			      //gc314SetSyncMask(gc314fs[card], SYNC1_SIA_ONETIME);
                              gc314SetSyncMask(gc314fs[card], SYNC1_SIA_HOLD);                          
                            //for(card=0;card<MAX_CARDS;card++) 
                            //  gc314SetSync1(gc314fs[card], GC314_ON); 
                            for(card=0;card<MAX_CARDS;card++) 
			      gc314SetGlobalReset(gc314fs[card], GC314_ON);
                            for(card=0;card<MAX_CARDS;card++) 
			      gc314LoadGC4016s(gc314fs[card]);
                            for(card=0;card<MAX_CARDS;card++) 
			      gc314SetGlobalReset(gc314fs[card], GC314_OFF);
                            for(card=0;card<MAX_CARDS;card++) 
                              gc314SetSync1(gc314fs[card], GC314_OFF);
			    for(i=0;i<numclients;i++){
                              r=clients[i].radar-1; 
                              c=clients[i].channel-1; 
                              card=r;
                              b=0; 
                              gc314SetRDA(gc314fs[card], c); 
                            }                             
 
                            for(card=0;card<MAX_CARDS;card++) 
                              //gc314SetSyncMask(gc314fs[card], SYNC1_SIA_ONETIME);
                              gc314SetSyncMask(gc314fs[card], SYNC1_SIA_HOLD);
			    for(i=0;i<numclients;i++){
                              r=clients[i].radar-1; 
                              c=clients[i].channel-1; 
                              card=r;
                              b=0; 
                              gc314ChannelOn(gc314fs[card], c);
                            } 
                            for(card=0;card<MAX_CARDS;card++) {
                              //gc314SetSyncMask(gc314fs[card], SYNC1_RTSC_CLEAR);                            
                              gc314StartCollection(gc314fs[card]);
                            }
                            for(card=0;card<MAX_CARDS;card++) 
                              gc314SetExternalTrigger(gc314fs[card], GC314_ON);
			 }
			 if(verbose > 0 ) printf("Done with client parameters\n");	
                        } else {
                          //unconfigured
                          msg.status=0;
                        }
                        armed=1;
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
                        gettimeofday(&t1,NULL);
                        elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
                        elapsed+=(t1.tv_usec-t0.tv_usec);
                        if (verbose > 0) printf("  Receiver Pre-trigger Elapsed Microseconds: %ld\n",elapsed);
                        if (verbose > 1)  printf("Ending Pretrigger Setup\n");
                        gettimeofday(&tpre,NULL);
                        break; 

		      case RECV_POSTTRIGGER:
                        gettimeofday(&t0,NULL);
			if(verbose > 0 ) printf("Receiver post-trigger\n");	
                        armed=0;
                        numclients=0;
                        for (r=0;r<MAX_RADARS;r++){
                          for (c=0;c<MAX_CHANNELS;c++){
                            ready_index[r][c]=-1;
                          }
                        }
/*
                        for(card=0;card<MAX_CARDS;card++) {
                           if(verbose > 0 ) printf("Setup external trigger on card %d\n",card);
                           gc314SetExternalTrigger(gc314fs[card], GC314_OFF);
                        }
*/
                        msg.status=0;
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
                        gettimeofday(&t1,NULL);
                        elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
                        elapsed+=(t1.tv_usec-t0.tv_usec);
                        if (verbose > 0) printf("  Receiver Post-trigger Elapsed Microseconds: %ld\n",elapsed);
                        if (verbose > 1)  printf("Ending Post-trigger\n");
                        break;
//JDS : Pick up here
		      case RECV_GET_DATA:
                        gettimeofday(&t0,NULL);
			if(verbose > 1 ) printf("Receiver get data %d\n",configured);	
		        rval=recv_data(msgsock,&client,sizeof(struct ControlPRM));
                        r=client.radar-1; 
                        c=client.channel-1; 
			if(verbose > 1 ) printf("  radar: %d channel: %d\n",client.radar,client.channel);
                        b=0; 
			if(verbose > 1 ) printf("r: %d c: %d\n",r,c);	
                        if (configured) {
                          if (IMAGING==0) {
			    status=gc314WaitForData(gc314fs[r], c);
                          } else {
                             //imaging
                            status=0;
                            wait_status=0;
                            for(card=0;card<MAX_CARDS;card++) {
                              wait_status=gc314WaitForData(gc314fs[card], c);
                              if (verbose > 1 ) printf("%d ",wait_status);
                              if(wait_status!=0) status=wait_status;
                            }
                          }
 
                          gettimeofday(&t1,NULL);
                          elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
                          elapsed+=(t1.tv_usec-t0.tv_usec);
                          if (verbose > 1) printf("  Receiver Wait For Data Elapsed Microseconds: %ld\n",elapsed);
                          rval=send_data(msgsock,&status,sizeof(status));
			  if(verbose > 1 ) printf("  SHM Memory: %d\n",shm_memory);	
                          if (status==0) {
                            rval=send_data(msgsock,&shm_memory,sizeof(shm_memory));
			    if(verbose > 1 ) printf("  FRAME Offset: %d\n",RECV_SAMPLE_HEADER);	
                            temp=RECV_SAMPLE_HEADER;
                            rval=send_data(msgsock,&temp,sizeof(temp));
			    if(verbose > 1 ) printf("  DMA Buf: %d\n",b);	
                            rval=send_data(msgsock,&b,sizeof(b));
                            samples=client.number_of_samples;
                            rval=send_data(msgsock,&samples,sizeof(samples));
			    if(verbose > 1 ) printf("Sent Number of Samples %d\n",samples);	
                            if (IMAGING==0) {
                              main=(unsigned int *)virtual_addresses[r][main_input][c][b];
                              main_address=physical_addresses[r][main_input][c][b];
                              back_address=physical_addresses[r][back_input][c][b];
			      if(verbose > 1 ) printf("Send physical addresses%p %p\n",main_address,back_address);	
                              write_raw_files (client.tfreq, client.tbeam, samples+RECV_SAMPLE_HEADER,  r, c,  b);
			      rval=send_data(msgsock,&main_address,sizeof(main_address));
			      rval=send_data(msgsock,&back_address,sizeof(back_address));
                            } else {
			      if(verbose >1) printf("Send imaging shm addresses %p %p\n",
                                                summed_main_addresses[r][c][0],summed_back_addresses[r][c][0]);	
                              phasediff=add_phase(client.rfreq*1000, client.rbeam, samples+RECV_SAMPLE_HEADER, r, c, b,write_out);
                              write_raw_files (client.tfreq, client.tbeam, samples+RECV_SAMPLE_HEADER,  r, c,  b);
                              post_clr[c]=0;
                              rval=send_data(msgsock,&summed_main_addresses[r][c][b],sizeof(unsigned int));
                              rval=send_data(msgsock,&summed_back_addresses[r][c][b],sizeof(unsigned int));
                            }
                          } else {
                              //status non-zero
                          }    
                        } else {
                          //unconfigured
                          usleep(100000);
                          status=0;
                          rval=send_data(msgsock,&status,sizeof(status));
			  if(verbose > 1 ) printf("  SHM Memory: %d\n",shm_memory);	
                          rval=send_data(msgsock,&shm_memory,sizeof(shm_memory));
			  if(verbose > 1 ) printf("  FRAME Offset: %d\n",RECV_SAMPLE_HEADER);	
                          temp=RECV_SAMPLE_HEADER;
                          rval=send_data(msgsock,&temp,sizeof(temp));
			  if(verbose > 1 ) printf("  DMA Buf: %d\n",b);	
                          rval=send_data(msgsock,&b,sizeof(b));
                          samples=client.number_of_samples;
                          rval=send_data(msgsock,&samples,sizeof(samples));
			  if(verbose > 1 ) printf("Sent Number of Samples %d\n",samples);	
			  if(verbose >1) printf("Send test shm addresses %p %p\n",
                                            main_test_data[r][c][0],back_test_data[r][c][0]);	
			  rval=send_data(msgsock,&main_test_data[r][c][0],sizeof(unsigned int));
			  rval=send_data(msgsock,&back_test_data[r][c][0],sizeof(unsigned int));
                        }
                        msg.status=status;
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
                        gettimeofday(&t2,NULL);
                        gettimeofday(&tpost,NULL);
                        elapsed=(t2.tv_sec-t1.tv_sec)*1E6;
                        elapsed+=(t2.tv_usec-t1.tv_usec);
                        if (verbose > 1) printf("  Receiver Data Post Process Elapsed Microseconds: %ld\n",elapsed);
                        elapsed=(t2.tv_sec-t0.tv_sec)*1E6;
                        elapsed+=(t2.tv_usec-t0.tv_usec);
                        if (verbose > 1) printf("  Receiver Get Data Elapsed Microseconds: %ld\n",elapsed);
                        elapsed=(tpost.tv_sec-tpre.tv_sec)*1E6;
                        elapsed+=(tpost.tv_usec-tpre.tv_usec);
                        if (verbose > 1) printf("  Receiver PreTrig to Get Data finish Elapsed Microseconds: %ld\n",elapsed);
                        if (verbose > 1)  printf("Ending Get Data\n");
                        break;

                      case RECV_CLRFREQ:
                        if(verbose > 1 ) gettimeofday(&t0,NULL);
			rval=recv_data(msgsock,&clrfreq_parameters, sizeof(struct CLRFreqPRM));
                        rval=recv_data(msgsock,&client,sizeof(struct ControlPRM));
			if(verbose > 1 ) printf("Clear Frequency Search %d %d\n",
                                            client.radar-1,client.channel-1);	
                        nave=0;
                        centre=(clrfreq_parameters.end+clrfreq_parameters.start)/2;
                        usable_bandwidth=clrfreq_parameters.end-clrfreq_parameters.start;
			if(verbose > 1 ) printf("  requested values\n");	
			if(verbose > 1 ) printf("    start: %d\n",clrfreq_parameters.start);	
			if(verbose > 1 ) printf("    end: %d\n",clrfreq_parameters.end);	
			if(verbose > 1 ) printf("    centre: %d\n",centre);	
			if(verbose > 1 ) printf("    bandwidth: %lf in Khz\n",usable_bandwidth);	
			if(verbose > 1 ) printf("    nave:  %d %d\n",nave,clrfreq_parameters.nave);	
                        usable_bandwidth=floor(usable_bandwidth/2)*2 ;
/*
*  Set up fft variables
*/
                        N=(int)pow(2,ceil(log10(1.25*(float)usable_bandwidth)/log10(2)));
                        if(N>1024){
                          N=512;
                          usable_bandwidth=300;
                          start=(int)(centre-usable_bandwidth/2+0.49999);
                          end=(int)(centre+usable_bandwidth/2+0.49999);
                        }
/* 1 kHz fft boundaries*/
                        /* set up search parameters search_bandwidth > usable_bandwidth */
                        search_bandwidth=N;            
                        //search_bandwidth=800;            
                        start=(int)(centre-search_bandwidth/2.0+0.49999);
                        end=(int)(centre+search_bandwidth/2+0.49999);
                        unusable_sideband=(search_bandwidth-usable_bandwidth)/2;
                        clrfreq_parameters.start=start;
                        clrfreq_parameters.end=end;
			if(verbose > 1 ) printf("  search values\n");	
			if(verbose > 1 ) printf("  start: %d %d\n",start,clrfreq_parameters.start);	
			if(verbose > 1 ) printf("  end: %d %d\n",end,clrfreq_parameters.end);	
			if(verbose > 1 ) printf("  centre: %d\n",centre);	
			if(verbose > 1 ) printf("  search_bandwidth: %lf in Khz\n",search_bandwidth);	
			if(verbose > 1 ) printf("  usable_bandwidth: %d in Khz\n",usable_bandwidth);	
			if(verbose > 1 ) printf("  unusable_sideband: %lf in Khz\n",unusable_sideband);	
			if(verbose > 1 ) printf("  N: %d\n",N);	
  
			if(verbose > 1 ) printf("Malloc fftw_complex arrays %d\n",N);	
                        if(in!=NULL) free(in);
                        in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) *N);
                        if(out!=NULL) free(out);
                        out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
                        if(pwr!=NULL) free(pwr);
                        pwr = (double*) malloc(sizeof(double)*N);
                        if(pwr2!=NULL) free(pwr2);
                        pwr2 = (double*) malloc(sizeof(double)*N);
			if(verbose > 1 ) printf("Malloc fftw_complex arrays %p %p\n",in,out);	
			if(verbose > 1 ) printf("Build Plan\n");	
                        plan = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

                        for (i=0;i<N;i++) {
                          pwr[i]=0;
                        }

                       msg.status=1;
                       if (clrfreq_parameters.nave > 10 ) clrfreq_parameters.nave=10;
                       nave=clrfreq_parameters.nave;
                       ii=0;
#ifdef __QNX__
                         if(!armed){
		           if(verbose > 1 ) printf("Nave %d %d\n",nave, clrfreq_parameters.nave);	
                           r=client.radar-1;
                           c=client.channel-1;
                           c=0;
                           b=0; 
                           for(try=0;try<max_retries;try++) {
                             //fprintf(stderr,"Trying CLR %d\n",try);
                             //fflush(stderr);
                             if(IMAGING){  
                                for(card=0;card<MAX_CARDS;card++) { 
                                  gc314SetFilters(gc314fs[card], (int) search_bandwidth*1000, c,0); 
                                  gc314SetOutputRate(gc314fs[card], (double) search_bandwidth*1000, c);  
                                  gc314SetFrequency(gc314fs[card], centre*1000, c); 
                                  gc314UpdateChannel(gc314fs[card], c); 
                                  gc314SetSamples(gc314fs[card], clrfreq_parameters.nave*N+RECV_SAMPLE_HEADER+CLR_SAMP_OFFSET, c);
			          gc314SetExternalTrigger(gc314fs[card], GC314_OFF);
                                }
                                //for(card=0;card<MAX_CARDS;card++) gc314SetSyncMask(gc314fs[card], SYNC1_SIA_ONETIME);
                                for(card=0;card<MAX_CARDS;card++) gc314SetSyncMask(gc314fs[card], SYNC1_SIA_HOLD);
                                gc314SetSync1(gc314fs[SYNC_MASTER], GC314_ON);
                                for(card=0;card<MAX_CARDS;card++) gc314SetGlobalReset(gc314fs[card], GC314_ON);
                                for(card=0;card<MAX_CARDS;card++) gc314LoadGC4016s(gc314fs[card]);
                                for(card=0;card<MAX_CARDS;card++) gc314SetGlobalReset(gc314fs[card], GC314_OFF);
                                gc314SetSync1(gc314fs[SYNC_MASTER], GC314_OFF);
                                for(card=0;card<MAX_CARDS;card++) gc314SetRDA(gc314fs[card], c); 
                                //for(card=0;card<MAX_CARDS;card++) gc314SetSyncMask(gc314fs[card], SYNC1_SIA_ONETIME);
                                for(card=0;card<MAX_CARDS;card++) gc314SetSyncMask(gc314fs[card], SYNC1_SIA_HOLD);
                                for(card=0;card<MAX_CARDS;card++) gc314ChannelOn(gc314fs[card], c);
                                for(card=0;card<MAX_CARDS;card++) gc314StartCollection(gc314fs[card]);
                                //trigger via Sync1
                                gc314SetSync1(gc314fs[SYNC_MASTER], GC314_ON);
                                usleep(10);
                                gc314SetSync1(gc314fs[SYNC_MASTER], GC314_OFF);
                             } else {
			        if(verbose > 1 ) printf("Non-Imaging CLRSearch\n");
                                //non-imaging
                                card=r;
			        gc314SetExternalTrigger(gc314fs[card], GC314_OFF);
			        gc314SetFilters(gc314fs[card], (int) search_bandwidth*1000, c, 0); //no match filter
			        gc314SetOutputRate(gc314fs[card], (double) search_bandwidth*1000, c);
			        gc314SetFrequency(gc314fs[card], centre*1000, c);
			        gc314UpdateChannel(gc314fs[card], c);
			        gc314SetSamples(gc314fs[card], clrfreq_parameters.nave*N+RECV_SAMPLE_HEADER+CLR_SAMP_OFFSET, c);
                                gc314SetSyncMask(gc314fs[card], SYNC1_SIA_HOLD);                          
			        //gc314SetSyncMask(gc314fs[card], SYNC1_SIA_ONETIME);
			        gc314SetSync1(gc314fs[card], GC314_ON);
			        gc314SetGlobalReset(gc314fs[card], GC314_ON);
			        gc314LoadGC4016s(gc314fs[card]);
			        gc314SetGlobalReset(gc314fs[card], GC314_OFF);
			        gc314SetSync1(gc314fs[card], GC314_OFF);
                                usleep(10);
                                gc314SetRDA(gc314fs[card], c); 
                                usleep(10);
                                //gc314SetSyncMask(gc314fs[card], SYNC1_SIA_ONETIME);
                                gc314SetSyncMask(gc314fs[card], SYNC1_SIA_HOLD);                          
                                gc314ChannelOn(gc314fs[card], c);
                                gc314StartCollection(gc314fs[card]);
                                //trigger via Sync1
                                usleep(10);
                                gc314SetSync1(gc314fs[card], GC314_ON);
                                usleep(10);
                                gc314SetSync1(gc314fs[card], GC314_OFF);
                             } 

                             if(IMAGING) {
                               wait_status=0;
                               status=0;
                               for(card=0;card<MAX_CARDS;card++) {
                                 wait_status=gc314WaitForData(gc314fs[card], c);
                                 if (wait_status!=0) status=wait_status;
                               }
                             } else {
			       if(verbose > 1 ) printf("Non-Imaging CLRSearch Wait for Data\n");
                               card=r;
                               status=gc314WaitForData(gc314fs[card], c);
                             }

			     if(verbose > 1 ) printf("CLRSearch: Status :: %d\n",status);
                             if (status<0) {
			       if(verbose > 1 ){
                                 fprintf(stderr,"ReTrying CLR\n");
                                 fflush(stderr);
                               }
                               continue; 
                             } else {
                               break;
                             }
                           }  // retry loop  
                           if (status>=0) {
			       if(verbose > 1 ){
                                 fprintf(stderr,"Good CLR Tries: %d\n",try);
                                 fflush(stderr);
                               }
                               if(IMAGING) {
                                 phasediff=add_phase(centre*1000, client.rbeam, clrfreq_parameters.nave*N+RECV_SAMPLE_HEADER+CLR_SAMP_OFFSET, r, c, b,write_clr_out);
                                 main=summed_main_addresses[r][c][b];
                               } else {
                                 main=(unsigned int *)virtual_addresses[card][main_input][c][b];
                               }
 
                           } else {
			       if(verbose > 1 ){
                                 fprintf(stderr,"Bad CLR Tries: %d\n",try);
                                 fflush(stderr);
                               }
                              msg.status=-1;
                              if (verbose > -10 ) {
                                printf("  Reciever:: CLRFREQ: Bad Data: %d %d\n",ii,status);
                              }    
                           } 
                         } else {
                            //if armed set an error condition
                            msg.status=-1;
                         }
#endif
                         //printf("  Reciever:: CLRFREQ: averaging msg_status: %d\n",msg.status);
                         for(ii=0;ii<nave;ii++) {
                           for (j=0;j<N;j++) {
#ifdef __QNX__
                             if (msg.status) {
                               if(REVERSE_IQ_ORDER) {
                                  Q=(main[ii*N+j+RECV_SAMPLE_HEADER+CLR_SAMP_OFFSET] & 0xffff0000) >> 16;
                                  I= main[ii*N+j+RECV_SAMPLE_HEADER+CLR_SAMP_OFFSET] & 0x0000ffff;
                               } else {
                                  I=(main[ii*N+j+RECV_SAMPLE_HEADER+CLR_SAMP_OFFSET] & 0xffff0000) >> 16;
                                  Q= main[ii*N+j+RECV_SAMPLE_HEADER+CLR_SAMP_OFFSET] & 0x0000ffff;
                               }
                               in[j][0]=Q;
                               in[j][1]=I;
                             } else {
			     if(verbose > -1 ) printf("Bad Data Fill %d %d\n",ii,j);
                               in[j][0]=0;
                               in[j][1]=-1;
                             }
#else
  		             in[j][0]=1;	
  		             in[j][1]=0;	
#endif
                           }
                           fftw_execute(plan);
/* calculate power from output of fft */
                           for (j=0;j<N;j++) {
                             pwr[j]+=((out[j][0]*out[j][0])+(out[j][1]*out[j][1]))/(double)(N*N);
                           }
			   if(verbose > 10 ) {  printf("FFT Out/pwr\n");	
                             for (j=0;j<N;j++) {
                               printf("%d :: Out: %lf %lf  pwr: %lf\n",j,out[j][0],out[j][1],pwr[j]);
                             }
                           }
                         } //end of nave loop

/* take average power for nave number of calculations */
                       if(verbose > 1 ) printf("Average pwr\n");	
                       if (nave > 0 ) for(i=0;i<N;i++) pwr[i]=pwr[i]/(nave);
  /* Re-arrange the output of the fft to go from start to end (fftshift).
     This centers the fft, so now the first element in pwr2 corresponds
     with the start freq, and goes up to the end freq
  */
                       if(verbose > 1 ) printf("Reorder pwr\n");	
                       for(i=0;i<(N/2);i++){
                          pwr2[N/2+i]=pwr[i];
                          pwr2[i]=pwr[N/2+i];
                       }
                       if(verbose > 4 ) {
                          printf("fft power\n");	
                          printf(": Index : Freq : Shifted\n");	
                          if(write_clr_file) 
                            clr_data=fopen("/tmp/clr_data.txt","a+");  
                          for(i=0;i<N;i++){
                            printf("%4d: %8d %8.3lf\n",i,start+i,pwr2[i]);
                            if(write_clr_file)
                              fprintf(clr_data,"%4d %8d %8.3lf\n",i,start+i,pwr2[i]);
                          }
                          fclose(clr_data);  
                       }
                       if(pwr!=NULL) free(pwr);
                       pwr=NULL; 
                       /* Lets shave off the unusable_sideband and just send over the * 
                        *   the usable_bandwidth centered of the centre frequency     */
                       pwr=&pwr2[(int)unusable_sideband]; 
                       if(verbose > 0 ) printf("Send clrfreq data back\n");	
                       rval=send_data(msgsock, &clrfreq_parameters, sizeof(struct CLRFreqPRM));
                       rval=send_data(msgsock, &usable_bandwidth, sizeof(int));
                       if(verbose > 1 ) printf("  final values\n");
                       if(verbose > 1 ) printf("  start: %d\n",clrfreq_parameters.start);
                       if(verbose > 1 ) printf("  end: %d\n",clrfreq_parameters.end);
                       if(verbose > 1 ) printf("  nave: %d\n",clrfreq_parameters.nave);
                       if(verbose > 1 ) printf("  usable_bandwidth: %d\n",usable_bandwidth);
                       post_clr[0]=1;
                       post_clr[1]=1;
                       post_clr[2]=1;
                       post_clr[3]=1;
                       rval=send_data(msgsock, pwr, sizeof(double)*usable_bandwidth);  //freq order power
                       rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
                       fftw_destroy_plan(plan);
                       fftw_free(in); 
                       fftw_free(out);
                       in=NULL;
                       out=NULL;
                       pwr=NULL;
                       if(pwr2!=NULL) free(pwr2);
                       pwr2=NULL;  
                       if (verbose > 1) { 
                          gettimeofday(&t4,NULL);
                          elapsed=(t4.tv_sec-t0.tv_sec)*1E6;
                          elapsed+=(t4.tv_usec-t0.tv_usec);
                          printf("  Receiver: ClrFreq Search Elapsed Microseconds: %ld\n",elapsed);
                       }
            
                        break;                      

		      default:
			if (verbose > 0) fprintf(stderr,"BAD CODE: %c : %d\n",datacode,datacode);
                        msg.status=0;
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
			break;
		    }
		  }	
		} 
		if (verbose > 0 ) fprintf(stderr,"Closing socket\n");
		close(msgsock);
	};
        fftw_destroy_plan(plan);
        fftw_free(in); 
        fftw_free(out);

        return 1;
}






