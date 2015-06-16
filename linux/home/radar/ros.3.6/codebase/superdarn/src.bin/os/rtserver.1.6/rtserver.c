/* fitacfwrite
    =========== 
   Author: R.J.Barnes
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

#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <zlib.h>
#include "rtypes.h"
#include "option.h"
#include "connex.h"
#include "tcpipmsg.h"

#include "dmap.h"
#include "rprm.h"
#include "fitdata.h"
#include "radar.h"
#include "rmsg.h"
#include "rmsgrcv.h"
#include "errlog.h"

#include "fitpacket.h"
#include "fork.h"

#include "errstr.h"
#include "hlpstr.h"

#include "version.h"

#include "rtserver.h"

#define DEF_PORT 41103

struct OptionData opt;

struct RMsgBlock rblk;
unsigned char *store=NULL; 

struct DMsg {
  int tag;
  void *pbuf;
  void *fbuf;
};

int dnum=0;
int dptr=0;
struct DMsg dmsg[32];


struct RadarParm *prm;
struct FitData *fit;

char *taskname="rtserver";

char *errhost=NULL;
char *derrhost="127.0.0.1";
int errport=44000;
int errsock=-1;

char errbuf[1024];
int outpipe=-1;
int channel=-1;

/*
Operate:
Called after the client connects parses fit and prm data
@param:
sock: msgsock of the server
port: server port
run: indicates if it is the first run or subsequent calls
*/
int operate(int sock,int port,int run) {
 
  int s,i;
  int msg,rmsg;
  errsock = -1;
  unsigned char *cbufadr=NULL;
  size_t cbuflen;


  unsigned char *bufadr=NULL;
  int bufsze;
  if(run ==0){
  	  outpipe=forkinet(port);
  }
/* JDS required to avoid rtserver race condition 
that deadlocks controlprograms */
  sleep(1);

  while(1) {

    s=TCPIPMsgRecv(sock,&msg,sizeof(int));

    if (s !=sizeof(int)) break;
    rmsg=TASK_OK;
    switch (msg) {
    
    case TASK_OPEN:
      ErrLog(errsock,taskname,"Opening file.");
      rmsg=RMsgRcvDecodeOpen(sock,&cbuflen,&cbufadr);
      break;
    case TASK_CLOSE:
      ErrLog(errsock,taskname,"Closing file.");
      break;
    case TASK_RESET:
      ErrLog(errsock,taskname,"Reset.");
      break;
    case TASK_QUIT:
      ErrLog(errsock,taskname,"Stopped.");
      TCPIPMsgSend(sock,&rmsg,sizeof(int));
      exit(0);
      break;
    case TASK_DATA:
      ErrLog(errsock,taskname,"Received Data.");
      rmsg=RMsgRcvDecodeData(sock,&rblk,&store);    
    default:
      break;  
    }
    TCPIPMsgSend(sock,&rmsg,sizeof(int));

    if (msg==TASK_DATA) {
      dnum=0;
      for (i=0;i<rblk.num;i++) {
        for (dptr=0;dptr<dnum;dptr++) 
          if (dmsg[dptr].tag==rblk.data[i].tag) break;
          if (dptr==dnum) {
            dmsg[dptr].tag=rblk.data[i].tag;
            dmsg[dptr].pbuf=NULL;
            dmsg[dptr].fbuf=NULL;
            dnum++;
          }
         switch (rblk.data[i].type) {
         	case PRM_TYPE:
         		dmsg[dptr].pbuf=rblk.ptr[rblk.data[i].index];
         		break;
         	case FIT_TYPE:
         		dmsg[dptr].fbuf=rblk.ptr[rblk.data[i].index];
         		break;
         	default:
         		break;
         }
      }


      for (dptr=0;dptr<dnum;dptr++) {

      	  if (dmsg[dptr].pbuf==NULL) continue;
      	  if (dmsg[dptr].fbuf==NULL) continue;

      	  s = RadarParmExpand(prm,dmsg[dptr].pbuf);

      	  if (s !=-1){ 
      	  	  s = FitExpand(fit,prm->nrang,dmsg[dptr].fbuf);
      	  }
      	  else{
      	  	fit = NULL;  
      	  }

      	  if ((channel !=-1) && (prm->channel !=0)) {
      	  	  if ((channel==1) && (prm->channel==2)) continue;
      	  	  if ((channel==2) && (prm->channel!=2)) continue;
      	  }
      	  if (outpipe==-1) {
      	  	  sprintf(errbuf,"Child process died - Restarting.");
      	  	  ErrLog(errsock,taskname,errbuf);  
      	  	  if(run ==0){
      	  	  	  outpipe=forkinet(port);
      	  	  }
      	  }
      	  return 0;
      	}
   
      if (store !=NULL) free(store);
      store=NULL;
    }  
  }

  return 0;
}

