#include <stdlib.h>
#include <devctl.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sched.h>
#include <process.h>
#include <pthread.h>
#include <unistd.h>
#include <hw/pci.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/iomsg.h>
#include <sys/resmgr.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include "include/_regs_GC314FS.h"
#include "include/_regs_GC4016.h"
#include "include/_prog_conventions.h"
#include "include/_global.h"
#include "include/_structures.h"
#include "../include/gc314FS.h"

#define FIFO_LVL	   100 
#define RECORD_SIZE	   500

#define PRINT 0 


int io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
int io_write (resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);
int io_devctl (resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);

static resmgr_connect_funcs_t connect_funcs;
static resmgr_io_funcs_t io_funcs;
static iofunc_attr_t attr;

typedef union _devctl_msg {
	int tx;
	int rx;
} data_t;
int pci_index;

struct channel_params chanA, chanB, chanC, chanD;


int		 fd_1, fd_2, fd_3, *tmp_ptr, file_count=0;
int		 sample_count=0, header_size=52, DMA_switch=0;
unsigned int	 virtual_addr[12], physical_addr[12], write_size=0;
pthread_t	helperA=NULL, helperB=NULL, helperC=NULL, helperD=NULL;
struct		timespec time_write;
struct		 tm* time_struct;
struct         	 tm* time_struct2;
FILE		 *fid1, *fout, *fid2, *fid3, *fidtemp, fid_ch1, fid_ch2, fid_ch3, *f_err;
int		 timeri=0,  records=0;
unsigned char	 *BASE0, *BASE1, *BASE1_phys;
unsigned char	 *BASE0_dio, *io_BASE1_dio, *BASE1_phys_dio;
struct		 timespec start_p, stop_p, start, stop, sleeper, time_now;
int		 count=0, loop=1, dma_size=512, collect=1, samples_to_collect;
float		 timerf=0;
short		 resampcoeffs[256];
unsigned int	 resampratios[4];
struct		 GC4016_reg_vals GC1_vals, GC2_vals, Gc3_vals;
struct		 GC4016_channel_regs GC4016[4], *GC4016_ptr[4];
unsigned int	 msg_ptr_temp;
int		channel_on_off[4]={0,0,0,0};
int		channel_waiting[4]={0,0,0,0};
int		channel_stat[4]={0,0,0,0};
int		DMA_in_use[2]={0,0};
int		fifo_lvl[4];
int		skip[4]={0,0,0,0};
int		version;
resmgr_attr_t resmgr_attr;
dispatch_t *dpp;
dispatch_context_t *ctp;
char *device;
io_devctl_t *msg_to_pass;
struct		 iovec iov[1];

int get_available_DMA_channel(){
	struct  timespec  sleep;
	int	temp;
	
	sleep.tv_sec=0;
	sleep.tv_nsec=1000;


	return 1;

	while(1){
		if(DMA_in_use[0]==0){
			DMA_in_use[0]=1;
			//fprintf(stderr, "DMA 0 in use\n");
			return 0;
		}
		if(DMA_in_use[1]==0){
			DMA_in_use[1]=1;
			//fprintf(stderr, "DMA 1 in use\n");
			return 1;
		}
		temp=clock_nanosleep(CLOCK_REALTIME, 0, &sleep, 0);
	}

}

