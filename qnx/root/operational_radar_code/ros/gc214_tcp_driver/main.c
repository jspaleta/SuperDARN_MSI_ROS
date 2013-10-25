#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
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
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/types.h>
#include <sys/socket.h>
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
#include "gc214_defines.h"
#include "gc4016.h"
#include "setcard.h"
#include "rtypes.h"
#define IMAGING 0
#define BUFS 1
#define DMA_BUF_SIZE 32768L
#define MAX_SAMPLES 262144 
#define MAIN 0
#define BACK 1

int verbose=0;


int sock,msgsock;
unsigned int	 BASE1[MAX_CARDS], BASE2[MAX_CARDS],BASE3[MAX_CARDS];
//MSI RADAR VARIABLES
unsigned int     virtual_addr[MAX_RADARS][MAX_CHANNELS][2][BUFS],physical_addr[MAX_RADARS][MAX_CHANNELS][2][BUFS];
unsigned int     main_virtual[MAX_RADARS][MAX_CHANNELS][BUFS],back_virtual[MAX_RADARS][MAX_CHANNELS][BUFS];
unsigned int     main_physical[MAX_RADARS][MAX_CHANNELS][BUFS],back_physical[MAX_RADARS][MAX_CHANNELS][BUFS];
unsigned int     *main_test_data[MAX_RADARS][MAX_CHANNELS][BUFS],*back_test_data[MAX_RADARS][MAX_CHANNELS][BUFS],*aux_test_data[MAX_RADARS][MAX_CHANNELS][BUFS];
unsigned int     *shm_main_addresses[MAX_RADARS][MAX_CHANNELS][BUFS];
unsigned int     *shm_back_addresses[MAX_RADARS][MAX_CHANNELS][BUFS];

struct GC4016Global global;
struct GC4016Channel chana,chanb,chanc,chand;
int pci_index[MAX_RADARS];
short   resampcoeffs[256];
int     resampratios[4];
/* according to GC4-16 data sheet, pp 57 & 58; cfir_34 & pfir_34 */
  int cfircoeffsAB[11]={ -24, 74, 494, 548, -977, -3416, -3672, 1525, 13074, 26547, 32767};
  int cfircoeffsCD[11]={ -24, 74, 494, 548, -977, -3416, -3672, 1525, 13074, 26547, 32767};
  int pfircoeffsAB[32]={ 14, 30, 41, 27, -29, -118, -200, -212, -95, 150,
                     435, 598, 475, 5, -680, -1256, -1330, -653, 669,
                     2112, 2880, 2269, 101, -2996, -5632, 6103,
                     -3091, 3666, 13042, 22747, 30053, 32767};
  int pfircoeffsCD[32]={ 14, 30, 41, 27, -29, -118, -200, -212, -95, 150,
                     435, 598, 475, 5, -680, -1256, -1330, -653, 669,
                     2112, 2880, 2269, 101, -2996, -5632, 6103,
                     -3091, 3666, 13042, 22747, 30053, 32767};
long baseGC214;
int fclrflag=0;
uint32 ifmode=IF_ENABLED;
int32 status;

int armed=0,configured=0;
int CH1_ENABLED=0 , CH2_ENABLED=0;
int scanstatus=-1;
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


int wait_thread() { 
  int r,r1,r2;
  double elapsed=0.0;
  struct timespec start;
  struct timespec stop;

  while (1) {    
    if ((CH1_ENABLED || CH2_ENABLED) && configured) {
#ifdef __QNX__ 
      clock_gettime(CLOCK_REALTIME,&start);
      clock_gettime(CLOCK_REALTIME,&stop);
#endif  
      while (CH1_ENABLED || CH2_ENABLED) {    
    
    /* Check to see if at least one frame has been collected, if so, make sure there was no FIFO overrun, and exit */
        if(CH1_ENABLED) r1=pollGC214_CH1ID(baseGC214);
        else r1=100;
        if(CH2_ENABLED) r2=pollGC214_CH3ID(baseGC214);
        else r2=100;
        if ((r1 >= 1) && (r2 >= 1) ){
          elapsed=((stop.tv_sec-start.tv_sec)*1E9+(stop.tv_nsec-start.tv_nsec))/1.0E9;
          if (verbose > 1 ) printf("  Data Wait Elasped: %lf\n",elapsed);
          stopGC214(baseGC214);
          scanstatus=SCAN_OK;
          CH1_ENABLED=0;
          CH2_ENABLED=0;
          r=pollGC214FIFO(baseGC214);
          if ((r & 0x0000000f)>0){
            fprintf(stderr,"FIFO Overflow: \n");
            scanstatus=SCAN_OVERFLOW;
            CH1_ENABLED=0;
            CH2_ENABLED=0;
          }
          if ((r & 0x00000010)>0){
            if(fclrflag) fprintf(stderr,"FCLR CH1 A/D Over-range: \n");
            //return SCAN_OVERFLOW;
            scanstatus=SCAN_OK;
            CH1_ENABLED=0;
            CH2_ENABLED=0;
          }
          if ((r & 0x00000020)>0){
            if(fclrflag) fprintf(stderr,"FCLR CH2 A/D Over-range: \n");
            //return SCAN_OVERFLOW;
            scanstatus=SCAN_OK;
            CH1_ENABLED=0;
            CH2_ENABLED=0;
          }
          else {
            scanstatus=SCAN_OK;
            CH1_ENABLED=0;
            CH2_ENABLED=0;
          } 
        }
    /* Check to see if this is taking too long (this can be due to too much data being requested, or to a lack of trigge
r */
#ifdef __QNX__
        clock_gettime(CLOCK_REALTIME,&stop);
        if ((stop.tv_sec-start.tv_sec)>2){
          stopGC214(baseGC214);
          fprintf(stderr,"Did not collect data in alloted time: \n");
          scanstatus=SCAN_ERROR;
          CH1_ENABLED=0;
          CH2_ENABLED=0;
        }
        delay(1);
#else
        usleep(1000);
#endif
      } //end while enabled
    } else {
      usleep(1000);
    } //end if enabled
  } //while 1
}

