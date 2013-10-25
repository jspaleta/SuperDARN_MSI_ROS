#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#ifdef __QNX__
  #include <hw/pci.h>
  #include <hw/inout.h>
  #include <sys/neutrino.h>
  #include <sys/mman.h>
#endif
#include "plx_defines.h"
#include "control_program.h"

int build_RXFE_EEPROM_address(struct RXFESettings settings){
  int code=15;
  if (settings.att1==1) code-=1;
  if (settings.att2==1) code-=2;
  if (settings.att3==1) code-=4;
  if (settings.att4==1) code-=8;
  if (settings.amp1==1) code+=16;
  if (settings.amp2==1) code+=64;
  if (settings.amp3==1) code+=128;
  if (settings.ifmode==1) code+=32;
  return code;
}

int set_RXFE_EEPROM_address(unsigned int  base, unsigned int  address){
#ifdef __QNX__
        out8(base+PB_GRP_4,address);
        delay(1);
#endif
  return 0;
}
int read_RXFE_EEPROM_address(int  base, int cardnum, unsigned int  address){
        int temp=0;
        
        if ( (cardnum > 32) || (cardnum < 0)){
                printf("ERROR: Card numnber must be between 0 and 32\n");
        }
        cardnum=cardnum & 0x1f;
        cardnum=cardnum << 1;
#ifdef __QNX__
        // set port A as input
        out8(base+CNTRL_GRP_4,0x90);
        delay(1);
        // select address to read
        set_RXFE_EEPROM_address(base, address);
        // set R/W bit low
        out8(base+PC_GRP_4,cardnum);
        delay(1);
        temp=in8(base+PA_GRP_4);
#endif
        return temp;

}

int write_RXFE_EEPROM_address(int  base, int cardnum, unsigned int  address, int Adata){

        int temp;
        int WE_on=0x01, RW_on=0x40;

        if ( (cardnum > 32) || (cardnum < 0)){
                printf("ERROR: Card numnber must be between 0 and 32\n");
        }
        cardnum=cardnum & 0x1f;
        cardnum=cardnum << 1;
#ifdef __QNX__
        // set all groups as output
        out8(base+CNTRL_GRP_4,0x80);
        set_RXFE_EEPROM_address(base, address);
        // set R/W bit to write
        out8(base+PC_GRP_4,RW_on | cardnum);
        // set data to write
        out8(base+PA_GRP_4,Adata);
        // set WE bit high
        out8(base+PC_GRP_4,RW_on | WE_on | cardnum);
        // set WE bit low
        out8(base+PC_GRP_4,RW_on | cardnum);
        delay(1);
        // set port A as input
        out8(base+CNTRL_GRP_4,0x90);
        set_RXFE_EEPROM_address(base, address);
        // set R/W bit low
        out8(base+PC_GRP_4,cardnum);
        //delay(1000);
        delay(1);
        temp=in8(base+PA_GRP_4);

        if (temp != Adata){
                printf("WRITE ERROR\n");
        }
        printf("%d was read on port A\n", temp);
#endif
}