void * WaitForChannelA(){
	int i,j,temp, nbytes=0;
	int scount;
	int DMA_to_use=0;  
	int loops,remaining_samples;
        channel_stat[0]=0;	
	if(PRINT) printf("wait for data\n");	
	loops=(int)(chanA.samples/fifo_lvl[0]);
	remaining_samples=chanA.samples%fifo_lvl[0];
        if (PRINT) {
          fprintf(stderr,"%d: Channel A: Start Wait\n",pci_index);
          fprintf(stderr,"%d: Channel A: Loops: %d fifo_lvl: %d remaining: %d\n",pci_index,loops,fifo_lvl[0],remaining_samples);
          fflush(stderr);
        }
	scount=0;
	temp=clock_gettime(CLOCK_REALTIME, &start);
	for (i=0;i<loops;i++){
		channel_stat[0] = wait_on_fifo_lvl(BASE1, CHANNEL_A);
                if (channel_stat[0]!=0) {
                  if (PRINT) {
                    fprintf(stderr,"%d Channel A: fifo_lvl ERROR:  Loop: %d\n",pci_index,i);
                    fflush(stderr);
                  }
                  break;
                }
	        temp=clock_gettime(CLOCK_REALTIME, &stop);
		DMA_to_use=get_available_DMA_channel();
		channel_stat[0]=_DMA_transfer_sg(BASE0, BASE1, 0x40000, &physical_addr[0], 4*scount, fifo_lvl[0], 0, DMA_to_use);
//                channel_stat[0]=_DMA_transfer_NODMA(BASE0,BASE1,0x40000,&physical_addr[0],9,fifo_lvl[0]);
		DMA_in_use[DMA_to_use]=0;
		scount += fifo_lvl[0];
                if (channel_stat[0]!=0) {
                  if (PRINT ) {
                    fprintf(stderr,"%d Channel A: DMA transfer ERROR:  Loop: %d\n",pci_index,i);
                    fflush(stderr);
                  }
                  break;
                }
	}
        if (channel_stat[0]== 0) {
	  if (remaining_samples > 0){
	    channel_stat[0]=wait_on_fifo_finish(BASE1, CHANNEL_A);
            if ( channel_stat[0]==0) {
	      DMA_to_use=get_available_DMA_channel();
	      channel_stat[0]=_DMA_transfer_sg(
                BASE0, BASE1, 0x40000, &physical_addr[0], 
                4*scount, remaining_samples, 0, DMA_to_use);
	      DMA_in_use[DMA_to_use]=0;
	      scount += remaining_samples;
              if( channel_stat[0]!=0) {
                if (PRINT) {
                  fprintf(stderr,"%d Channel A: final DMA transfer ERROR\n",pci_index);
                  fflush(stderr);
                }
              }
            } else {
              if (PRINT) {
                fprintf(stderr,"%d Channel A: fifo_finish ERROR\n",pci_index);
                fflush(stderr);
              }
            }
	  }
        } 
        *((uint32*)(BASE1+GC314FS_R2ACSR)) |= 0x00000004;
        *((uint32*)(BASE1+GC314FS_R1ACSR)) |= 0x00000004;
        *((uint32*)(BASE1+GC314FS_R3ACSR)) |= 0x00000004;
	temp=clock_gettime(CLOCK_REALTIME, &stop);
        channel_on_off[0]=0;
        if (PRINT) {
          fprintf(stderr,"%d Channel A: End Wait\n",pci_index);
          fflush(stderr);
        }
	pthread_exit(0);
}
void * WaitForChannelB(){
	int i,j,temp, nbytes=0;
	int scount;
	int DMA_to_use=0;
	int loops,remaining_samples;
        channel_stat[1]=0;	
        //fprintf(stderr,"%d: Channel B: Start Wait\n",pci_index);
        //fflush(stderr);
	if(PRINT) printf("wait for data\n");	
	loops=(int)(chanB.samples/fifo_lvl[1]);
	remaining_samples=chanB.samples%fifo_lvl[1];
        //fprintf(stderr,"%d Channel B: Wait: LVL: %4d Loops: %4d Remaining: %4d\n",pci_index,fifo_lvl[1],loops,remaining_samples);
        //fflush(stderr);
	scount=0;
        temp=clock_gettime(CLOCK_REALTIME, &start);
	for (i=0;i<loops;i++){
                channel_stat[1] = wait_on_fifo_lvl(BASE1, CHANNEL_B);
                if (channel_stat[1]!=0) {
                  //fprintf(stderr,"%d Channel B: fifo_lvl ERROR:  Loop: %d\n",pci_index,i);
                  //fflush(stderr);
                  break;
                }
                temp=clock_gettime(CLOCK_REALTIME, &stop);
		DMA_to_use=get_available_DMA_channel();
		channel_stat[1]=_DMA_transfer_sg(BASE0, BASE1, 0x44000, &physical_addr[3], 4*scount, fifo_lvl[1], 0, DMA_to_use);
		DMA_in_use[DMA_to_use]=0;
		scount += fifo_lvl[1];
                if (channel_stat[1]!=0) {
                  //fprintf(stderr,"%d Channel B: DMA transfer ERROR:  Loop: %d\n",pci_index,i);
                  //fflush(stderr);
                  break;
                }
	}
        if (channel_stat[1]== 0) {
	  if (remaining_samples > 0){
	    channel_stat[1]=wait_on_fifo_finish(BASE1, CHANNEL_B);
            if (channel_stat[1]== 0) {
	      DMA_to_use=get_available_DMA_channel();
	      channel_stat[1]=_DMA_transfer_sg(BASE0, 
                BASE1, 0x44000, &physical_addr[3], 4*scount, 
                remaining_samples, 0, DMA_to_use);
	      DMA_in_use[DMA_to_use]=0;
	      scount += remaining_samples;
              if( channel_stat[1]!=0) {
                //fprintf(stderr,"%d Channel B: final DMA transfer ERROR\n",pci_index);
               // fflush(stderr);
              }
            } else {
              //fprintf(stderr,"%d Channel B: fifo_finish ERROR\n",pci_index);
              //fflush(stderr);
            }
	  }
	}
        //fprintf(stderr,"%d Channel B: Stop Collection\n",pci_index);
        //fflush(stderr);
        *((uint32*)(BASE1+GC314FS_R2BCSR)) |= 0x00000004;
        *((uint32*)(BASE1+GC314FS_R1BCSR)) |= 0x00000004;
        *((uint32*)(BASE1+GC314FS_R3BCSR)) |= 0x00000004;
        temp=clock_gettime(CLOCK_REALTIME, &stop);
        channel_on_off[1]=0;
        //fprintf(stderr,"%d Channel B: End Wait\n",pci_index);
        //fflush(stderr);
	pthread_exit(0);
}
void * WaitForChannelC(){
	int i,j,temp, nbytes=0;
	int scount;
	int DMA_to_use=0;
	int loops,remaining_samples;
        channel_stat[2]=0;      
	if(PRINT) printf("wait for data\n");	
	loops=(int)(chanC.samples/fifo_lvl[2]);
	remaining_samples=chanC.samples%fifo_lvl[2];
	scount=0;
        temp=clock_gettime(CLOCK_REALTIME, &start);
	for (i=0;i<loops;i++){
                channel_stat[2] = wait_on_fifo_lvl(BASE1, CHANNEL_C);
                if (channel_stat[2]!=0) {
                  //fprintf(stderr,"%d Channel C: fifo_lvl ERROR:  Loop: %d\n",pci_index,i);
                  //fflush(stderr);
                  break;
                }
                temp=clock_gettime(CLOCK_REALTIME, &stop);
		DMA_to_use=get_available_DMA_channel();
	        channel_stat[2]=_DMA_transfer_sg(BASE0, BASE1, 0x48000, &physical_addr[6], 4*scount, fifo_lvl[2], 0, DMA_to_use);
		DMA_in_use[DMA_to_use]=0;
		scount += fifo_lvl[2];
                if (channel_stat[2]!=0) {
                  //fprintf(stderr,"%d Channel C: DMA transfer ERROR:  Loop: %d\n",pci_index,i);
                  //fflush(stderr);
                  break;
                }
	}
        if (channel_stat[2]== 0) {
	  if (remaining_samples > 0){
	    channel_stat[2]=wait_on_fifo_finish(BASE1, CHANNEL_C);
            if (channel_stat[2]== 0) {
	      DMA_to_use=get_available_DMA_channel();
	      channel_stat[2]=_DMA_transfer_sg(BASE0, 
                BASE1, 0x48000, &physical_addr[6], 4*scount, 
                remaining_samples, 0, DMA_to_use);
	      DMA_in_use[DMA_to_use]=0;
	      scount += remaining_samples;
              if( channel_stat[2]!=0) {
                //fprintf(stderr,"%d Channel C: final DMA transfer ERROR\n",pci_index);
                //fflush(stderr);
              }
            } else {
              //fprintf(stderr,"%d Channel C: fifo_finish ERROR\n",pci_index);
              //fflush(stderr);
            }
	  }
	} 
        *((uint32*)(BASE1+GC314FS_R2CCSR)) |= 0x00000004;
        *((uint32*)(BASE1+GC314FS_R1CCSR)) |= 0x00000004;
        *((uint32*)(BASE1+GC314FS_R3CCSR)) |= 0x00000004;
        temp=clock_gettime(CLOCK_REALTIME, &stop);
        channel_on_off[2]=0;
	pthread_exit(0);
}
void * WaitForChannelD(){
	int i,j,temp, nbytes=0;
	int scount;
	int DMA_to_use=0;
	int loops,remaining_samples;
        channel_stat[3]=0;      
	if(PRINT) printf("wait for data\n");	
	loops=(int)(chanD.samples/fifo_lvl[3]);
	remaining_samples=chanD.samples%fifo_lvl[3];
	scount=0;
        temp=clock_gettime(CLOCK_REALTIME, &start);
	for (i=0;i<loops;i++){
                channel_stat[3] = wait_on_fifo_lvl(BASE1, CHANNEL_D);
                if (channel_stat[3]!=0) {
                  //fprintf(stderr,"%d Channel D: fifo_lvl ERROR:  Loop: %d\n",pci_index,i);
                  //fflush(stderr);
                  break;
                }
                temp=clock_gettime(CLOCK_REALTIME, &stop);
		DMA_to_use=get_available_DMA_channel();
		channel_stat[3]=_DMA_transfer_sg(BASE0, BASE1, 0x4c000, &physical_addr[9], 4*scount, fifo_lvl[3], 0, DMA_to_use);
                DMA_in_use[DMA_to_use]=0;
		scount += fifo_lvl[3];
                if (channel_stat[3]!=0) {
	          //fprintf(stderr,"%d Channel D: DMA transfer ERROR:  Loop: %d\n",pci_index,i);
                  //fflush(stderr);
                  break;
                }
	}
        if (channel_stat[3]== 0) {
	  if (remaining_samples > 0){
	    channel_stat[3]=wait_on_fifo_finish(BASE1, CHANNEL_D);
            if (channel_stat[3]== 0) {
	      DMA_to_use=get_available_DMA_channel();
	      channel_stat[3]=_DMA_transfer_sg(BASE0, 
                BASE1, 0x4c000, &physical_addr[9], 4*scount, 
                remaining_samples, 0, DMA_to_use);
	      DMA_in_use[DMA_to_use]=0;
	      scount += remaining_samples;
              if( channel_stat[3]!=0) {
                //fprintf(stderr,"%d Channel D: final DMA transfer ERROR\n",pci_index);
                //fflush(stderr);
              }
            } else {
              //fprintf(stderr,"%d Channel D: fifo_finish ERROR\n",pci_index);
              //fflush(stderr);
            }
    	  }
        } 
        *((uint32*)(BASE1+GC314FS_R2DCSR)) |= 0x00000004;
        *((uint32*)(BASE1+GC314FS_R1DCSR)) |= 0x00000004;
        *((uint32*)(BASE1+GC314FS_R3DCSR)) |= 0x00000004;
        temp=clock_gettime(CLOCK_REALTIME, &stop);
        channel_on_off[3]=0;
	pthread_exit(0);
}
int main( int argc, char **argv){

	int		 temp, pci_handle, pci_handle_dio, i, j, frame_size, IRQ, IRQ_dio, temp2, dma_count=0;
	short		 I, Q;
	unsigned int	 *mmap_io_ptr, *mmap_io_ptr_dio, mmap_io_dio, CLOCK_RES;
	float		 time;
	struct		 _clockperiod new, old;


	int		 stat[4000], stat_time[4000];
	
	int 		 test_array[6], *temp_ptr;
	int		 a[10]={1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	int		 *b;
	short		 *c;

	char 		 test_array2[128];

	float		 start_freq, stop_freq, step_freq, i_f;
	int		 freq_count=0;
	short		 CH0_I[CH0_SAMPLES_TO_COLLECT], CH0_Q[CH0_SAMPLES_TO_COLLECT], CH1_I[CH1_SAMPLES_TO_COLLECT], CH1_Q[CH1_SAMPLES_TO_COLLECT];
	int		 CH0_done=0, CH1_done=0;

	unsigned int	 dio_DMA_vaddr, dio_DMA_paddr, *dio_buff;	
	int		 pseq[10]={0, 10, 20, 30, 40, 50, 60, 70, 80, 90}, scope_sync[16384], TR[16384], TX[16384], TX_array[16384], trigger[16384];
	int		 tau=2000, tperiod=10, tlength=300, time_array[10];
	char		 hdr[4]="%HDR";
	char		 data_file[255], data_file2[255], data_file3[255], err_file[255], strtemp[255];

	int		 iovcnt;
	int		 expnum;
	int		 sample_freq=250000, freq1=10000000, freq2=10000000, freq3=10000000;
	int		 status, buffer_addr=27, thread_flag=0, status2;
	pthread_t	 helper=0;

	int		speriod;
	double		sfreq;
	float		sfreqfloat;
	expnum=9999;
	sample_freq=10000;
	freq1=12000000;
	freq2=12000000;
	freq3=12000000;
	samples_to_collect=500;
	fifo_lvl[0]=FIFO_LVL;
	fifo_lvl[1]=FIFO_LVL;
	fifo_lvl[2]=FIFO_LVL;
	fifo_lvl[3]=FIFO_LVL;


	//printf("Channel A fifo_lvl: %d\n", fifo_lvl[0]);
	if(PRINT) printf("\n\n\n\n The sampling frequency is %d Hz\n", sample_freq);
	if(PRINT) printf("The center frequency of CHANNEL 1 is %d Hz\n", freq1);
	if(PRINT) printf("The center frequency of CHANNEL 2 is %d Hz\n", freq2);
	if(PRINT) printf("The center frequency of CHANNEL 3 is %d Hz\n", freq3);
	if(PRINT) printf("The number of samples to be collected is %d\n", samples_to_collect);
	
	resampratios[0]=0x04000000;
	resampratios[1]=0x04000000;
	resampratios[2]=0x04000000;
	resampratios[3]=0x04000000;
	
	device = calloc((size_t) 64, sizeof(char));
	strcat(device,"temp.out");
	strcat(device,(const char *) argv[1]);
	fout=fopen(device, "w");
	free(device);
 //MAKE SURE A DEVICE NUMBER WAS SPECIFIED AT THE COMMAND LINE 
	// the command line argument defines which gc314fs card this driver
	// should be attached to
	if( argc == 1 ){
		fprintf(stderr, "%s: Invoke with device number (i.e. 'gc314FS 1')\n",argv[0]);
		return EXIT_FAILURE;
	}
     //READ THE COMMAND LINE ARGUMENT AND CREATE THE DEVICE NAME TO REGISTER WITH THE OS
	device = calloc((size_t) 64, sizeof(char));
	strcat(device,"/dev/gc314fs-");
	strcat(device,(const char *) argv[1]);
	sscanf(argv[1],"%d",&pci_index);
	if(PRINT) printf("_GC314FS_DRV: INITIALIZING DEVICE %s\n",device);
	if(PRINT) printf("_GC314FS_DRV: DEVICE ID %d\n",pci_index);
     //REGISTER AND INITALIZE THIS CODE AS A RESOURCE MANAGER
	//initialize dispatch interface 
	if((dpp = dispatch_create()) == NULL){
		fprintf(stderr, "%s: Unable to allocate dispatch handle: %s\n", argv[0],strerror(errno));
		return EXIT_FAILURE;
	}
	//initialize resource manager attributes 
	memset(&resmgr_attr, 0, sizeof resmgr_attr);
	resmgr_attr.nparts_max = 1;
	resmgr_attr.msg_max_size = 2048;
	//initialize functions for handling messages
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, 
		 _RESMGR_IO_NFUNCS, &io_funcs);
	io_funcs.read = io_read;
	io_funcs.write = io_write;
	io_funcs.devctl = io_devctl;
	//initialize attribute structure used by the device
	iofunc_attr_init(&attr, S_IFNAM | 0666, 0, 0);
	//attach our device name
	temp = resmgr_attach(dpp,         // dispatch handle
		&resmgr_attr,   	// resource manager attrs
		device,  		// device name
		_FTYPE_ANY,     	// open type
		0,              	// flags
		&connect_funcs, 	// connect routines
		&io_funcs,      	// I/O routines
		&attr);         	// handle
	if(temp == -1){
		fprintf(stderr, "%s: Unable to attach name.\n",argv[0]);
		return EXIT_FAILURE;
	}
	free(device);
	//allocate a context structure 
	ctp = dispatch_context_alloc(dpp);






    /* SET THE SYSTEM CLOCK RESOLUTION AND GET THE START TIME OF THIS PROCESS */
	// set the system clock resolution to 10 us
	new.nsec=10000;
	new.fract=0;
/*
	temp=ClockPeriod(CLOCK_REALTIME,&new,0,0);
	if(temp==-1) 	perror("Unable to change system clock resolution");
*/
	temp=ClockPeriod(CLOCK_REALTIME,0,&old,0);
	if(temp==-1) 	perror("Unable to read sytem time");
	CLOCK_RES=old.nsec;
    /* OPEN THE PLX9656 AND GET LOCAL BASE ADDRESSES */
	temp=_open_PLX9656(&BASE0, &BASE1, &BASE1_phys, &pci_handle, &mmap_io_ptr, &IRQ, pci_index, 0);
	if(temp==-1)	 fprintf(stderr, "PLX9695 configuration failed");
	else 		 fprintf(fout, "PLX9656 configuration successful!\n");
    /* CONFIGURE FPGA WITH DEFAULT IMAGE */
	temp=_config_FPGA(BASE1, 0, 0, 0);
	if(temp==-1)	 fprintf(stderr, "STX_FLASH configuration failed");
	else	 	 fprintf(fout, "STX_FLASH configuration successful!\n");
    /* INITIALIZE BOARD FOR DATA COLLECTION */
	// reset board
	temp=_reset_GC314FS(BASE1);
	// initialize GC314 to known state
	// *******************************************************************************************
	//if(pci_index==6) temp=_config_GC314FS(BASE1, CLOCK_INTERNAL, pci_index);
	//else temp=_config_GC314FS(BASE1, CLOCK_EXTERNAL, pci_index);
	//temp=_config_GC314FS(BASE1, CLOCK_EXTERNAL, pci_index);
	temp=_config_GC314FS(BASE1, CLOCK_EXTERNAL, pci_index);
	// *******************************************************************************************
	if(temp==-1)	 fprintf(stderr, "GC314 initialization failed\n");
	else		 fprintf(fout, "GC314FS Initialized successfully!\n");
        version=temp;
	// set pll registers
	// *******************************************************************************************
	//if(pci_index==6) temp=_config_PLLs(BASE1, CLOCK_INTERNAL, FCLOCK);
	//else temp=_config_PLLs(BASE1, CLOCK_EXTERNAL, FCLOCK);
	//temp=_config_PLLs(BASE1, CLOCK_EXTERNAL, FCLOCK);
	temp=_config_PLLs(BASE1, CLOCK_EXTERNAL, FCLOCK);
	// *******************************************************************************************
	if(temp==-1)	 fprintf(stderr, "PLL configuration failed\n");
	else		 fprintf(fout, "PLL configuration sucessfull!\n");
    /* CREATE DMA BUFFERS FOR ALL RECIEVER CHANNELS */
	for (i=0;i<12;i++){
		//temp=_create_DMA_buff(&virtual_addr[i], &physical_addr[i], samples_to_collect*4+header_size);
		temp=_create_DMA_buff(&virtual_addr[i], &physical_addr[i], 1048576);
                printf("DMA buff: %d virt: %x phys: %x\n",i,virtual_addr[i],physical_addr[i]);
		if (temp==-1){
			fprintf(fout, "ERROR MAKING DMA BUFFERS!\n");
			break;
		}
	}
	if (temp == 1)	fprintf(fout, "DMA buffers created sucessfully!\n");
    /* SET DEFAULT OPERATING PARAMETERS */
	chanA.channel=0;
	chanB.channel=1;
	chanC.channel=2;
	chanD.channel=3;
	chanA.freq=12000000;
	chanB.freq=12000000;
	chanC.freq=12000000;
	chanD.freq=12000000;
	chanA.BW=10000;
	chanB.BW=10000;
	chanC.BW=10000;
	chanD.BW=10000;
	chanA.rate=10000;
	chanB.rate=10000;
	chanC.rate=10000;
	chanD.rate=10000;
	chanA.matched=1;
	chanB.matched=1;
	chanC.matched=1;
	chanD.matched=1;
	chanA.samples=samples_to_collect;
	chanB.samples=samples_to_collect;
	chanC.samples=samples_to_collect;
	chanD.samples=samples_to_collect;
    /* CALCULATE AND SET THE PARAMETERS OF THE GC4016s */
	temp=_set_GC4016_vals(&chanA, &GC4016[0].cfir_coeffs[0], resampcoeffs, resampratios);
	temp=_set_GC4016_vals(&chanA, &GC4016[1].cfir_coeffs[0], resampcoeffs, resampratios);
	temp=_set_GC4016_vals(&chanA, &GC4016[2].cfir_coeffs[0], resampcoeffs, resampratios);
	temp=_set_GC4016_vals(&chanA, &GC4016[3].cfir_coeffs[0], resampcoeffs, resampratios);
	GC4016_ptr[0]=&GC4016[0];
	GC4016_ptr[1]=&GC4016[1];
	GC4016_ptr[2]=&GC4016[2];
	GC4016_ptr[3]=&GC4016[3];
	c=&GC4016;
	if (temp == 1)	fprintf(fout, "GC4016s parameters calculated sucessfully!\n");

	//printf("we are collecting %d\n", samples_to_collect);

	sleeper.tv_sec=0;
	sleeper.tv_nsec=CLOCK_RES;
	temp=setprio(0,20);


// ****************************************************************
	speriod=30;
	sfreq=1/(speriod*1e-6);
	sfreqfloat=(float)sfreq;
	sfreqfloat=3333.3334;
	//printf("sample freq is %f\n", sfreqfloat);
	
//	exit(0);
// ****************************************************************

	
     //START THE RESOURCE MANAGER MAIN LOOP
	while(1){
		if((ctp = dispatch_block(ctp)) == NULL) {
			fprintf(stderr,"block_error\n");
			return EXIT_FAILURE;
		}
		dispatch_handler(ctp);
	}

	
    /* CLOSE THE PLX9656 AND CLEAR ALL MEMORY MAPS */
	temp=_close_PLX9656(pci_handle, BASE0, BASE1, mmap_io_ptr);
	printf("close:		0x%x\n", 3);

    /* READ SYSTEM TIME AND END OF PROCESS, AND PRINT TOTAL TIME TO RUN */
	temp=clock_gettime(CLOCK_REALTIME, &stop_p);
	if(temp==-1){
		perror("Unable to read sytem time");
	}
	time=(float)(stop_p.tv_nsec-start_p.tv_nsec);
	fprintf(fout, "TOTAL TIME: %f\n",time);

	fflush(fout);
	fclose(fout);
	exit;
 }
