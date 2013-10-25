#include <sys/types.h>
#include <hw/inout.h>
#include <sys/socket.h>
#include <sys/neutrino.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <math.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "_structures.h"
#include "_regs_PLX9052.h"

/*-SET_TM--------------------------------------------------------------*/
int set_tm(unsigned int base,int mode){
        
        int temp;
        struct  timespec nsleep;

        nsleep.tv_sec=0;
        //nsleep.tv_nsec=100000;
        nsleep.tv_nsec=5000;

    // Shift the tm mode to the correct bit in the PortB address
        mode = mode << 7;
    // check for other bits in CH0, PortB that may be on
        temp=in8(base+PB_GRP_0);
        temp=temp & 0x7F;
    // add other bit of CH0, PortB to the TM address
        mode=mode+temp;

    // send to Chan0 PortB
        out8(base+PB_GRP_0,mode);
        //nanosleep(&nsleep,NULL);
        usleep(5);
    // verify the output
        temp=in8(base+PB_GRP_0);
        if (temp==mode) return 0;
        else{
                //fprintf(stderr,"TM Mode ERROR - requested mode not sent\n");
                //fprintf(stderr," mode=%x\n", temp);
                return -1;
        }

}