/*
Initialize
The old main method sets up the server to the 
ports recieved in argv and listens for other clients to connect.
Once the other clients connect once connected the operate method
is called
@params run: the msgsock number. The initialize method is called 
	repeatedly after the server is already initialized and it indicates
	to skip the initalization and send in the msgsock
	argc and *argv[]:takes in command line arguments
@return msgsock
*/
int initialize(int run,int argc, char *argv[]) {
  
  int rport=DEF_PORT,tport=1024,arg=0;
  int sock,s;
  int sc_reuseaddr=1,temp;
  unsigned char help=0; 
  unsigned char option=0; 
  socklen_t length;
  socklen_t clength;
  int outpipe = -1;
  struct sockaddr_in server;
  struct sockaddr_in client;
  if(run ==0){
	  fd_set ready;
	  struct hostent *gethostbyname();
	  pid_t root;
	
	  char *chstr=NULL;
	
	  int msgsock=0;	
	
	  prm=RadarParmMake();
	  fit=FitMake();

	  OptionAdd(&opt,"-help",'x',&help);
	  OptionAdd(&opt,"-option",'x',&option);
	
	  OptionAdd(&opt,"rp",'i',&rport);
	  OptionAdd(&opt,"tp",'i',&tport);
	
	  OptionAdd(&opt,"eh",'t',&errhost);
	  OptionAdd(&opt,"ep",'i',&errport);
	
	  OptionAdd(&opt,"c",'t',&chstr);
	
	  arg=OptionProcess(0,argc,argv,&opt,NULL);
	
	  if (help==1) {
		OptionPrintInfo(stdout,hlpstr);
		exit(0);
	  }
	
	  if (option==1) {
		OptionDump(stdout,&opt);
		exit(0);
	  }
	
	  if (chstr !=NULL) {
		if (tolower(chstr[0])=='a') channel=1;
		if (tolower(chstr[0])=='b') channel=2;
	  }
	
	
	  if (errhost==NULL) errhost=derrhost;
	  errsock=TCPIPMsgOpen(errhost,errport);
	
	  sprintf(errbuf,"Started (version %s.%s) listening on port %d for control program, and on port %d for clients",
			  MAJOR_VERSION,MINOR_VERSION,rport,tport);
	  ErrLog(errsock,taskname,errbuf);  
	
	
	  signal(SIGCHLD,SIG_IGN); 
	  signal(SIGPIPE,SIG_IGN);
	
	  root=getpid();
	
	  sock=socket(AF_INET,SOCK_STREAM,0); /* create our listening socket */
	  if (sock<0) {
		perror("opening stream socket");
		exit(1);
	  }
	
	  /* set socket options */
	  temp=setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&sc_reuseaddr,
					 sizeof(sc_reuseaddr));
	
	  /* name and bind socket to an address and port number */
	
	  server.sin_family=AF_INET;
	  server.sin_addr.s_addr=INADDR_ANY;
	  if (rport !=0) server.sin_port=htons(rport); 
	  else server.sin_port=0;
	  
	  if (bind(sock,(struct sockaddr *) &server,sizeof(server))) {
		 perror("binding stream socket");
		 exit(1);
	  }
	
	  /* Find out assigned port number and print it out */
	
	  length=sizeof(server);
	  if (getsockname(sock,(struct sockaddr *) &server,&length)) {
		 perror("getting socket name");
		 exit(1);
	  }
	  listen(sock,5); /* mark our socket willing to accept connections */
	  do {
	
		  /* block until someone wants to attach to us */
	
		  FD_ZERO(&ready);
		  FD_SET(sock,&ready);
		  if (select(sock+1,&ready,0,0,NULL) < 0) { 
		   perror("while testing for connections");
		   continue;
		  }
		 
		  /* Accept the connection from the client */
	
		  clength=sizeof(client);
		  msgsock=accept(sock,(struct sockaddr *) &client,&clength);
			
		  if (msgsock==-1) {
			 perror("accept"); 
			 continue;
		  }
	  
		  if (fork() == 0) {
			close(sock);
			s = operate(msgsock,tport,run);
			if (s!=0){
				exit(0);
        	}
        	else{
        		return msgsock;
        	}
		  }
		  close (msgsock);
	  } while(1);
	    }
       printf("Outpipe %d\n",s);
        if (s!=0){
        	exit(0);
        }
        else{
        	return run;
        }
  return run;

}

/*
Main method calls initialize
*/
int main(int argc, char *argv[]){
	
	initialize(0,argc,argv);
	return 0;
}
/*
Returns the RadarParm structure
*/
struct RadarParm* getRadarParm(){
	return prm;
}

/*
Returns lag array
*/
int16 return_lag(int index1,int index2){
	return prm->lag[index1][index2];	
}

/*
returns pulse array
*/
int16 return_pulse(int index){
	return prm->pulse[index];	
}
/*
Returns the Fit data structure
*/
struct FitData* getFitData(){
	return fit;
}

/*
returns the FitNoise structure
*/
struct FitNoise* return_noise(){
	return &fit->noise;	
}

/*
Returns the FitRange for rng or xrng
@params: 
index: array index
xflag: flag indicator if data being returned is xrng or rng
*/
struct FitRange* return_rng_xrng(int index,int xflag){
	if(xflag){
		return &fit->xrng[index];
	}
	else{
		return &fit->rng[index];	
	}
}

/*
Returns the FitElv struture
@params:
index: array index
*/
struct FitElv* return_elv(int index){
	return &fit->elv[index];
}

/*
Returns outpipe information
*/
int get_outpipe(){
	return outpipe;	
}