int io_devctl (resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb){
	int nbytes=0, status, previous;
	uint32_t value, temp;
	int	i,j,channel ;
	double elapsed=0;

	union {
		data_t	   data;
		int    data32;
		short	CFIRcoeffs[11];
		struct S_external_trigger_data	external_trigger_data;
		struct S_filter_data  		filter_data;
		struct S_frequency_data		frequency_data;
		struct S_global_reset_data	global_reset_data;
		struct S_output_rate_data	output_rate_data;
		struct S_buffer_address		buffer_address;
		struct S_samples_data		samples_data;
		struct S_sync1_data		sync1_data;
		struct S_sync_mask_data		sync_mask_data;
		struct S_RDA_data		RDA_data;
		int	channel;
	} *rx_data;


	if((status = iofunc_devctl_default(ctp, msg, ocb)) != _RESMGR_DEFAULT){
		return(status);
	}

	rx_data = _DEVCTL_DATA(msg->i);

	temp = get_device_command(msg->i.dcmd);

	switch(msg->i.dcmd){
		case GC314_GET_BUFFER_ADDRESS:
			//printf("get buffer address\n");	
			switch(rx_data->buffer_address.chip){
				case 0:
					//printf(" chip %d\n", rx_data->buffer_address.chip);
					switch(rx_data->buffer_address.channel){
						case CHANNEL_A:
							//printf(" channel A\n");
							rx_data->buffer_address.address=physical_addr[0];
                                                        //printf("Chip 0  Channel A: %d  phys_addr: %x\n",CHANNEL_A,physical_addr[0]);
							nbytes=sizeof(rx_data->buffer_address);
							msg->o.nbytes = nbytes;
							break;
						case CHANNEL_B:
							//printf(" channel B\n");
							rx_data->buffer_address.address=physical_addr[3];
							nbytes=sizeof(rx_data->buffer_address);
							msg->o.nbytes = nbytes;
							break;
						case CHANNEL_C:
							//printf(" channel C\n");
							rx_data->buffer_address.address=physical_addr[6];
							nbytes=sizeof(rx_data->buffer_address);
							msg->o.nbytes = nbytes;
							break;
						case CHANNEL_D:
							//printf(" channel D\n");
							rx_data->buffer_address.address=physical_addr[9];
							nbytes=sizeof(rx_data->buffer_address);
							msg->o.nbytes = nbytes;
							break;
					}
					break;
				case 1:
					//printf(" chip %d\n", rx_data->buffer_address.chip);
					switch(rx_data->buffer_address.channel){
						case CHANNEL_A:
							//printf(" channel A\n");
							rx_data->buffer_address.address=physical_addr[1];
							nbytes=sizeof(rx_data->buffer_address);
							msg->o.nbytes = nbytes;
							break;
						case CHANNEL_B:
							//printf(" channel B\n");
							rx_data->buffer_address.address=physical_addr[4];
							nbytes=sizeof(rx_data->buffer_address);
							msg->o.nbytes = nbytes;
							break;
						case CHANNEL_C:
							//printf(" channel C\n");
							rx_data->buffer_address.address=physical_addr[7];
							nbytes=sizeof(rx_data->buffer_address);
							msg->o.nbytes = nbytes;
							break;
						case CHANNEL_D:
							//printf(" channel D\n");
							rx_data->buffer_address.address=physical_addr[10];
							nbytes=sizeof(rx_data->buffer_address);
							msg->o.nbytes = nbytes;
							break;
					}
					break;
				case 2:
					//printf(" chip %d\n", rx_data->buffer_address.chip);
					switch(rx_data->buffer_address.channel){
						case CHANNEL_A:
							//printf(" channel A\n");
							rx_data->buffer_address.address=physical_addr[2];
							nbytes=sizeof(rx_data->buffer_address);
							msg->o.nbytes = nbytes;
							break;
						case CHANNEL_B:
							//printf(" channel B\n");
							rx_data->buffer_address.address=physical_addr[5];
							nbytes=sizeof(rx_data->buffer_address);
							msg->o.nbytes = nbytes;
							break;
						case CHANNEL_C:
							//printf(" channel C\n");
							rx_data->buffer_address.address=physical_addr[8];
							nbytes=sizeof(rx_data->buffer_address);
							msg->o.nbytes = nbytes;
							break;
						case CHANNEL_D:
							//printf(" channel D\n");
							rx_data->buffer_address.address=physical_addr[11];
							nbytes=sizeof(rx_data->buffer_address);
							msg->o.nbytes = nbytes;
							break;
					}
					break;
			}
			return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
			break;
  		case GC314_CHANNEL_ON:
			if(PRINT) fprintf(stderr,"Turning on channel\n");	
			switch(rx_data->channel){
				case CHANNEL_A:
					if(PRINT) fprintf(stderr,"channel A\n");
					channel_on_off[0]=1;
					break;
				case CHANNEL_B:
					if(PRINT) fprintf(stderr,"channel B\n");
					channel_on_off[1]=1;
					break;	
				case CHANNEL_C:
					if(PRINT) fprintf(stderr,"channel C\n");
					channel_on_off[2]=1;
					break;
				case CHANNEL_D:
					if(PRINT) fprintf(stderr,"channel D\n");
					channel_on_off[3]=1;
					break;
			}
                        if (PRINT) fflush(stderr); 
			return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
			break;  
		case GC314_SET_FILTERS:
			if(PRINT) printf("set filters\n");
			switch(rx_data->filter_data.channel){
				case CHANNEL_A:
					chanA.BW=rx_data->filter_data.bandwidth;
					chanA.matched=rx_data->filter_data.matched;
					if(PRINT) printf("channel A BW=%d, matche=%d\n", chanA.BW, chanA.matched);
					break;
				case CHANNEL_B:
					chanB.BW=rx_data->filter_data.bandwidth;
					chanB.matched=rx_data->filter_data.matched;
					break;	
				case CHANNEL_C:
					chanC.BW=rx_data->filter_data.bandwidth;
					chanC.matched=rx_data->filter_data.matched;
					break;
				case CHANNEL_D:
					chanD.BW=rx_data->filter_data.bandwidth;
					chanD.matched=rx_data->filter_data.matched;
					break;
				
			}
			return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
			break;	
		case GC314_SET_FREQUENCY:
			if(PRINT) printf("set frequency\n");
			switch(rx_data->frequency_data.channel){
				case CHANNEL_A:
					chanA.freq=rx_data->frequency_data.freq;
					if(PRINT) printf("channel A freq=%d   %d\n", chanA.freq, rx_data->frequency_data.freq);
					break;
				case CHANNEL_B:
					chanB.freq=rx_data->frequency_data.freq;
					break;	
				case CHANNEL_C:
					chanC.freq=rx_data->frequency_data.freq;
					break;
				case CHANNEL_D:
					chanD.freq=rx_data->frequency_data.freq;
					break;
			}
			return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
			break;
		case GC314_SET_GLOBAL_RESET:
			if(PRINT) printf("Global reset:");
			switch(rx_data->global_reset_data.state){
				case GC314_ON:
					if(PRINT) printf(" ON\n");
					write08(BASE1, GC314FS_GC1offset+GC4016_GLOBAL_RESET,		0xf8);
					write08(BASE1, GC314FS_GC2offset+GC4016_GLOBAL_RESET,		0xf8);
					write08(BASE1, GC314FS_GC3offset+GC4016_GLOBAL_RESET,		0xf8);
					break;
				case GC314_OFF:
					if(PRINT) printf(" OFF\n");
					write08(BASE1, GC314FS_GC1offset+GC4016_GLOBAL_RESET,		0x08);
					write08(BASE1, GC314FS_GC2offset+GC4016_GLOBAL_RESET,		0x08);
					write08(BASE1, GC314FS_GC3offset+GC4016_GLOBAL_RESET,		0x08);
					break;
			}
			return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
			break;
		case GC314_LOAD_GC4016S:
			if(PRINT) printf("Load Gc4016's\n");
			temp=_config_GC4016(BASE1, 1, &GC4016_ptr, resampcoeffs, resampratios);
			return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
			break;
		case GC314_SET_RDA:
			if(PRINT) printf("Set RDA channel: %d\n", rx_data->RDA_data.channel);
			switch(rx_data->RDA_data.channel){
				case CHANNEL_A:
					temp=_set_RDA(BASE1, 1, chanA.samples, skip[0], fifo_lvl[0], CHANNEL_A);
					break;
				case CHANNEL_B:
					temp=_set_RDA(BASE1, 1, chanB.samples, skip[1], fifo_lvl[1], CHANNEL_B);
					break;
				case CHANNEL_C:
					temp=_set_RDA(BASE1, 1, chanC.samples, skip[2], fifo_lvl[2], CHANNEL_C);
					break;
				case CHANNEL_D:
					temp=_set_RDA(BASE1, 1, chanD.samples, skip[3], fifo_lvl[3], CHANNEL_D);
					break;
			}
			return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
			break;
		case GC314_SET_EXTERNAL_TRIGGER:
			if(PRINT) printf("External trigger:");
			switch(rx_data->external_trigger_data.state){
				case GC314_ON:
					if(PRINT) printf(" ON\n");
					*((uint32*)(BASE1+GC314FS_GCSR))				|=0x00800000;
					break;
				case GC314_OFF:
					if(PRINT) printf(" OFF\n");
					*((uint32*)(BASE1+GC314FS_GCSR))				&=0xff7fffff;
					break;
			}
			return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
			break;
		case GC314_SET_SAMPLES:
			if(PRINT) printf("set samples\n");
			switch(rx_data->samples_data.channel){
				case CHANNEL_A:
					chanA.samples=rx_data->samples_data.samples;
					if(chanA.samples < FIFO_LVL) fifo_lvl[0]=chanA.samples;
					else fifo_lvl[0]=FIFO_LVL;
					if(PRINT) printf("channel A samples=%d\n", chanA.samples);
					break;
				case CHANNEL_B:
					chanB.samples=rx_data->samples_data.samples;
					if(chanB.samples < FIFO_LVL) fifo_lvl[1]=chanB.samples;
					else fifo_lvl[1]=FIFO_LVL;
					break;	
				case CHANNEL_C:
					chanC.samples=rx_data->samples_data.samples;
					if(chanC.samples < FIFO_LVL) fifo_lvl[2]=chanC.samples;
					else fifo_lvl[2]=FIFO_LVL;
					break;
				case CHANNEL_D:
					chanD.samples=rx_data->samples_data.samples;
					if(chanC.samples < FIFO_LVL) fifo_lvl[3]=chanD.samples;
					else fifo_lvl[3]=FIFO_LVL;
					break;
			}
			return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
			break;
		case GC314_SET_SYNC1:
			if(PRINT) printf("Set Sync1\n");
			switch(rx_data->sync1_data.state){
				case GC314_ON:
					if(PRINT) printf(" sync1 on\n");
					*((uint32*)(BASE1+GC314FS_GCSR))				|=0x00200000;
					break;
				case GC314_OFF:
					if(PRINT) printf(" sync1 off\n");
					*((uint32*)(BASE1+GC314FS_GCSR))				&=0xffdfffff;
					break;
				case GC314_PULSE:
					if(PRINT) printf(" sync1 pulsed\n");
					*((uint32*)(BASE1+GC314FS_GCSR))				|=0x00400000;
					break;
			}
			return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
			break;
		case GC314_SET_SYNC_MASK:
			if(PRINT) printf("Set Sync Mask\n");
			switch(rx_data->sync_mask_data.state){
				case SYNC1_SIA_ONETIME:
					if(PRINT) printf(" sync1 SIA one time\n");
					*((uint32*)(BASE1+GC314FS_GSM))					|=0x00008000;
					break;
				case SYNC1_SIB_ONETIME:
					if(PRINT) printf(" sync1 SIB one time\n");
					*((uint32*)(BASE1+GC314FS_GSM))					|=0x00002000;
					break;
				case SYNC1_RTSC_CLEAR:
					if(PRINT) printf(" sync1 relative time stamp counter clear one time\n");
					*((uint32*)(BASE1+GC314FS_GSM))					|=0x00040000;
					break;
				case SYNC1_SIA_HOLD:
					if(PRINT) printf(" sync1 SIA hold\n");
					*((uint32*)(BASE1+GC314FS_GSM))					|=0x00100000;
					break;
				case SYNC1_SIA_CONTINUOUS:
					if(PRINT) printf(" sync1 SIA continuous\n");
					*((uint32*)(BASE1+GC314FS_GSM))					|=0x00004000;
					break;
			}
			return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
			break;
		case GC314_SET_OUTPUT_RATE:
			if(PRINT) printf("set output rate: %d\n", rx_data->output_rate_data.channel);
			switch(rx_data->output_rate_data.channel){
				case CHANNEL_A:
					chanA.rate=rx_data->output_rate_data.output_rate;
					if(PRINT) printf("channel A rate=%f\n", chanA.rate);
					break;
				case CHANNEL_B:
					chanB.rate=rx_data->output_rate_data.output_rate;
					break;	
				case CHANNEL_C:
					chanC.rate=rx_data->output_rate_data.output_rate;
					break;
				case CHANNEL_D:
					chanD.rate=rx_data->output_rate_data.output_rate;
					break;
			}
			//nbytes=sizeof(rx_data->data.rx);
			//msg->o.ret_val = GC314_SET_FILTERS_CHA;
			//msg->o.nbytes = nbytes;
			//return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
			return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
			break;
		case GC314_UPDATE_CHANNEL:
			if(PRINT) printf("update channel\n");	
			switch(rx_data->channel){
				case CHANNEL_A:
					if(PRINT) printf("channel A\n");
					skip[0]=_set_GC4016_vals(&chanA, &GC4016[0].cfir_coeffs[0], resampcoeffs, resampratios);
					break;
				case CHANNEL_B:
					if(PRINT) printf("channel B\n");
					skip[1]=_set_GC4016_vals(&chanB, &GC4016[1].cfir_coeffs[0], resampcoeffs, resampratios);
					break;	
				case CHANNEL_C:
					if(PRINT) printf("channel C\n");
					skip[2]=_set_GC4016_vals(&chanC, &GC4016[2].cfir_coeffs[0], resampcoeffs, resampratios);
					break;
				case CHANNEL_D:
					if(PRINT) printf("channel D\n");
					skip[3]=_set_GC4016_vals(&chanD, &GC4016[3].cfir_coeffs[0], resampcoeffs, resampratios);
					break;
			}
			return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
			break;
		case GC314_START_COLLECTION:
                        if (PRINT) fprintf(stderr,"Start Collection\n");
                        if ( ( *((uint32*)(BASE1+GC314FS_CPLL)) & 0x00000004 ) != 0x00000004 ){ 
                          fprintf(stderr,"Collection: PLLs not locked to external clock\n");
                          fflush(stderr);
                        }
			if(channel_on_off[0]) {
                          if(channel_waiting[0]) fprintf(stderr,"ChanA already waiting for data!\n");

                          if(helperA!=NULL) {
                             fprintf(stderr,"%d GC314_START_COLLECTION\n",pci_index);
                             fprintf(stderr,"  %d HelperA thread %d already exists! Waiting for it to die\n",pci_index,helperA);
	                    temp=clock_gettime(CLOCK_REALTIME, &start);
                            pthread_join(helperA, NULL);
	                    temp=clock_gettime(CLOCK_REALTIME, &stop);
                            elapsed=stop.tv_sec-start.tv_sec+((double)(stop.tv_nsec-start.tv_nsec))/1E9;
                            fprintf(stderr,"  %d Joined HelperA thread\n",pci_index,helperA);
                            fprintf(stderr,"  %d Elapsed  secs: %lf\n",pci_index,elapsed);
                            helperA=NULL;
                          }
                          pthread_create(&helperA,NULL,WaitForChannelA,NULL);
                        }
			if(channel_on_off[1]) {
                          if(channel_waiting[1]) printf("ChanB already waiting for data!\n");
                          if(helperB!=NULL) printf("HelperB thread already exists!\n");
                          else pthread_create(&helperB,NULL,WaitForChannelB,NULL);
                        }
			if(channel_on_off[2]) {
                          if(channel_waiting[2]) printf("ChanC already waiting for data!\n");
                          if(helperC!=NULL) printf("HelperC thread already exists!\n");
                          else pthread_create(&helperC,NULL,WaitForChannelC,NULL);
                        }
			if(channel_on_off[3]) {
                          if(channel_waiting[3]) printf("ChanD already waiting for data!\n");
                          if(helperD!=NULL) printf("HelperD thread already exists!\n");
                          else pthread_create(&helperD,NULL,WaitForChannelD,NULL);
                        }
                        fflush(stderr);
			return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));			//
			break;
		case GC314_WAIT_FOR_DATA:
			sample_count=0;
			
			/*
			if(DMA_switch==0){
				DMA_switch=1;
			}
			else{
				DMA_switch=0;
			}
			*/
                          channel=rx_data->channel;
                        if (PRINT) {
                          fprintf(stderr,"%d: Wait for Data Start: %d\n",pci_index,channel);
                          fprintf(stderr,"Wait for Data:: PCI Index: %d Channel: %d\n",pci_index,channel);
                          fflush(stderr);
                        }
                        channel_waiting[0]=1;
                        if(helperA!=NULL) pthread_join(helperA, NULL);
                        helperA=NULL;
                        channel_waiting[0]=0;
			channel_on_off[0]=0;

                        channel_waiting[1]=1;
                        if(helperB!=NULL) pthread_join(helperB, NULL);
                        helperB=NULL;
                        channel_waiting[1]=0;
			channel_on_off[1]=0;
                        channel_waiting[2]=1;

                        if(helperC!=NULL) pthread_join(helperC, NULL);
                        helperC=NULL;
                        channel_waiting[2]=0;
			channel_on_off[2]=0;
                        channel_waiting[3]=1;

                        if(helperD!=NULL) pthread_join(helperD, NULL);
                        helperD=NULL;
                        channel_waiting[3]=0;
			channel_on_off[3]=0;

                        switch(channel) {
                          case 0:  
                            rx_data->channel=channel_stat[0];
                            break;
                          case 1: 
                            rx_data->channel=channel_stat[1];
                            break;
                          case 2:
                            rx_data->channel=channel_stat[2];
                            break;
                          case 3:
                            rx_data->channel=channel_stat[3];
                            break;
                          default:
                            rx_data->channel=-10;
                            break;
                        }
			nbytes=sizeof(rx_data->channel);
			msg->o.nbytes = nbytes;
                        if (PRINT) {
                          fprintf(stderr,"%d: Wait for Data End: %d\n",pci_index,channel);
                          fflush(stderr);
                        }
			return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
			break;

		case GC314_ABORT_CHANNEL:
                       if(PRINT) printf("Turning on channel\n");
                       switch(rx_data->channel){
                               case CHANNEL_A:
                                       if(PRINT) printf("channel A\n");
                                       channel_on_off[0]=0;
                  *((uint32*)(BASE1+GC314FS_R1ACSR)) |= 0x00000004;
                  *((uint32*)(BASE1+GC314FS_R2ACSR)) |= 0x00000004;
                  *((uint32*)(BASE1+GC314FS_R3ACSR)) |= 0x00000004;
                                       break;
                               case CHANNEL_B:
                                       if(PRINT) printf("channel B\n");
                                       channel_on_off[1]=0;
                  //fprintf(stderr,"%d Channel B: Abort Collection\n",pci_index);
                  //fflush(stderr);
                  *((uint32*)(BASE1+GC314FS_R1BCSR)) |= 0x00000004;
                  *((uint32*)(BASE1+GC314FS_R2BCSR)) |= 0x00000004;
                  *((uint32*)(BASE1+GC314FS_R3BCSR)) |= 0x00000004;
                                       break;
                               case CHANNEL_C:
                                       if(PRINT) printf("channel C\n");
                                       channel_on_off[2]=0;
                  *((uint32*)(BASE1+GC314FS_R1CCSR)) |= 0x00000004;
                  *((uint32*)(BASE1+GC314FS_R2CCSR)) |= 0x00000004;
                  *((uint32*)(BASE1+GC314FS_R3CCSR)) |= 0x00000004;
                                       break;
                               case CHANNEL_D:
                                       if(PRINT) printf("channel D\n");
                                       channel_on_off[3]=0;
                  *((uint32*)(BASE1+GC314FS_R1DCSR)) |= 0x00000004;
                  *((uint32*)(BASE1+GC314FS_R2DCSR)) |= 0x00000004;
                  *((uint32*)(BASE1+GC314FS_R3DCSR)) |= 0x00000004;
                                       break;
                       }
                       return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)+nbytes));
                       break;
		default:
			return(ENOSYS);
	}
}