int main(int argc, char **argv){
	// socket and message passing variables
        int smpdly,smpdlyAB,smpdlyCD,chanABready,chanCDready;
        struct  timeval tv;
	char	datacode;
	int	rval;
        fd_set rfds,efds;
        int status;
        short I,Q;
        long long P;
	// counter and temporary variables
	int32	temp,tmp,tmp32,buf,r,c,cc,x,i,ii,j,n,b,N;
        unsigned int utemp;
        struct  DriverMsg msg;
        struct RXFESettings rf_settings,if_settings;
        struct CLRFreqPRM clrfreq_parameters;
        unsigned int *main, *back;
        int  maxclients=MAX_RADARS*MAX_CHANNELS+1;
        int numclients=0;
        int ready_index[MAX_RADARS][MAX_CHANNELS];
        struct  ControlPRM  clients[maxclients],client;
        struct timeval tpre,tpost,t0,t1,t2,t3,t4,t5;
        unsigned long elapsed;
        int card=0;
        char shm_device[80];
        int shm_fd;
        int32 shm_memory=0;
        unsigned int main_offset,back_offset;

        unsigned int CLOCK_RES;
        int pci_handle, pci_handle_dio,IRQ;
        fftw_complex *in=NULL, *out=NULL;
        double *pwr=NULL,*pwr2=NULL;
        fftw_plan plan;
        int32 usable_bandwidth; 
        double search_bandwidth,unusable_sideband;
        int centre,start,end;
#ifdef __QNX__
	struct	 _clockperiod 	new, old;
#endif
        float freqout=3333.33;
        float freqoutCD=50000.0;
        int tfreq=0;
        int Fsamp=FCLOCK;
        int32 samples=300;
        int metsample=3000;
        uint64 main_address,back_address;
        double power;
        FILE *fp;
        signal(SIGINT, graceful_cleanup);
        for (r=0;r<MAX_RADARS;r++){
          pci_index[r]=GC214_PCI_INDEX;  //JDS:  This may need to change based on computer hardware, timing card uses 9080 as well
          for (c=0;c<MAX_CHANNELS;c++){
            ready_index[r][c]=-1;
          }
        }
#ifdef __QNX__
   /* SET UP COMMUNICATION TO GC314 DRIVERS */
        configured=1;
        if(MAX_CHANNELS > 2) {
	    fprintf(stderr, "Too many channels requested for gc214 reciever: maximum of 2 channels allowed\n");
        }
        if(IMAGING==1) {
          if (MAX_CARDS < (MAX_TRANSMITTERS+MAX_BACK_ARRAY+1)) {
	    fprintf(stderr, "Too few cards configured for imaging radar configuration\n");
            configured=0;
          }
        } else {
          if (MAX_CARDS <  MAX_RADARS) {
	    fprintf(stderr, "One Card per radar needed\n");
            configured=0;
          }
        }
// TODO: JDS initial setup of gc214 cards here if needed
#endif
#ifdef __QNX__
    /* SET THE SYSTEM CLOCK RESOLUTION AND GET THE START TIME OF THIS PROCESS */
	// set the system clock resolution to 10 us
	new.nsec=10000;
	new.fract=0;
	temp=ClockPeriod(CLOCK_REALTIME,&new,0,0);
	if(temp==-1) 	perror("Unable to change system clock resolution");
	temp=ClockPeriod(CLOCK_REALTIME,0,&old,0);
	if(temp==-1) 	perror("Unable to read sytem time");
	CLOCK_RES=old.nsec;
#endif
//TODO: JDS GC214 Configuration Here
    /* OPEN THE PLX9080 AND GET LOCAL BASE ADDRESSES */
        if (configured) {
          if (verbose > 0 ) printf("Opening PLX9080 on each card\n");
#ifdef __QNX__
	  for (i=0;i<MAX_CARDS;i++){
      /* BASE1 mmap'd device pointer to PCI 256 kb io/config space */
      /* BASE2 mmap'd device pointer to GC214 register space */
      /* BASE3 non-mmap'd device address to register space */
            if ((temp =
             initPCI9080(&IRQ, &BASE1[i], &BASE2[i], &BASE3[i], pci_index[i],1)) == -1) {
              perror("Unable to initialize PCI Bus");
              exit(-1);
            }
            fflush(stderr);
	    if(temp==-1)	 fprintf(stderr, "PLX9080 configuration failed: Card %d\n", i);
 	    else 		 fprintf(stderr, "PLX9080 configuration successful: Card %d\n", i);
	  }
#endif
          if(IMAGING==1) {
          } else {
//TODO: JDS GC214 setup here only 4 buffers per card in total
            /* CREATE DMA BUFFERS FOR ALL RECIEVER CHANNELS */
#ifdef __QNX__
            for (cc = 0; cc < MAX_CHANNELS; cc++) {
              for ( i = 0;  i< MAX_RADARS; i++) {
                for ( b = 0;  b< BUFS; b++) {
                   c=cc % 2;  // Force channel number below 2
                  temp =
                   _create_DMA_buff(&virtual_addr[i][c][MAIN][b], &physical_addr[i][c][MAIN][b],
                             DMA_BUF_SIZE * 16);
                   _create_DMA_buff(&virtual_addr[i][c][BACK][b], &physical_addr[i][c][BACK][b],
                             DMA_BUF_SIZE * 16);
                  if (temp == -1) {
                    fprintf(stderr, "ERROR MAKING DMA BUFFERS! %d %d\n", i,cc);
                    break;
                  }
                }
              }
            }
#endif
	    pthread_create(NULL,NULL, wait_thread, NULL);	

            if (verbose > 0 ) printf("Setting up for Multi-site Radar\n");
            shm_memory=1;
	    for(r=0;r<MAX_RADARS;r++){
	      for(cc=0;cc<MAX_CHANNELS;cc++){
                for ( b = 0;  b< BUFS; b++) {
                  c=cc % 2; // Force channel number below 2 
                  sprintf(shm_device,"/receiver_main_%d_%d_%d",r,c,b);
                  shm_unlink(shm_device);
                  shm_fd=shm_open(shm_device,O_RDWR|O_CREAT,S_IRUSR | S_IWUSR);
                  if (ftruncate(shm_fd, MAX_SAMPLES*4) == -1) fprintf(stderr,"ftruncate error\n");
                  shm_main_addresses[r][c][b]=mmap(0,MAX_SAMPLES*4,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0); 
                  printf("main: %s  :: %p\n",shm_device,shm_main_addresses[r][c][b]);
                  close(shm_fd);
                  sprintf(shm_device,"/receiver_back_%d_%d_%d",r,c,b);
                  
                  shm_unlink(shm_device);
                  shm_fd=shm_open(shm_device,O_RDWR|O_CREAT,S_IRUSR | S_IWUSR);                                                                        
                  ftruncate(shm_fd, MAX_SAMPLES*4);                                                                                                    
                  shm_back_addresses[r][c][b]=mmap(0,MAX_SAMPLES*4,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);                                       
                  close(shm_fd);                          
                  for (i=0;i<MAX_SAMPLES;i++) {                                                                                                        
                    shm_main_addresses[r][c][b][i]=i;                                                                                               
                    shm_back_addresses[r][c][b][i]=i;                                                                                               
                  }             
                  printf("Virt: %p Physical: %p\n",virtual_addr[r][c][MAIN][b],physical_addr[r][c][MAIN][b]);
  		  main_virtual[r][cc][b]=virtual_addr[r][c][MAIN][b];	
  		  main_physical[r][cc][b]=physical_addr[r][c][MAIN][b];	
		  back_virtual[r][cc][b]=virtual_addr[r][c][BACK][b];	
		  back_physical[r][cc][b]=physical_addr[r][c][BACK][b];	
                }
              }
            } 
          }
        } else {
	    printf("Filling Test Data Arrays\n");
            shm_memory=1;
	    for(r=0;r<MAX_RADARS;r++){
	      for(c=0;c<MAX_CHANNELS;c++){
                  sprintf(shm_device,"/receiver_main_%d_%d_%d",r,c,0);
                  printf("%s ",shm_device);
                  shm_unlink(shm_device);
                  shm_fd=shm_open(shm_device,O_RDWR|O_CREAT,S_IRUSR | S_IWUSR);
                  if (shm_fd == -1) printf("shm_open error\n");
                  printf("%d \n",shm_fd);
                  if (ftruncate(shm_fd, MAX_SAMPLES*4) == -1) printf("ftruncate error\n");

                  printf("%d \n",shm_fd);
                  main_test_data[r][c][0]=mmap(0,MAX_SAMPLES*4,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
                  close(shm_fd);
                  sprintf(shm_device,"/receiver_back_%d_%d_%d",r,c,0);
                  printf("%s ",shm_device);
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

	for(r=0;r<MAX_RADARS;r++){
          buildGC4016Global(&global);
          buildGC4016Channel(&chana);
          buildGC4016Channel(&chanb);
          buildGC4016Channel(&chanc);
          buildGC4016Channel(&chand);
          setcard(&global,&chana,&chanb,&chanc,&chand,Fsamp,&smpdlyAB,&smpdlyCD,cfircoeffsAB,cfircoeffsCD,pfircoeffsAB,pfircoeffsCD,resampcoeffs);

        }
        for (i=0;i<256;i++)  resampcoeffs[i]=0;
        resampcoeffs[0]=1024;

//JDS :: TEST CODE
        if(configured) {
          fp=fopen("test.out","w");
//          matched=1;
          samples=1000;
          metsample=1000;
          printf(" Samples: %d  MetSamples %d\n",samples,metsample);
          r=0;  
          c=0;
          b=0;
          for(tfreq=10000;tfreq<=10000;tfreq+=100){
            fprintf(fp,"freq : %d\n",tfreq);
            freqout=33333.33;
            //freqout=512*1000;
            chana.rate=freqout;
            chanb.rate=freqout;
            chanc.rate=freqout;
            chand.rate=freqout;
            chana.match=1;
            chanb.match=1;
            chanc.match=1;
            chand.match=1;
            chana.samples=samples;
            chanb.samples=samples;
            chanc.samples=samples;
            chand.samples=samples;
            chana.freq= pow( 2, 32)* 
              ( (double) ( (tfreq)*1000))/ ( (double) Fsamp);
            chanb.freq= chana.freq;
            chanc.freq= pow( 2, 32)* 
              ( (double) ( (tfreq+1000)*1000))/ ( (double) Fsamp);
            chand.freq= chanc.freq;
            printf(" A::  Samples: %d  Dly: %d B:: Samples %d Dly: %d\n",chana.samples,smpdlyAB,chanb.samples,smpdlyAB);
            printf(" C::  Samples: %d  Dly: %d D:: Samples %d Dly: %d\n",chanc.samples,smpdlyCD,chand.samples,smpdlyCD);
            setcard(&global,&chana,&chanb,&chanc,&chand,Fsamp,&smpdlyAB,&smpdlyCD,cfircoeffsAB,cfircoeffsCD,pfircoeffsAB,pfircoeffsCD,resampcoeffs);

            printf(" A::  Samples: %d  Dly: %d B:: Samples %d Dly: %d\n",chana.samples,smpdlyAB,chanb.samples,smpdlyAB);
            printf(" C::  Samples: %d  Dly: %d D:: Samples %d Dly: %d\n",chanc.samples,smpdlyAB,chand.samples,smpdlyCD);
            baseGC214=BASE2[r];
            setupGC214(baseGC214,chana.samples+smpdlyAB,chanc.samples+smpdlyCD);
            setupGC4016(baseGC214,&global, resampcoeffs);
            setupGC4016channel(baseGC214,0,&chana,cfircoeffsAB,pfircoeffsAB);
            setupGC4016channel(baseGC214,1,&chanb,cfircoeffsAB,pfircoeffsAB);
            setupGC4016channel(baseGC214,2,&chanc,cfircoeffsCD,pfircoeffsCD);
            setupGC4016channel(baseGC214,3,&chand,cfircoeffsCD,pfircoeffsCD);
            releaseGC4016(baseGC214);
            enableGC214(baseGC214);
            printf(" Getting ready to software trigger %d %d\n",CH1_ENABLED,CH2_ENABLED);
            CH1_ENABLED=1;
            CH2_ENABLED=1;
            triggerGC214(baseGC214);
            while(CH1_ENABLED || CH2_ENABLED); //wait for data
            switch(scanstatus) {
              case SCAN_OK: 
                printf("  Scan Status: OK %d\n",scanstatus);
                break;
              default:
                printf("  Scan Status: Problem %d\n",scanstatus);
                break;
            }
            power=0;
            for(c=0;c<2;c++) {
              fprintf(fp,"c: %d\n",c);
              main=shm_main_addresses[r][c][b];                                                     
              back=shm_back_addresses[r][c][b];
              if(c==0) {                                                                      
                main_offset=0x00000;                                                          
                back_offset=0x20000;                                                          
                smpdly=smpdlyAB;
              } else if (c==1) {
                main_offset=0x40000;                                                          
                back_offset=0x60000;                                                          
                smpdly=smpdlyCD;
              } else {
                main_offset=0x40000;                                                          
                back_offset=0x60000;                                                          
                smpdly=smpdlyCD;                                                              
              }
              //for(x=0;x<smpdly;x++) tmp=read32(baseGC214+main_offset);
              for(x=0;x<smpdly+samples;x++) {                                                         
                tmp=read32(baseGC214+main_offset);                                            
                main[x]=tmp;                                                                  
              }                                                                               
              //for(x=0;x<smpdly;x++) tmp=read32(baseGC214+back_offset);
              for(x=0;x<smpdly+samples;x++) {                                                         
                tmp=read32(baseGC214+back_offset);                                            
                back[x]=tmp;                                                                  
              }
              for(x=0;x<smpdly+samples;x++){
                I=main[x]&0x0000ffff;
                Q=main[x]>>16;
                power=(double)((double)I*(double)I+(double)Q*(double)Q);
                power=10*log10(power); 
                //printf("%8d :: %8d %8d %8.3lf :: \n",x,I,Q,power);
                fprintf(fp,"%8d :: %8d %8d %8.3lf :: ",x,I,Q,(double)power);
                I=back[x]&0x0000ffff;
                Q=back[x]>>16;
                power=(double)((double)I*(double)I+(double)Q*(double)Q);
                power=10*log10(power); 
                fprintf(fp,"%8d %8d %8.3lf\n",I,Q,(double)power);
              }
              power=power/samples;
              power=power/(32768*32768);
              power=10*log10(power);
              fflush(fp);
            }
          }
          fclose(fp);
        }




    // OPEN TCP SOCKET AND START ACCEPTING CONNECTIONS 
        //sock=tcpsocket(RECV_HOST_PORT);
        sock=server_unixsocket("rosrecv",0);

	listen(sock, 5);
	while (1) {
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
                  rval = select(msgsock+1, &rfds, NULL, &efds, NULL);
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

		    switch( datacode ){
                      case RECV_CtrlProg_END:
                        if (verbose > 1) printf("RECV driver: Closing a control program\n");
                        rval=recv_data(msgsock,&client,sizeof(struct ControlPRM));
                        r=client.radar-1;
                        c=client.channel-1;
// JDS: Added for consistency FIX reciever_handler code for this
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
//  Code Merge Review Needed
//                        if(IMAGING) {
//                          for(card=0;card<MAX_CARDS;card++) gc314ChannelAbort(gc314fs[card], c);
//                        } else {
//                          card=r;
//                          gc314ChannelAbort(gc314fs[card], c);
//                        }
                        break;
                      case RECV_RXFE_SETTINGS:
                        if (verbose > -1) printf("RECV driver: Configuring for IF Mode\n");
                        rval=recv_data(msgsock,&ifmode,sizeof(ifmode)); 
                        if (verbose > -1) printf("RECV driver: IF Mode %d \n",ifmode);
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
                        if ((ready_index[r][c]>=0) && (ready_index[r][c] <maxclients) ) {
                          clients[ready_index[r][c]]=client;
                        } else {
                          clients[numclients]=client;
                          ready_index[r][c]=numclients;
                          numclients++;
                        }
			if (verbose > 1) printf("Radar: %d, Channel: %d Recv Beamnum: %d Status %d\n",
			  client.radar,client.channel,client.rbeam,msg.status);	
                        if (numclients >= maxclients) msg.status=-2;
		        if (verbose > 1) printf("\nclient ready done\n");	
                        numclients=numclients % maxclients;
		        if (verbose > 1) printf("numclients: %d\n",numclients);	
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
                        gettimeofday(&t1,NULL);
                        elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
                        elapsed+=(t1.tv_usec-t0.tv_usec);
                        if (verbose > 0) printf("  Receiver Client Ready Elapsed Microseconds: %ld\n",elapsed);
                        if (verbose > 1)  printf("Ending Client Ready Setup\n");
                        break; 

		      case RECV_PRETRIGGER:
                        gettimeofday(&t0,NULL);
			if(verbose > 1 ) printf("Setup Receiver Card for next trigger numclients: %d\n",numclients);	
                        if (configured) {
                          if (IMAGING==0) {
                          //MSI
			   for(r=0;r<MAX_RADARS;r++){
			    chanABready=0;
			    chanCDready=0;
			    for(i=0;i<numclients;i++){
                             if(r==(clients[i].radar-1)) { 
                              c=(clients[i].channel-1 % 2); 
                              b=0; 
			      if(verbose > 1 ) printf("Set up card parameters for client: %d r: %d c: %d\n",i,r,c);	
                              if(verbose > 1 ) printf(" client: %d :  Samples: %d  Rate: %f rFreq: %d\n",
                                i,clients[i].number_of_samples,clients[i].baseband_samplerate,clients[i].rfreq);
                              card=r;
			      if(verbose > 1 ) printf("%d Setup filters %d %d\n",
                                                 i,(int) clients[i].filter_bandwidth,clients[i].match_filter);	
//JDS: TODO set match num samples and frequency here
                              if (c==0) {
			        if(verbose > 1 ) printf("%d: ChanAB\n",i);	
				chanABready=1;
                                chana.rate=clients[i].baseband_samplerate;
                                chanb.rate=clients[i].baseband_samplerate;
                                chana.match=clients[i].match_filter;
                                chanb.match=clients[i].match_filter;
                                chana.samples=clients[i].number_of_samples;
                                chanb.samples=clients[i].number_of_samples;
                                if(ifmode) {
                                  chana.freq= pow( 2, 32)* 
                                  ( (double) ( FCLOCK - IF_FREQ*1000))/ ( (double) Fsamp);
                                }
                                else {
                                  chana.freq= pow( 2, 32)* 
                                  ( (double) ( clients[i].rfreq*1000))/ ( (double) Fsamp);
                                }
                                chanb.freq= chana.freq;
                                //printf("Channel A:\n");
                                //printf("  rfreq: %d\n",clients[i].rfreq);
                                //printf("  samples: %d\n",clients[i].number_of_samples);
                                //printf("  samplerate: %lf\n",clients[i].baseband_samplerate);
 
                              } // c=0
                              if (c==1) {
			        if(verbose > 1 ) printf("%d: ChanCD\n",i);	
				chanCDready=1;
                                chanc.rate=clients[i].baseband_samplerate;
                                chand.rate=clients[i].baseband_samplerate;
                                chanc.match=clients[i].match_filter;
                                chand.match=clients[i].match_filter;
                                chanc.samples=clients[i].number_of_samples;
                                chand.samples=clients[i].number_of_samples;
                                if(ifmode) {
                                  chanc.freq= pow( 2, 32)* 
                                  ( (double) ( FCLOCK - IF_FREQ*1000))/ ( (double) Fsamp);
                                }
                                else {
                                  chanc.freq= pow( 2, 32)* 
                                  ( (double) ( clients[i].rfreq*1000))/ ( (double) Fsamp);
                                }
                                chand.freq= chanc.freq;
 
                              } //c=1
                             }
			    } // client loop
                            if((chanABready==0) && (chanCDready==1)) {
			        if(verbose > 1 ) printf("Force ChanAB\n");	
				chanABready=1;
                                chana.rate=chanc.rate;
                                chanb.rate=chand.rate;
                                chana.match=chanc.match;
                                chanb.match=chand.match;
                                chana.samples=chanc.samples;
                                chanb.samples=chand.samples;
				chana.freq=chanc.freq;
				chanb.freq=chand.freq;

                            } 
                            if((chanCDready==0) && (chanABready==1)) {
			        if(verbose > 1 ) printf("Force ChanCD\n");	
				chanCDready=1;
                                chanc.rate=0;
                                chand.rate=0;
                                chanc.match=0;
                                chand.match=0;
                                chanc.samples=0;
                                chand.samples=0;
				chanc.freq=0;
				chand.freq=0;

                            } 
                            //printf(" preA::  Samples: %d  Dly: %d B:: Samples %d Dly: %d\n",chana.samples,smpdlyAB,chanb.samples,smpdlyAB);
                            //printf(" preC::  Samples: %d  Dly: %d D:: Samples %d Dly: %d\n",chana.samples,smpdlyAB,chanb.samples,smpdlyAB);
                            setcard(&global,&chana,&chanb,&chanc,&chand,Fsamp,&smpdlyAB,&smpdlyCD,cfircoeffsAB,cfircoeffsCD,pfircoeffsAB,pfircoeffsCD,resampcoeffs);
                            if(chana.rate>=10000) smpdlyAB+=2;
                            if(chanc.rate>=10000) smpdlyCD+=2;
                            baseGC214=BASE2[r];
                            setupGC214(baseGC214,chana.samples+smpdlyAB+RECV_SAMPLE_HEADER,chanc.samples+smpdlyCD+RECV_SAMPLE_HEADER);
                            setupGC4016(baseGC214,&global, resampcoeffs);
                            setupGC4016channel(baseGC214,0,&chana,cfircoeffsAB,pfircoeffsAB);
                            setupGC4016channel(baseGC214,1,&chanb,cfircoeffsAB,pfircoeffsAB);
                            setupGC4016channel(baseGC214,2,&chanc,cfircoeffsCD,pfircoeffsCD);
                            setupGC4016channel(baseGC214,3,&chand,cfircoeffsCD,pfircoeffsCD);
                            
                            //printf(" postA::  Samples: %d  Dly: %d B:: Samples %d Dly: %d\n",chana.samples,smpdlyAB,chanb.samples,smpdlyAB);
                            //printf(" postC::  Samples: %d  Dly: %d B:: Samples %d Dly: %d\n",chanc.samples,smpdlyCD,chand.samples,smpdlyCD);
                            releaseGC4016(baseGC214);
                            enableGC214(baseGC214);
                           } // radar loop
			   if(verbose > 0 ) printf("Done with setting up parameters\n");	
                          } else {
                          //IMAGING
                            msg.status=0;
                          } 
                        } //if configured 
                        if(CH1_ENABLED || CH2_ENABLED) printf("Collection already enabled %d %d\n",CH1_ENABLED,CH2_ENABLED);
                        CH1_ENABLED=1;
                        CH2_ENABLED=1;
                        armed=1;
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
                        gettimeofday(&t1,NULL);
                        elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
                        elapsed+=(t1.tv_usec-t0.tv_usec);
                        if (verbose > 0) printf("  Receiver Pre-trigger Elapsed Microseconds: %ld\n",elapsed);
                        if (verbose > 1)  printf("Ending Pretrigger Setup\n");
                        gettimeofday(&tpre,NULL);
                        //triggerGC214(baseGC214);
                        break; 

		      case RECV_POSTTRIGGER:
                        gettimeofday(&t0,NULL);
			if(verbose > 1 ) printf("Receiver post-trigger\n");	
                        armed=0;
                        numclients=0;
                        for (r=0;r<MAX_RADARS;r++){
                          for (c=0;c<MAX_CHANNELS;c++){
                            ready_index[r][c]=-1;
                          }
                        }

                        msg.status=0;
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
                        gettimeofday(&t1,NULL);
                        elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
                        elapsed+=(t1.tv_usec-t0.tv_usec);
                        if (verbose > 0) printf("  Receiver Post-trigger Elapsed Microseconds: %ld\n",elapsed);
                        if (verbose > 1)  printf("Ending Post-trigger\n");
                        break;

		      case RECV_GET_DATA:
                        gettimeofday(&t0,NULL);
			if(verbose > 1 ) printf("Receiver get data\n");	
		        rval=recv_data(msgsock,&client,sizeof(struct ControlPRM));
                        r=client.radar-1; 
                        c=client.channel-1; 
                        baseGC214=BASE2[r];
			if(verbose > 1 ) printf("  radar: %d channel: %d\n",client.radar,client.channel);	
                        b=0; 
                        status=0;
                        rval=send_data(msgsock,&status,sizeof(status));
                        if(status==0) {
                          if(verbose > 1 ) printf("  SHM Memory: %d\n",shm_memory);
                          rval=send_data(msgsock,&shm_memory,sizeof(shm_memory));
                          if(verbose > 1 ) printf("  FRAME Offset: %d\n",RECV_SAMPLE_HEADER);
                          tmp32=RECV_SAMPLE_HEADER;
                          rval=send_data(msgsock,&tmp32,sizeof(tmp32));
                          if(verbose > 1 ) printf("  DMA Buf: %d\n",b);
                          rval=send_data(msgsock,&b,sizeof(b));
//JDS - NEED to convert driver to double buffer
//                          b=client.buffer_index; 
                          samples=client.number_of_samples;
                          rval=send_data(msgsock,&samples,sizeof(samples));
			  if(verbose > 1 ) printf("Sent Number of Samples %d\n",samples);	
			  if(verbose > 1 ) printf("r: %d c: %d\n",r,c);	
                          if (IMAGING==0) {
                            if (configured) {
                              while(CH1_ENABLED || CH2_ENABLED); //Wait for data
			      if(verbose > 0 ) printf("Send collected data %p %p\n",main_virtual[r][c][b],back_virtual[r][c][b]);	
//Take Data and Retrieve it
                              main=shm_main_addresses[r][c][b];
                              back=shm_back_addresses[r][c][b];
                              if(c==0) {
                                main_offset=0x00000;
                                back_offset=0x20000;
                                smpdly=smpdlyAB;
                              } else if (c==1) {
                                main_offset=0x40000;
                                back_offset=0x60000;
                                smpdly=smpdlyCD;
                              } else {
                                main_offset=0x40000;
                                back_offset=0x60000;
                                smpdly=smpdlyCD;
                              }
                              //printf("Get Data smpdly: %d\n",smpdly); 
                              if (verbose > 1 ) fprintf(stdout,"---- smpdly samples ----\n");  
			      for(x=0;x<smpdly;x++) {
                                tmp=read32(baseGC214+main_offset);
                                if (verbose > 1) {      
                                  I=(tmp & 0xffff0000) >> 16;
                                  Q=tmp & 0x0000ffff;  
                                  power=(double)((double)I*(double)I+(double)Q*(double)Q);  
                                  power=10*log10(power);   
                                  if (verbose > 1 ) fprintf(stdout,"%8d :: %8d %8d %8.3lf :: ",x,I,Q,(double)power);                                        
                                }
                                tmp=read32(baseGC214+back_offset);
                                if (verbose > 1) {      
                                  I=(tmp & 0xffff0000) >> 16;
                                  Q=tmp & 0x0000ffff;  
                                  power=(double)((double)I*(double)I+(double)Q*(double)Q);  
                                  power=10*log10(power);   
                                  if (verbose > 1 ) fprintf(stdout,"%8d %8d %8.3lf\n",x,I,Q,(double)power);                                        
                                }
                              }
                              fflush(stdout);
                              if (verbose > 1 ) fprintf(stdout,"---- samples ----\n");  
                              for(x=0;x<samples;x++) {
                                tmp=read32(baseGC214+main_offset);
                                main[x]=tmp;
                                if(x<300) {
                                  if (verbose > 1) {
                                    I=(main[x] & 0xffff0000) >> 16;
                                    Q=main[x] & 0x0000ffff;
                                    //P=(long long)pow(I,2)+pow(Q,2);
                                    power=(double)((double)I*(double)I+(double)Q*(double)Q);  
                                    power=10*log10(power); 
                                    fprintf(stdout,"%8d :: %8d %8d %8.3lf :: ",x,I,Q,(double)power);
                                  }
                                }
                                tmp=read32(baseGC214+back_offset);
                                back[x]=tmp;
                                if(x<300) {
                                  if (verbose > 1) {
                                    I=(back[x] & 0xffff0000) >> 16;
                                    Q=back[x] & 0x0000ffff;
                                    //P=(long long)pow(I,2)+pow(Q,2);
                                    power=(double)((double)I*(double)I+(double)Q*(double)Q);  
                                    power=10*log10(power); 
                                    fprintf(stdout,"%8d %8d %8.3lf\n",I,Q,(double)power);
                                  }
                                }
                              }
                              fflush(stdout);
                              main_address=main_physical[r][c][b];
                              back_address=back_physical[r][c][b]; 
			      rval=send_data(msgsock,&main_address,sizeof(main_address));
			      rval=send_data(msgsock,&back_address,sizeof(back_address));
			      if(verbose > 0 ) printf("Sent Main Address %p\n",main_address);	
			      if(verbose > 0 ) printf("Sent Back Address %p\n",back_address);	
                            } else {
			      if(verbose >0) printf("Send simulated data %p %p\n",main_test_data[r][c][0],back_test_data[r][c][0]);	
                              utemp=0;
                              main_address=main_test_data[r][c][0];
                              back_address=back_test_data[r][c][0]; 
			      rval=send_data(msgsock,&main_address,sizeof(main_address));
			      rval=send_data(msgsock,&back_address,sizeof(back_address));
			      if(verbose > 0 ) printf("Data sent\n");	
                            }
                          } else {
                            //IMAGING  
                          }
                        }  else {  //end status check
                          //status non-zero
                        }
                        msg.status=0;
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
                        gettimeofday(&t1,NULL);
                        gettimeofday(&tpost,NULL);
                        elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
                        elapsed+=(t1.tv_usec-t0.tv_usec);
                        if (verbose > 1) printf("  Receiver Get Data Elapsed Microseconds: %ld\n",elapsed);
                        if (verbose > 1)  printf("Ending Get Data\n");
                        elapsed=(tpost.tv_sec-tpre.tv_sec)*1E6;
                        elapsed+=(tpost.tv_usec-tpre.tv_usec);
                        if (verbose > 1) printf("  Receiver PreTrig to Get Data Elapsed Microseconds: %ld\n",elapsed);
                        break;

                      case RECV_CLRFREQ:
                        gettimeofday(&t0,NULL);
			if(verbose > 0 ) printf("Clear Frequency Search\n");	
			rval=recv_data(msgsock,&clrfreq_parameters, sizeof(struct CLRFreqPRM));
                        rval=recv_data(msgsock,&client,sizeof(struct ControlPRM));
                        if(verbose > 1 ) printf("Clear Frequency Search %d %d\n",
                                            client.radar-1,client.channel-1);

                        centre=(clrfreq_parameters.end+clrfreq_parameters.start)/2;
                        usable_bandwidth=clrfreq_parameters.end-clrfreq_parameters.start;
                        if(verbose > 1 ) printf("requested values\n");
			if(verbose > 1 ) printf("start: %d\n",clrfreq_parameters.start);	
			if(verbose > 1 ) printf("end: %d\n",clrfreq_parameters.end);	
			if(verbose > 1 ) printf("centre: %d\n",centre);	
			if(verbose > 1 ) printf("bandwidth: %lf in Khz\n",usable_bandwidth);	
			if(verbose > 1 ) printf("nave: %d\n",clrfreq_parameters.nave);	
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
                        usable_bandwidth=N/1.25; //JDS: Added ito maximimize safe usable bandwidth;
                        search_bandwidth=N;            
                        start=(int)(centre-search_bandwidth/2+0.49999);
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
		        if(verbose > 1 ) printf("Nave %d\n",clrfreq_parameters.nave);	
                          gettimeofday(&t2,NULL);
			if(clrfreq_parameters.nave > 5 )
                          clrfreq_parameters.nave=5;
                        for(ii=0;ii<clrfreq_parameters.nave;ii++) {
		          if(verbose > 1 ) printf("Iter: %d\n",ii);	
#ifdef __QNX__
//do the hardware sampling here.
//JDS - NEED to convert driver to double buffer
//                              b=client.buffer_index; 
//			      gc314SetDMABuffer(gc314fs[r], c, b);
//JDS Force Card index to 0 for testing 
                          if(!armed){
			    if(verbose > 1 ) printf("  armed: %d\n",armed);	
                            c=0;
                            b=0;
                            r=0;
                            freqout=N*1000;
                            chana.rate=freqout;
                            chanb.rate=freqout;
                            chanc.rate=freqout;
                            chand.rate=freqout;
                            chana.match=0;
                            chanb.match=0;
                            chanc.match=0;
                            chand.match=0;
                            chana.samples=N+30;
                            chanb.samples=N+30;
                            chanc.samples=N+30;
                            chand.samples=N+30;
                            chana.freq= pow( 2, 32)* 
                              ( (double) ( centre*1000))/ ( (double) Fsamp);
                            chanb.freq= chana.freq;
                            chanc.freq= chana.freq;
                            chand.freq= chana.freq;
			    if(verbose > 1 ) printf("  setcard\n");	
                            setcard(&global,&chana,&chanb,&chanc,&chand,Fsamp,&smpdlyAB,&smpdlyCD,cfircoeffsAB,cfircoeffsCD,pfircoeffsAB,pfircoeffsCD,resampcoeffs);
			    if(verbose > 1 ) printf("  baseGC214\n");	
                            baseGC214=BASE2[r];
			    if(verbose > 1 ) printf("  setupGC214\n");	
                            setupGC214(baseGC214,chana.samples+smpdlyAB,chanc.samples+smpdlyCD);
			    if(verbose > 1 ) printf("  setupGC4016\n");	
                            setupGC4016(baseGC214,&global, resampcoeffs);
                            setupGC4016channel(baseGC214,0,&chana,cfircoeffsAB,pfircoeffsAB);
                            setupGC4016channel(baseGC214,1,&chanb,cfircoeffsAB,pfircoeffsAB);
                            setupGC4016channel(baseGC214,2,&chanc,cfircoeffsAB,pfircoeffsAB);
                            setupGC4016channel(baseGC214,3,&chand,cfircoeffsAB,pfircoeffsAB);
			    if(verbose > 1 ) printf("  releaseGC4016\n");	
                            releaseGC4016(baseGC214);
			    if(verbose > 1 ) printf("  enableGC4016\n");	
                            enableGC214(baseGC214);
			    if(verbose > 1 ) printf("trigger\n");	
                            CH1_ENABLED=1;
                            CH2_ENABLED=1;
                            triggerGC214(baseGC214);
                            gettimeofday(&t4,NULL);
                            while(CH1_ENABLED || CH2_ENABLED); //wait for data
                            gettimeofday(&t5,NULL);
                            if (verbose > 1) { 
                              elapsed=(t5.tv_sec-t4.tv_sec)*1E6;
                              elapsed+=(t5.tv_usec-t4.tv_usec);
                              printf("    Receiver: ClrFreq Collection Wait Elapsed Microseconds: %ld\n",elapsed);
                            }
                            power=0;
                            main=main_virtual[r][c][b];
                            back=back_virtual[r][c][b];
                            if(c==0) {       
                              main_offset=0x00000;   
                              back_offset=0x20000;                                    
                              smpdly=smpdlyAB;
                            } else if (c==1) {
                              main_offset=0x40000;
                              back_offset=0x60000;
                              smpdly=smpdlyCD;
                            } else {
                              main_offset=0x40000; 
                              back_offset=0x60000; 
                              smpdly=smpdlyCD;
                            }
                            for(x=0;x<smpdly+20;x++) tmp=read32(baseGC214+main_offset);
                            for(x=0;x<N;x++) { 
                              tmp=read32(baseGC214+main_offset); 
                              I=tmp&0x0000ffff;
                              Q=tmp>>16;
                              in[x][1]=Q;
                              in[x][0]=I;
                              if((x<20) && (ii==0)) {
                                if(verbose > 1) fprintf(stdout,"%8d :: %8d %8d :: %8.3lf %8.3lf :: \n",x,I,Q,in[x][0],in[x][1]);
                              }
                            }
                            if (verbose > 1) { 
                              gettimeofday(&t3,NULL);
                              elapsed=(t3.tv_sec-t2.tv_sec)*1E6;
                              elapsed+=(t3.tv_usec-t2.tv_usec);
                              printf("    Receiver: ClrFreq Iteration %d Fill %d FFT Input Elapsed Microseconds: %ld\n",ii,N,elapsed);
                            }
                            fflush(stdout); 
                          } else {
                        //if armed set an error condition
                            msg.status=-1;
                          }
#else
			  if(verbose > 0 ) printf("Not QNX: Fill In array %p\n",in);	
                          for (j=0;j<N;j++) {
  		            in[j][0]=1;	
  		            in[j][1]=0;	
                          }
#endif
                          if (msg.status) {
                            gettimeofday(&t2,NULL);
                            fftw_execute(plan);
/* calculate power from output of fft */
                            for (j=0;j<N;j++) {
                              pwr[j]+=((out[j][0]*out[j][0])+(out[j][1]*out[j][1]))/(double)(N*N);
                            }
//			    if(verbose > 1 ) {  printf("FFT Out/pwr\n");	
//                              for (j=0;j<N;j++) {
//                                printf("%d :: Out: %lf %lf  pwr: %lf\n",j,out[j][0],out[j][1],pwr[j]);
//                              }
//                            }
                            if (verbose > 1) { 
                              gettimeofday(&t3,NULL);
                              elapsed=(t3.tv_sec-t2.tv_sec)*1E6;
                              elapsed+=(t3.tv_usec-t2.tv_usec);
                              printf("      Receiver: ClrFreq Iteration %d Calculate %d FFT Elapsed Microseconds: %ld\n",ii,N,elapsed);
                            }

                          }
                        } //end of nave loop

/* take average power for nave number of calculations */
                        if(verbose > 1 ) printf("Average pwr\n");	
                        for(i=0;i<N;i++) pwr[i]=pwr[i]/(clrfreq_parameters.nave);
  /* Re-arrange the output of the fft to go from start to end (fftshift).
     This centers the fft, so now the first element in pwr2 corresponds
     with the start freq, and goes up to the end freq
  */
                        if(verbose > 1 ) printf("Reorder pwr\n");	
                        for(i=0;i<(N/2);i++){
                          pwr2[N/2+i]=pwr[i];
                          pwr2[i]=pwr[N/2+i];
                        }
//                        if(verbose > 0 ) {
//                          printf("shifted pwr array\n");	
//                          for(i=0;i<N;i++){
//                            printf("%d: %lf %lf\n",i,pwr[i],pwr2[i]);
//                          }
//                        }
                        if(pwr!=NULL) free(pwr);
                        pwr=NULL;  
                        rval=send_data(msgsock, &clrfreq_parameters, sizeof(struct CLRFreqPRM));
                        rval=send_data(msgsock, &usable_bandwidth, sizeof(usable_bandwidth));
                        rval=send_data(msgsock, &pwr2[(int)ceil(unusable_sideband)], sizeof(double)*usable_bandwidth);  //freq order power
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
                        fftw_destroy_plan(plan);
                        fftw_free(in); 
                        fftw_free(out);
                        in=NULL;
                        out=NULL;
                        if(pwr2!=NULL) free(pwr2);
                        pwr2=NULL;  
                        if (verbose > 1) { 
                          gettimeofday(&t1,NULL);
                          elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
                          elapsed+=(t1.tv_usec-t0.tv_usec);
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






