//top- clock-out
//     clock-in
//     trigger
//     chan4
//     chan3
//     chan2
//bot- chan1
 

#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#ifdef __QNX__
  #include <sys/dispatch.h>
  #include <sys/iofunc.h>
  #include <sys/iomsg.h>
  #include <devctl.h>
  #include <hw/inout.h>
  #include  <hw/pci.h>
  #include <sys/socket.h>
  #include <sys/neutrino.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
  #include <sys/neutrino.h>
#endif
#include "control_program.h"
#include "global_server_variables.h"
#include "utils.h"
#include "beam_phase.h"
#include "dds_defs.h"
#include "ics660b.h"


int sock,msgsock;
int verbose=0;
uint32_t ifmode=IF_ENABLED;
struct  RXFESettings rf_settings;
struct  RXFESettings if_settings;
double d=ANTENNA_SEPARATION;
unsigned int     transmitters[16][2] = {
                 /* pci index:0-?, chip:1-4 */
                        {0,1},
                        {0,2},
                        {0,3},
                        {0,4},
                        {1,1},
                        {1,2},
                        {1,3},
                        {1,4},
                        {2,1},
                        {2,2},
                        {2,3},
                        {2,4},
                        {3,1},
                        {3,2},
                        {3,3},
                        {3,4},
                };
float  test_freqs[16]= {
			1E6, 
			2E6, 
			3E6, 
			4E6, 
                        5E6, 
                        6E6, 
                        7E6,
                        8E6, 
                        9E6, 
                        10E6,
                        11E6, 
                        12E6, 
                        13E6,
                        14E6, 
                        15E6, 
                        16E6,
                };
float dp=0.41887902;
float  test_phase[16];
 
void graceful_cleanup(int signum)
{
  char path[256];
  sprintf(path,"%s:%d","rosdds",0);
  close(msgsock);
  close(sock);
  fprintf(stdout,"Unlinking Unix Socket: %s\n",path);
  unlink(path);
  exit(0);
}