int
io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
  int nleft;
  int nbytes;
  int nparts;
  int status;
  uint32_t chip;
  //char *lbuff="This is just a test!";
  char lbuff[2048];
  char tempbuff[2048];
  uint32_t cval;
  int return_len;
  int tempint;

  if ((status = iofunc_read_verify(ctp, msg, ocb, NULL)) != EOK)
    return(status);

  if ((msg->i.xtype & _IO_XTYPE_NONE) != _IO_XTYPE_NONE)
    return (ENOSYS);

  //set up 'string' of data to return
  //fprintf(stderr,"io_read - cval %d\n",cval);
  //tempint=*((uint08*)(gc314base+GC314FS_GC1offset+GC4016_GLOBAL_RESET));
/*
  sprintf(tempbuff,"GLOBAL_RESET Chip 1 = 0x%02x\n", *((uint08*)(gc314base+GC314FS_GC1offset+GC4016_GLOBAL_RESET)) );
  strcat(lbuff,tempbuff);
  sprintf(tempbuff,"GLOBAL_RESET Chip 2 = 0x%02x\n", *((uint08*)(gc314base+GC314FS_GC2offset+GC4016_GLOBAL_RESET)) );
  strcat(lbuff,tempbuff);
  sprintf(tempbuff,"GLOBAL_RESET Chip 3 = 0x%02x\n", *((uint08*)(gc314base+GC314FS_GC3offset+GC4016_GLOBAL_RESET)) );
  strcat(lbuff,tempbuff);
  lbuff[80]=0;
*/

  //sprintf(lbuff,"%s","just testing what I can");
  printf("%s\n", lbuff);
  return_len=strlen(lbuff)+1;
  printf("return length is %d\n", return_len);
  
  //nleft = ocb->attr->nbytes - ocb->offset;
  nleft = return_len - ocb->offset;
  nbytes = min(msg->i.nbytes, nleft);

  if (nbytes > 0){
    /* set up the return data IOV */
    SETIOV( ctp->iov, lbuff + ocb->offset, nbytes );

    /* set up the number of bytes (returned by client's read()) */
    _IO_SET_READ_NBYTES(ctp, nbytes);

    /*
     * advance the offset by the number of bytes 
     * returned to the client.
     */

    ocb->offset += nbytes;

    nparts = 1;
  } else {
    /*
     * they've asked for zero bytres or they've already 
     * read everything
     */
    _IO_SET_READ_NBYTES(ctp,0);

    nparts = 0;
  }

  /* mark the access time as invalid (we just accessed it) */
  if (msg->i.nbytes > 0)
    ocb->attr->flags |= IOFUNC_ATTR_ATIME;
 
  return (_RESMGR_NPARTS(nparts));
}

