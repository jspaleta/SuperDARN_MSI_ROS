/* normalscan.c
   ============
   Author: R.J.Barnes & J.Spaleta
*/

/*
 LICENSE AND DISCLAIMER
 
 Copyright (c) 2012 The Johns Hopkins University/Applied Physics Laboratory
 
 This file is part of the Radar Software Toolkit (RST).
 
 RST is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version.
 
 RST is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with RST.  If not, see <http://www.gnu.org/licenses/>.
 
 
  
*/


#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>
#include "rtypes.h"
#include "option.h"
#include "rtime.h"
#include "dmap.h"
#include "limit.h"
#include "radar.h"
#include "rprm.h"
#include "iq.h"
#include "rawdata.h"
#include "fitblk.h"
#include "fitdata.h"
#include "fitacf.h"


#include "errlog.h"
#include "freq.h"
#include "tcpipmsg.h"

#include "rmsg.h"
#include "rmsgsnd.h"

#include "radarshell.h"

#include "build.h"
#include "global.h"
#include "reopen.h"
#include "setup.h"
#include "sync.h"

#include "site.h"
#include "sitebuild.h"
#include "siteglobal.h"
#include "rosmsg.h"
#include "tsg.h"
char server[256]="127.0.0.1";
int  port=0;
int  sock=0;

void CP_Exit(int signum);

char *ststr=NULL;
char *dfststr="tst";

void *tmpbuf;
size_t tmpsze;

char progid[80]={"transponder_pulse"};
char progname[256];

int arg=0;
struct OptionData opt;

char *roshost=NULL;
char *droshost={"127.0.0.1"};
int exitflag=0;

void CP_Exit(int signum) {

  struct ROSMsg msg;
  switch(signum) {
    case 2:
      if (debug) printf("CP_Exit: Sig %d: %d\n",signum,exitflag); 
      cancel_count++;
      exitflag=signum;
      if (cancel_count < 3 )
        break;
    case 0:
      if (debug) printf("CP_Exit: Sig %d: %d\n",signum,exitflag); 
      if(exitflag!=0) {
        msg.type=QUIT;
        TCPIPMsgSend(sock, &msg, sizeof(struct ROSMsg));
        TCPIPMsgRecv(sock, &msg, sizeof(struct ROSMsg));
        if (debug) {
          fprintf(stderr,"QUIT:type=%c\n",msg.type);
          fprintf(stderr,"QUIT:status=%d\n",msg.status);
        }
        close(sock);
        exit(errno);
      } 
      break;
    default:
      if (debug) printf("CP_Exit: Sig %d: %d\n",signum,exitflag); 
      if(exitflag==0) {
        exitflag=signum;
      }
      if(exitflag!=0) {
        msg.type=QUIT;
        TCPIPMsgSend(sock, &msg, sizeof(struct ROSMsg));
        TCPIPMsgRecv(sock, &msg, sizeof(struct ROSMsg));
        if (debug) {
          fprintf(stderr,"QUIT:type=%c\n",msg.type);
          fprintf(stderr,"QUIT:status=%d\n",msg.status);
        }
        close(sock);
        exit(errno);
      }

      break;
  }
}