int main(){
    // DECLARE AND INITIALIZE ANY NECESSARY VARIABLES

	// socket and message passing variables
	char	datacode;
	int	rval;
        fd_set rfds,efds;
        int status;

	// counter and temporary variables
	int	i,ant,j,k,r,c,cc,count;
        int     configured,buf,ratio,index,last_num=-1,numclients=0;
	int	tempcode;
        double microseconds;
        // ics660 variables
        int pci_ind,pci_min;
        int pci_master=DDS_MASTER_INDEX;
        FILE *ics660[4];
        char *device;
        int enable = ENABLE;

        struct  DriverMsg msg;
	// timing related variables
        struct  timeval tv;
	struct	timespec	start, stop, sleep, now;
	int	clockresolution;

        int  maxclients=MAX_RADARS*MAX_CHANNELS;
        int  max_seq_count;
        int  seq_count[MAX_RADARS][MAX_CHANNELS];
        int32_t  current_pulse_index[MAX_RADARS][MAX_CHANNELS];
        int  ready_index[MAX_RADARS][MAX_CHANNELS];
        int  *seq_buf[MAX_RADARS][MAX_CHANNELS];
        int  active[MAX_RADARS][DDS_MAX_CHANNELS];
        struct  ControlPRM  clients[maxclients],client;
        struct  TSGbuf *pulseseqs[MAX_RADARS][MAX_CHANNELS][MAX_SEQS];
        int new_seq_flag=0;
        struct timeval t0,t1,t2,t3;
        unsigned long elapsed;

#ifdef __QNX__
	struct	 _clockperiod 	new, old;

#endif
//BEAM vars
        int channel,chip,b_ind;
        int T_rise=10;
        double delta,phase,b0,freq_in;
        double wvnum,bmang; 
        float C=3.e8;
        double pi;
        double state_time = STATE_TIME;
//TIMESEQ vars
        int t_seq[200];
        int state1,state2;
        int inc;
        int seqlen=0; 


//	unsigned int	*master_buf;
        
//	int		 pseq[7]={0, 3, 4, 6}, scope_sync[16384], TR[16384], TX[16384], TX_array[16384], 
//                         trigger[16384], FIFOlevel[16384];
//	int		 tau=2400, tperiod=1, tlength=300, time_array[10], intt=200, loopcount=0, fifocnt=0;

//        signal(SIGINT, graceful_cleanup);
        ifmode=IF_ENABLED;
        for(ant=0;ant<16;ant++) test_phase[ant]=ant*dp;
        if (verbose > 0 ) fprintf(stdout,"Clock Freq: %lf\n",CLOCK_FREQ);
        pi=3.1415926;
        pci_min=0;
        max_seq_count=0;
	if (verbose > 1) fprintf(stdout, "Zeroing arrays\n");
	for (r=0;r<MAX_RADARS;r++){
	  for (c=0;c<MAX_CHANNELS;c++){
	    if (verbose > 1) fprintf(stdout,"%d %d\n",r,c);
	    for (i=0;i<MAX_SEQS;i++) pulseseqs[r][c][i]=NULL;
            current_pulse_index[r][c]=-10;
            ready_index[r][c]=-1; 
            active[r][c]=-1; 
            seq_buf[r][c]=malloc(4*MAX_TIME_SEQ_LEN);
          } 
        }
  if(IMAGING) {
    if (verbose > 0 ) fprintf(stdout,"DDS IMAGING Mode\n");
    if (MAX_RADARS !=1 ) {
            fprintf(stderr, "imaging configuration only supports one radar\n");
            configured=0;
    }
    if (DDS_MAX_CARDS*4 < MAX_TRANSMITTERS) {
            fprintf(stderr, "Too few cards configured for imaging radar configuration\n");
            configured=0;
    }
  } else {
    if (verbose > 0 ) fprintf(stdout,"DDS NON-IMAGING Mode\n");
    if (DDS_MAX_CARDS*2 < MAX_RADARS) {
            fprintf(stderr, "Too few cards configured for imaging radar configuration\n");
            configured=0;
    }

  }
#ifdef __QNX__
  /*open ics660 file */

  if (verbose > 0) fprintf(stdout,"Opening ics660 device files\n");
  for( pci_ind=pci_min; pci_ind < DDS_MAX_CARDS; pci_ind ++)
    {  
      device = (char *)calloc((size_t) 64, sizeof(char));
      sprintf(device,"/dev/ics660-%d",(int)pci_ind);
      // pdebug(stderr,"ICS660_XMT_FP opening %s\n",device);
      ics660[pci_ind] = (FILE *)open(device, O_RDWR);
      if (verbose > 1) fprintf(stdout,"%d Device: %s File: %d\n",pci_ind,device,ics660[pci_ind]);
      free(device);
    }
  
  /* initial the ics660 and dc60m card */
  if (verbose > 0) fprintf(stdout,"Init ics660 and dc60m chips\n");
  for( pci_ind=pci_min; pci_ind<DDS_MAX_CARDS; pci_ind++){
    status=ics660_init(ics660[pci_ind],pci_ind);
    if (verbose > 1) fprintf(stdout,"%d Status: ",pci_ind);
    if (verbose > 1) fprintf(stdout,"%d\n",status);
  }
  
  /* Set DAC enable bit in control register */
  if (verbose > 0) fprintf(stdout,"Set DAC enable bit in control register\n");
  for( pci_ind=pci_min; pci_ind<DDS_MAX_CARDS; pci_ind++)
    ics660_set_parameter((int)ics660[pci_ind],ICS660_DAC_ENABLE,&enable, sizeof(enable));
  
  if (verbose > 0) fprintf(stdout,"Set DC READY bit in control register\n");
  for(pci_ind=pci_min; pci_ind<DDS_MAX_CARDS; pci_ind++)
    ics660_set_parameter((int)ics660[pci_ind],ICS660_SET_DC_READY, &enable, sizeof(enable));
  
  if (verbose > 0) fprintf(stdout,"Release Resets\n");
  for(pci_ind=pci_min; pci_ind<DDS_MAX_CARDS; pci_ind++)
    ics660_set_parameter((int)ics660[pci_ind],ICS660_RELEASE_RESETS, &enable, sizeof(enable));
#endif


//                master_buf=malloc(4*MAX_TIME_SEQ_LEN);



    // OPEN TCP SOCKET AND START ACCEPTING CONNECTIONS 
	//sock=tcpsocket(DDS_HOST_PORT);
	sock=server_unixsocket("rosdds",0);
	listen(sock, 5);
	while (1) {
                rval=1;
		msgsock=accept(sock, 0, 0);
		if (verbose > 0) fprintf(stdout,"accepting socket!!!!!\n");
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
		  if (verbose > 1) fprintf(stdout,"%d Entering Select\n",msgsock);
                  rval = select(msgsock+1, &rfds, NULL, &efds, &tv);
		  if (verbose > 1) fprintf(stdout,"%d Leaving Select %d\n",msgsock,rval);
                  /* Don’t rely on the value of tv now! */
                  if (FD_ISSET(msgsock,&efds)){
                    if (verbose > 1) fprintf(stdout,"Exception on msgsock %d ...closing\n",msgsock);
                    break;
                  }
                  if (rval == -1) perror("select()");
                  rval=recv(msgsock, &buf, sizeof(int), MSG_PEEK); 
                  if (verbose>1) fprintf(stdout,"%d PEEK Recv Msg %d\n",msgsock,rval);
		  if (rval==0) {
                    if (verbose > 1) fprintf(stdout,"Remote Msgsock %d client disconnected ...closing\n",msgsock);
                    break;
                  } 
		  if (rval<0) {
                    if (verbose > 0) fprintf(stdout,"Msgsock %d Error ...closing\n",msgsock);
                    break;
                  } 
                  if ( FD_ISSET(msgsock,&rfds) && rval>0 ) {
                    if (verbose>1) fprintf(stdout,"Data is ready to be read\n");
		    if (verbose > 1) fprintf(stdout,"%d Recv Msg\n",msgsock);
                    rval=recv_data(msgsock,&msg,sizeof(struct DriverMsg));
                    datacode=msg.type;
		    if (verbose > 1) fprintf(stdout,"\nmsg code is %c\n", datacode);
		    switch( datacode ){
		      case DDS_REGISTER_SEQ:
		        if (verbose > -1) fprintf(stdout,"\nRegister new timing sequence for timing card\n");	
		        rval=recv_data(msgsock,&client,sizeof(struct ControlPRM));
                        r=client.radar-1; 
                        c=client.channel-1; 
                        ready_index[r][c]=-1;
                        active[r][c]=-1;
                        current_pulse_index[r][c]=-10;
			if (verbose > -1) fprintf(stdout,"  Radar: %d, Channel: %d Beamnum: %d Status %d\n",
			  client.radar,client.channel,client.tbeam,msg.status);	
		        rval=recv_data(msgsock,&index,sizeof(index));
                        if (pulseseqs[r][c][index]!=NULL) {
		          if (verbose > -1) fprintf(stdout,"  Pulse index %d exists %p\n",index,pulseseqs[r][c][index]);
                          if (pulseseqs[r][c][index]->rep!=NULL) free(pulseseqs[r][c][index]->rep);
                          pulseseqs[r][c][index]->rep=NULL;
                          if (pulseseqs[r][c][index]->code!=NULL) free(pulseseqs[r][c][index]->code);
                          pulseseqs[r][c][index]->code=NULL;
                          free(pulseseqs[r][c][index]);
                          pulseseqs[r][c][index]=NULL;
		          if (verbose > -1) fprintf(stdout,"  Freed Pulse index %d %p\n",index,pulseseqs[r][c][index]);
                        }
                        pulseseqs[r][c][index]=malloc(sizeof(struct TSGbuf));
                        rval=recv_data(msgsock,pulseseqs[r][c][index], sizeof(struct TSGbuf)); // requested pulseseq
                        pulseseqs[r][c][index]->rep=
                          malloc(sizeof(unsigned char)*pulseseqs[r][c][index]->len);
                        pulseseqs[r][c][index]->code=
                          malloc(sizeof(unsigned char)*pulseseqs[r][c][index]->len);
                        rval=recv_data(msgsock,pulseseqs[r][c][index]->rep, 
                          sizeof(unsigned char)*pulseseqs[r][c][index]->len);
                        rval=recv_data(msgsock,pulseseqs[r][c][index]->code, 
                          sizeof(unsigned char)*pulseseqs[r][c][index]->len);
			if (verbose > -1) {
		          fprintf(stdout,"  New Pulse index %d : %p\n",index,pulseseqs[r][c][index]);
                          fprintf(stdout,"    Pulseseq length: %d\n",pulseseqs[r][c][index]->len);	
                        }
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
                        break;

		      case DDS_CtrlProg_END:
                        if (verbose > 1) fprintf(stdout,"DDS driver: Closing a control program\n");
                        break;

		      case DDS_RXFE_SETTINGS:
                        if (verbose > -1) fprintf(stdout,"DDS driver: Configuring for IF Mode\n");
                        rval=recv_data(msgsock,&ifmode,sizeof(ifmode)); 
                        if (verbose > -1) fprintf(stdout,"DDS driver: IF Mode %d \n",ifmode);
                        rval=recv_data(msgsock,&rf_settings,sizeof(struct RXFESettings)); 
                        rval=recv_data(msgsock,&if_settings,sizeof(struct RXFESettings)); 
                        if (ifmode && IMAGING ) fprintf(stderr,"WARNING: RF Mode can not be enabled with IMAGING\n");
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
                       
                        break;
		      case DDS_CtrlProg_READY:
		        if (verbose > 1) fprintf(stdout,"Asking to set up dds timing info for client that is ready\n");	
		        rval=recv_data(msgsock,&client,sizeof(struct ControlPRM));
                        r=client.radar-1; 
                        c=client.channel-1; 
                        index=client.current_pulseseq_index; 

                        clients[numclients]=client;
                        numclients=(numclients+1);
			if (verbose > 1) fprintf(stdout,"Radar: %d, Channel: %d Beamnum: %d Index: %d Numclients: %d\n",
			  client.radar,client.channel,client.tbeam,index,numclients);	
                        if ( (index!=current_pulse_index[r][c]) && (index >= 0) ) {
			  if (verbose > -1) fprintf(stdout,"Need to unpack pulseseq\n");	
			  if (verbose > -1) fprintf(stdout,"Pulseseq length: %d\n",pulseseqs[r][c][index]->len);	
                          ready_index[r][c]=1;
                          active[r][c]=c;
			// unpack the timing sequence
			  seq_count[r][c]=0;
                          microseconds=0.0; 
                          tempcode=0;
	                  if (verbose > 1) fprintf(stdout,"microstep: %d state: %lf ratio: %d\n",
                                                pulseseqs[r][c][index]->step,STATE_TIME,ratio);
                          ratio=(int)((pulseseqs[r][c][index]->step*1E-6)/STATE_TIME+.49999999);
			  for(i=0;i<pulseseqs[r][c][index]->len;i++){
			    tempcode=_decodestate(r,c,(pulseseqs[r][c][index]->code)[i]);	
//JDS TODO chop off first STATE_DELAY microseconds
			    for( j=0;j<ratio*(pulseseqs[r][c][index]->rep)[i];j++){
                              microseconds+=STATE_TIME/1E-6;
                              if(microseconds > STATE_DELAY ) {
			        seq_buf[r][c][seq_count[r][c]]=tempcode;
			        seq_count[r][c]++;
                              }
			    }
			  }
                          current_pulse_index[r][c]=-1;
                        }
	                  if (verbose > 1) fprintf(stdout,"seq length: %d state step: %lf time: %lf\n",
                                                seq_count[r][c],STATE_TIME,STATE_TIME*seq_count[r][c]);
                        if (numclients >= maxclients) msg.status=-2;
		        if (verbose > 1) fprintf(stdout,"client %d ready done %d %d \n",numclients,client.radar,client.channel);	
                        numclients=numclients % maxclients;
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
                        break; 

		      case DDS_PRETRIGGER:
	                one_shot_b(ics660[pci_master]);
                        gettimeofday(&t0,NULL);
			if(verbose > 0 ) {
                          fprintf(stdout,"Setup DDS Card for PRE-trigger Numclients : %d\n",numclients);	
                        }
                        if(numclients>0) {
                     
                          max_seq_count=0;
	                  for( i=0; i<numclients; i++) {
                            r=clients[i].radar-1; 
                            c=clients[i].channel-1; 
                            if (seq_count[r][c]>=max_seq_count) max_seq_count=seq_count[r][c];
		            if (verbose > 0) fprintf(stdout,"Max Seq length %d\n",max_seq_count);	
                            if(clients[i].current_pulseseq_index!=current_pulse_index[r][c]) {
                              current_pulse_index[r][c]=clients[i].current_pulseseq_index;
                            }
                          }
                        }  
	  /* send the seqence to timing_seqence function */
                        if (IMAGING==0) {
                          gettimeofday(&t2,NULL);
                          /* SET freq and filter for MSI */
                          new_seq_flag=0;
	                  for (r=0;r<MAX_RADARS;r++){
	                    for (c=0;c<MAX_CHANNELS;c++){
                              if (ready_index[r][c]>0) {
                                  new_seq_flag=new_seq_flag+1;
                                  ready_index[r][c]=0;
                              } 
                            }
                          }  
                          if (verbose > 0 ) fprintf(stdout," %d :: New seq flag: %d\n",numclients,new_seq_flag);
                              
                          if(numclients>0) {
                            //if (1) { 
                            if (new_seq_flag>0) { 
                              new_seq_flag=0;
                              if (verbose > -1) fprintf(stdout,"DDS:: %d.%06d :: msi_timing : \n",(int)t2.tv_sec,(int)t2.tv_usec);
		              if (verbose > -1) fprintf(stdout,"  Numclients %d\n",numclients);	
                              msi_timing_sequence(numclients,max_seq_count,clients,&seq_count,seq_buf,active,ics660);
                              if (verbose > 0) {
                                gettimeofday(&t3,NULL);
                                elapsed=(t3.tv_sec-t2.tv_sec)*1E6;
                                elapsed+=(t3.tv_usec-t2.tv_usec);
                                if (verbose > 0 ) fprintf(stdout,"  DDS MSI Timing Elapsed Microseconds: %ld\n",elapsed);
                              }
                            } else {
                            } 
                          }
                          gettimeofday(&t2,NULL);
	                  pci_ind=0;

                          for (r=1;r<=MAX_RADARS;r++){
                            for (cc=1;cc<=DDS_MAX_CHANNELS;cc++){
                                load_frequency(ics660[pci_ind], r, cc, 0.0);
                                load_phase(ics660[pci_ind],r,cc,0.0);
                            }
                          }
                          if (verbose > 1) fprintf(stdout,"DDS:: %d.%06d :: parameter_load : \n",(int)t2.tv_sec,(int)t2.tv_usec);
	                  for( i=0; i<numclients; i++) {
	                      freq_in= (double)clients[i].tfreq * 1000.; // in Hz
                              T_rise=clients[i].trise;
	                    r= clients[i].radar;
	                    c= clients[i].channel;
		            if (verbose > 1) fprintf(stdout,"  Client:: r: %d c: %d\n",r,c);	
		            if (verbose > 1) fprintf(stdout,"  Client:: freq: %lf\n",freq_in);	
                            load_frequency(ics660[pci_ind], r, c, freq_in);
                            load_phase(ics660[pci_ind],r,c,0.0);
                            load_filter_taps(ics660[pci_ind],r,c,T_rise,state_time);
                            if (verbose>2) fprintf(stdout," Loading %d freq: %lf chip: %d channel: %d\n",i,freq_in,r,c);
/*
                            for(cc=1;cc<=DDS_MAX_CHANNELS;cc++) {
                              if(active[r-1][cc-1]==(c-1)) {
//                                fprintf(stdout,"Active channel r: %d c: %d cc: %d active: %d\n",
//                                    r,c,cc,active[r-1][cc-1]);
                                gettimeofday(&t3,NULL);
                                load_frequency(ics660[pci_ind], r, cc, freq_in);
                                load_phase(ics660[pci_ind],r,cc,0.0);
                                load_filter_taps(ics660[pci_ind],r,cc,T_rise,state_time);
                              }
                            }
*/
                            if((ifmode==1) && (IMAGING==0)) {                   
                              r=clients[i].radar+2;

                              freq_in= ((double)(IF_FREQ-clients[i].tfreq)) * 1000./2.0; // in Hz
                              //freq_in = 29.5*1E6;
                              if (verbose > 0 ) fprintf(stdout,"IF Out Freq:  %d %lf\n",clients[i].tfreq,freq_in);
                              load_frequency(ics660[pci_ind], r, c, freq_in);
                              load_filter_taps(ics660[pci_ind],r,c,T_rise,state_time);
                              load_phase(ics660[pci_ind],r,c,0.0);
                            }
	                  }
	                  one_shot_b(ics660[pci_master]);
                          gettimeofday(&t3,NULL);
                          elapsed=(t3.tv_sec-t2.tv_sec)*1E6;
                          elapsed+=(t3.tv_usec-t2.tv_usec);
                          if (verbose > 0) fprintf(stdout,"  DDS Set Filter and Freq  Elapsed Microseconds: %ld\n",elapsed);
                          if (verbose > 1) fprintf(stdout,"Done setting the beam\n");	

                        } else {
                          /* SET freq, filter and BEAM CODE for IMAGING */
                          /* SET freq and filter for IMAGING */
                          if (new_seq_flag) { 
//                          if (1) { 
                              gettimeofday(&t2,NULL);
                              msi_timing_sequence(numclients,max_seq_count,clients,&seq_count,seq_buf,active,ics660);
                            //fprintf(stdout,"DDS:: %d %d :: msi_timing : \n",t2.tv_sec,t2.tv_usec);
                              if (verbose > 0) {
                                gettimeofday(&t3,NULL);
                                elapsed=(t3.tv_sec-t2.tv_sec)*1E6;
                                elapsed+=(t3.tv_usec-t2.tv_usec);
                                if (verbose > 0 )  fprintf(stdout,"  DDS IMAGING Timing Elapsed Microseconds: %ld\n",elapsed);
                              }
                              /* Required one shot */
	                    //one_shot_b(ics660[pci_master]);
                            } else {
                              /* Use old timing sequence */                          
                          } 
                          
                          gettimeofday(&t2,NULL);
	                  pci_ind=0;
                          for (ant=0;ant<MAX_TRANSMITTERS;ant++){
                            pci_ind=transmitters[ant][0];
                            chip=transmitters[ant][1];
                            for (cc=1;cc<=DDS_MAX_CHANNELS;cc++){
                                load_frequency(ics660[pci_ind], chip, cc, 0.0);
	                        //one_shot_b(ics660[pci_master]);
                                //load_phase(ics660[pci_ind],chip,cc,0.0);
	                        //one_shot_b(ics660[pci_master]);
                            }
                          }
	                  for( i=0; i<numclients; i++) {
	                    freq_in= (double)clients[i].tfreq * 1000.; // in Hz
                            T_rise=clients[i].trise;
	                    r= clients[i].radar;
	                    c= clients[i].channel;
                            if (verbose>2) fprintf(stdout," Loading %d freq: %lf chip: %d channel: %d\n",i,freq_in,r,c);
                            if ((clients[i].tbeam >=0) && (clients[i].tbeam < 16)) { 
                              delta=calculate_delta(freq_in,beamdirs_rad[clients[i].tbeam],d);
                            } else {
                              delta=0.0;
                            }
                            for(ant=0;ant<MAX_TRANSMITTERS;ant++) {
                              phase=delta*((double)ant-7.5);
                              pci_ind=transmitters[ant][0];
                              chip=transmitters[ant][1];
                              for(cc=1;cc<=DDS_MAX_CHANNELS;cc++) {
                                if(active[r-1][cc-1]==(c-1)) {
                                  gettimeofday(&t3,NULL);
                                  load_frequency(ics660[pci_ind], chip, cc, freq_in);
	                          //one_shot_b(ics660[pci_master]);
                                  load_phase(ics660[pci_ind],chip,cc,phase);
	                          //one_shot_b(ics660[pci_master]);
                                  load_filter_taps(ics660[pci_ind],chip,cc,T_rise,state_time);
	                          //one_shot_b(ics660[pci_master]);
                                }
                              }
                            }
	                  }
                          one_shot_b(ics660[pci_master]);
                          gettimeofday(&t3,NULL);
                          elapsed=(t3.tv_sec-t2.tv_sec)*1E6;
                          elapsed+=(t3.tv_usec-t2.tv_usec);
                          if (verbose > 0) fprintf(stdout,"  DDS Set Filter and Freq  Elapsed Microseconds: %ld\n",elapsed);
                          if (verbose > 1) fprintf(stdout,"Done setting the beam\n");	
                        } 
                        msg.status=0;
                        numclients=0;
                        max_seq_count=0;
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
                        gettimeofday(&t1,NULL);
                        elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
                        elapsed+=(t1.tv_usec-t0.tv_usec);
                        if (verbose > 1) fprintf(stdout,"  DDS Pretrigger Elapsed Microseconds: %ld\n",elapsed);
                        if (verbose > 1) fprintf(stdout,"Ending Pretrigger Setup\n");
                        break; 

/*		      case DDS_TRIGGER:
			if (verbose > 0 ) fprintf(stdout,"Setup DDS Card trigger\n");	
			if (verbose > 1) fprintf(stdout,"Read msg struct from tcp socket!\n");	
                        numclients=0;
                        max_seq_count=0;
                        rval=send_data(msgsock, &msg, sizeof(struct DriverMsg));
			if (verbose > 0 ) fprintf(stdout,"End Timing Card trigger\n");	
                        break;
*/
		      default:
			if (verbose > 0) fprintf(stderr,"BAD CODE: %c : %d\n",datacode,datacode);
			break;
		    }
		  }	
		} 
		if (verbose > 0 ) fprintf(stderr,"Closing socket\n");
		close(msgsock);
	};

        return 1;
}