int
io_write( resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb)
{
  
  /*
   * On all reads, calculate how many
   * bytes we can return to the client
   * based upon the number of bytes available (nleft)
   * and the client's buffer size
   */

  int status;
  char *buf;

  if((status = iofunc_write_verify(ctp, msg, ocb, NULL)) != EOK )
    return(status);

  if((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
    return(ENOSYS);

  _IO_SET_WRITE_NBYTES(ctp, msg->i.nbytes);

  buf = (char *)malloc(msg->i.nbytes + 1);
  if( buf == NULL)
    return(ENOMEM);

  /*
   * reread the data from the sender's message buffer.
   * We're not assuming that all of the data fit into the 
   * resource manager library's receive buffer.
   */
  if( (status = resmgr_msgread(ctp, buf, msg->i.nbytes, sizeof(msg->i))) == -1){ 
    return(ENOSYS);
  }
  //fprintf(stderr,"_ics660-drvr:  bytes attemted: %d  bytes received %d\n",msg->i.nbytes,status);
  buf[msg->i.nbytes] = '\0'; //just in case text is not NULL terminated 
//  memcpy((int *)ics660->mem1,buf,(size_t)msg->i.nbytes);
  free(buf);

  if(msg->i.nbytes > 0)
    ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

  return(_RESMGR_NPARTS(0));
}