int main(int argc,char *argv[]) {
/*  here is the timing sequence */
/*
 * each bit in the code value can represents a different digital i/o line 
 * that you can potentially control.
 * for this example I'm just turning all the bits on during the pulse:
 *  100 repeated clock periods with code==0x00
 *  200 repeated clock periods with code==0xFF
 *  100 repeated clock periods with code==0x00
 */
  struct SeqPRM   tprm;
  unsigned char rep[7] = {   20,   10, 180,   200,   10,   10};
  unsigned char code[7] = {0x80, 0x02, 0x06, 0x06, 0x02, 0x00};

  struct ROSMsg smsg,rmsg;
  int exitpoll=0;
  int single=0,scannowait=0;
  int skip=0;
  int status=0;
  int fixfrq=-1;


  /* ========= PROCESS COMMAND LINE ARGUMENTS ============= */

  OptionAdd( &opt, "fixfrq", 'i', &fixfrq);  /* lets you set a frequency */
  OptionAdd( &opt,"ros",'t',&roshost);  /* lets you override the ip address of the ros server */
  OptionAdd( &opt,"stid",'t',&ststr);  /* lets you override the superdarn stid */
  OptionAdd( &opt, "single", 'x', &single); /* test program to do just one timing sequence */
  OptionAdd( &opt,"sb",'i',&sbm); /* start beam number */
  OptionAdd( &opt,"eb",'i',&ebm); /* end beam number */
  OptionAdd( &opt,"c",'i',&cnum); /* radar channel number defaults to 1 */
  OptionAdd( &opt,"r",'i',&rnum); /* radar number defaults to 1 */

  arg=OptionProcess(1,argc,argv,&opt,NULL);  
 
  if (ststr==NULL) ststr=dfststr;

  if (roshost==NULL) roshost=getenv("ROSHOST");
  if (roshost==NULL) roshost=droshost;


/*
  Lets initialize some things here.
*/

  signal(SIGPIPE,CP_Exit);
  signal(SIGINT,CP_Exit);
  signal(SIGUSR1,CP_Exit);

  debug=0;  /*set this to 1 if you want some extra stderr messages */
  tfreq=12000;  /* defaults to 12000 kHz, can be overridden with cmdline arg */

  exitflag=0;
  cancel_count=0;
  sock=0;
  strcpy(server,roshost);
  port=45000;
  fprintf(stderr,"ROS server:%s\n",roshost);


/* Radar number to register */
  rnum=1;
/* Channel number to register */
  cnum=1;
/* Beam Scan Direction settings */
  backward=0;
  sbm=0;
  ebm=15;

/* reporcess the cmdline arguments */
  arg=OptionProcess(1,argc,argv,&opt,NULL);  



/* 
  status=SiteSetupRadar();
*/
/* lets tell the ros which radar and channel we want to use*/

  int32 temp32;
  if ((sock=TCPIPMsgOpen(server,port)) == -1) {
    fprintf(stderr,"ROS Server Socket Open Error: %d\n",sock);
    return -1;
  }
  fprintf(stderr,"Rnum: %d Cnum: %d\n",rnum,cnum);
  smsg.type=SET_RADAR_CHAN;
  TCPIPMsgSend(sock, &smsg,sizeof(struct ROSMsg)); 
  temp32=rnum;
  TCPIPMsgSend(sock, &temp32, sizeof(int32)); 
  temp32=cnum;
  TCPIPMsgSend(sock, &temp32, sizeof(int32));
  TCPIPMsgRecv(sock, &rmsg, sizeof(struct ROSMsg)); 
  if (rmsg.status < 0) {
    fprintf(stderr,"Requested radar channel unavailable\nSleeping 1 second and exiting\n");
    sleep(1);
    CP_Exit(-1);
  } 
  if (debug) {
    fprintf(stderr,"SET_RADAR_CHAN:type=%c\n",rmsg.type);
    fprintf(stderr,"SET_RADAR_CHAN:status=%d\n",rmsg.status);
  }


/* Lets register our timing sequence */
  fprintf(stdout,"Preparing SiteTimeSeq Station ID: %s  %d\n",ststr,stid);
  fflush(stdout);
  tprm.index=0;
  tprm.step=CLOCK_PERIOD;
  tprm.samples=0;
  tprm.smdelay=0;
  tprm.len=7;
  smsg.type=REGISTER_SEQ;
  TCPIPMsgSend(sock, &smsg, sizeof(struct ROSMsg));
  TCPIPMsgSend(sock, &tprm, sizeof(struct SeqPRM));
  TCPIPMsgSend(sock, &rep, sizeof(unsigned char)*tprm.len);
  TCPIPMsgSend(sock, &code, sizeof(unsigned char)*tprm.len);
  TCPIPMsgRecv(sock, &rmsg, sizeof(struct ROSMsg));

  fprintf(stdout,"Get Initial Radar Parameter from ROS server\n");
  smsg.type=GET_PARAMETERS;
  TCPIPMsgSend(sock,&smsg,sizeof(struct ROSMsg));
  TCPIPMsgRecv(sock,&rprm,sizeof(struct ControlPRM));
  TCPIPMsgRecv(sock,&rmsg,sizeof(struct ROSMsg));

/* lets loop over beam direction and have the ROS send out our timing sequence*/
  printf("Entering Scan loop Station ID: %s  %d\n",ststr,stid);
  do {
    
    if (backward) {
      bmnum=sbm-skip;
      if (bmnum<ebm) bmnum=sbm;
    } else {
      bmnum=sbm+skip;
      if (bmnum>ebm) bmnum=sbm;
    }

    do {

      TimeReadClock(&yr,&mo,&dy,&hr,&mt,&sc,&us);
      fprintf(stdout," %04d-%02d-%02d %02d:%02d:%02d.%06d\n",yr,mo,dy,hr,mt,sc,us);
      CP_Exit(0);
      
      if(fixfrq>0) {
        tfreq=fixfrq;
      }
/* lets set a subset of the rprm structure's parameters */
      rprm.tbeam=bmnum;  /*sets the beam direction */
      rprm.tfreq=tfreq;  /*sets the transmit freq */ 
      rprm.rfreq=tfreq;  /*sets the recv freq */
      rprm.trise=5000;   /* sets the rise time associted with the transmit pules shape */
      rprm.baseband_samplerate=((double)nbaud/(double)txpl)*1E6;    /* recv card sample rate, in Hz */
      rprm.filter_bandwidth=rprm.baseband_samplerate;  /* recv filter bandwidth */
      rprm.match_filter=1;  /* use match filtering */
      rprm.number_of_samples=0;  /* number of recv samples to collect */
      rprm.priority=1;  /* priority to be used when you have multiple channels sharing parameters leave at 1 */
      rprm.buffer_index=0; /* buffer index for swing buffering, not fully implemented just leave to 0 */

/* Now lets send the parameter structure over the wire */
      fprintf(stdout,"Setting Parameters\n");
      smsg.type=SET_PARAMETERS;
      TCPIPMsgSend(sock,&smsg,sizeof(struct ROSMsg));
      TCPIPMsgSend(sock,&rprm,sizeof(struct ControlPRM));
      TCPIPMsgRecv(sock,&rmsg,sizeof(struct ROSMsg));
/* Lets tell the ROS this controlprogram is ready for trigger */
      fprintf(stdout,"Setting Ready for trigger\n");
      smsg.type=SET_READY_FLAG;
      TCPIPMsgSend(sock,&smsg,sizeof(struct ROSMsg));
      TCPIPMsgRecv(sock,&rmsg,sizeof(struct ROSMsg));

/* Here is where we would run GET_DATA and wait for data to be ready */
      fprintf(stdout,"Wait for Recv Data here:");
      fprintf(stdout,"Get Parameters\n");
      smsg.type=GET_PARAMETERS;
      TCPIPMsgSend(sock,&smsg,sizeof(struct ROSMsg));
      TCPIPMsgRecv(sock,&rprm,sizeof(struct ControlPRM));
      TCPIPMsgRecv(sock,&rmsg,sizeof(struct ROSMsg));


      if (exitpoll !=0) break;
      if (bmnum==ebm) break;
      if (backward) bmnum--;
      else bmnum++;
    } while (1);

    if (single!=0) exitpoll=1;
    
  } while (exitpoll==0);

  CP_Exit(0);

  return 0;   
} 
 
