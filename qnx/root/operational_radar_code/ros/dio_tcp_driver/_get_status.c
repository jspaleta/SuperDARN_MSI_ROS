#include <sys/types.h>
#ifdef __QNX__
  #include <hw/inout.h>
  #include <sys/socket.h>
  #include <sys/neutrino.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
#endif
#include <math.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "global_server_variables.h"
#include "include/plx_functions.h"
#include "include/plx_defines.h"
#include "utils.h"

extern int verbose;

int _select_tx(unsigned int base, int radar,int address){

        /* This code selects a tx to address.  
        */
        
        int temp,offset,shifted_address;
        struct  timespec nsleep;
        unsigned int portA,portB,portC;
        nsleep.tv_sec=0;
        //nsleep.tv_nsec=100000;
        nsleep.tv_nsec=5000;
        if(verbose>1) printf("Selected Address: %d\n",address);       

        if ( (radar>MAX_RADARS) | (radar<=0) ){
                fprintf(stderr,"INVALID RADAR Number %d \n", radar);
                return -1;
        }

    // check if tx address is reasonable
        if ( (address>=MAX_TRANSMITTERS) | (address<0) ){
                fprintf(stderr,"INVALID TX ADDRESS %d \n", address );
                return -1;
        }
        switch(radar) {
          case 1:
            portA=PA_GRP_1;
            portB=PB_GRP_1;
            portC=PC_GRP_1;
            break;
          case 2:
            if (DEVICE_ID==0x0c78) {
              portA=PA_GRP_3;
              portB=PB_GRP_3;
              portC=PC_GRP_3;
            } else {
              portA=PA_GRP_1;
              portB=PB_GRP_1;
              portC=PC_GRP_1;
            }
            break;
          default:
            portA=PA_GRP_1;
            portB=PB_GRP_1;
            portC=PC_GRP_1;
            break;
        }
#ifdef __QNX__
      // shift address left 4 bit (using c port high bits)
          shifted_address=address << 4;
      // check for other bits in CH1, PortC that may be on
          temp=in8(base+portC);
          temp=temp & 0xF;
      // add other bit of PortC to the address bits
          shifted_address=shifted_address+temp;
      // output the address and original other bits to PortC
          out8(base+portC,shifted_address);
          //nanosleep(&nsleep,NULL);
	  usleep(5);
      // verify the output
          temp=in8(base+portC);
          temp=((temp >> 4) & 0xF);
          if (temp==address) {
            if (verbose>1) fprintf(stdout,"TX SELECT OUTPUT SUCCEEDED - requested code is %d\n",shifted_address);
            return 0;
                
          } else {
                fprintf(stderr,"TX SELECT OUTPUT ERROR - requested code not sent\n");
                fprintf(stderr," read_code=%d sent_code=%d address=%d\n", temp,shifted_address,address);
                return -2;
          }
#else 
            return 0;

#endif
}

int _get_status(unsigned int base,int radar,struct tx_status *txstatus )
{
  int tx_address;
  int status,return_status;
  int temp;
  unsigned int portA,portB,portC;

  if ( (radar>MAX_RADARS) | (radar<=0) ){
    fprintf(stderr,"INVALID RADAR Number %d \n", radar);
    return -1;
  }

  switch(radar) {
    case 1:
      portA=PA_GRP_1;
      portB=PB_GRP_1;
      portC=PC_GRP_1;
      break;
    case 2:
      if (DEVICE_ID==0x0c78) {
        portA=PA_GRP_3;
        portB=PB_GRP_3;
        portC=PC_GRP_3;
      } else {
        portA=PA_GRP_1;
        portB=PB_GRP_1;
        portC=PC_GRP_1;
      }
      break;
    default:
      portA=PA_GRP_1;
      portB=PB_GRP_1;
      portC=PC_GRP_1;
      break;
  }
  return_status=0;
  for (tx_address=0;tx_address<MAX_TRANSMITTERS;tx_address++) {
#ifdef __QNX__
      if(verbose > 1 ) printf("TX_ADDRESS: %d\n",tx_address);
      status=_select_tx(base,radar,tx_address);
      if (status==0){
  // read group 1 PortC lo
        temp=in8(base+portC);
        if(verbose>1) printf("  Hex Temp: %x\n",temp);    
        txstatus->status[tx_address]=temp & 0xf;
        if(verbose>1) printf("4 bit Status: %x\n",txstatus->status[tx_address]);        
        txstatus->AGC[tx_address]=(temp & 4) >> 2;       
        if(verbose>1) printf("  AGC Status: %x\n",txstatus->AGC[tx_address]);    
        txstatus->LOWPWR[tx_address]=(temp & 2) >> 1;
        if(verbose>1) printf("  Low PRW Status: %x\n",txstatus->LOWPWR[tx_address]);    
      }  else {
        printf("Bad TX selection: %d %d\n",tx_address,status);    
        return_status= -1;
      }
#else
      txstatus->status[tx_address]=0xf;
      txstatus->AGC[tx_address]=1;       
      txstatus->LOWPWR[tx_address]=1;

#endif
  }
  return return_status;
}



