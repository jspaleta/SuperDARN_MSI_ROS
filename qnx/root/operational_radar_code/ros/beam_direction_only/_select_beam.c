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
#include "control_program.h"
#include "global_server_variables.h"
#include "include/plx_functions.h"
#include "include/plx_defines.h"
#include "utils.h"

extern int verbose;
/*
extern uint32_t use_beam_table;
extern int32_t *final_beamcodes[MAX_RADARS][32],*final_attencodes[MAX_RADARS][32];
extern double *final_freqs[MAX_RADARS][32],*final_angles[MAX_RADARS][32];
extern double f0[MAX_RADARS],fm[MAX_RADARS],df[MAX_RADARS];
extern int32_t num_freqs[MAX_RADARS],max_angles[MAX_RADARS],num_angles[MAX_RADARS],num_beamcodes[MAX_RADARS],num_fsteps[MAX_RADARS],fstep[MAX_RADARS],foffset[MAX_RADARS],num_cards[MAX_RADARS];
*/
int lookup_beamcode_by_freq(int r, double freq_mhz,double beamnm){
  int beamcode=beamnm;
  int b,f,a,best_fstep;
  double tdiff,fdiff,best_freq;
  beamcode=beamnm;
}

int lookup_angle(int r, double freq,double angle){
/*
  double df,dangle;
  int best_freq_index,best_angle_index, beam_address;
  df=freqs[r][1]-freqs[r][0];
  best_freq_index=(int)(freq-freqs[r][0])/df;
  if (best_freq_index >= num_freqs[r])  best_freq_index=num_freqs[r]-1;
  if (best_freq_index < 0)  best_freq_index=0;

  dangle=angles[1]-angles[0];
  best_angle_index=(int)(angle-angles[r][0])/dangle;
  if (best_angle_index >= num_angles[r])  best_angle_index=num_angles[r]-1;
  if (best_angle_index < 0)  best_angle_index=0;
  beam_address=best_angle_index+best_freq_index*num_freqs[r]+angle_index_offset[r];
  return beam_address;
*/
}

/*-BEAM_CODE---------------------------------------------------------*/
int reverse_bits(int data){
        
        int temp=0;
        
        temp=temp + ((data & 1)  << 12);
        temp=temp + ((data & 2)  << 10);
        temp=temp + ((data & 4)  << 8);
        temp=temp + ((data & 8)  << 6);
        temp=temp + ((data & 16)  << 4);
        temp=temp + ((data & 32)  << 2);
        temp=temp + ((data & 64)  << 0);
        temp=temp + ((data & 128)  >> 2);
        temp=temp + ((data & 256)  >> 4);
        temp=temp + ((data & 512)  >> 6);
        temp=temp + ((data & 1024)  >> 8);
        temp=temp + ((data & 2048)  >> 10);
        temp=temp + ((data & 4096)  >> 12);

        return temp;
}


int _select_beam(unsigned int base,struct ControlPRM *client){

        /* This code selects the beam code to use.  
        */
        
        int code, beamnm, temp, oldB, oldC,hi,lo;
        double freq_mhz,angle;

        unsigned int portA,portB,portC,cntl;        
        if (verbose > 1) { 
          printf("DIO: Select beam\n",client->tfreq);	
          printf("  Base Beamnm: %d\n",client->tbeam);	
          printf("  Selected Freq [kHz]: %d\n",client->tfreq);	
          printf("  Selected Radar: %d\n",client->radar);	
          printf("  Selected Beam angle: %f\n",client->tbeamazm);	
          printf("  Selected Beam width: %f\n",client->tbeamwidth);	
        }  
        /* the beam number is 4 bits.  This number
           uses (lo) bits 5-6 of CH0, PortB , and (hi) bits 6-7 of CH0, PortC
           to output the beam number. Note: The beam number is used in addressing
           the old style phasing matrix.
        */
        beamnm=client->tbeam;
        if ( (beamnm>MAX_BEAM) || (beamnm<0) ){
                fprintf(stderr,"INVALID BEAMNM - must be between 0 and %d\n",MAX_BEAM);
                beamnm=0;
                lo=0;
 		hi=0;
        } else {
        	lo=beamnm & 0x3;
        	hi=beamnm & 0xC;
		hi=hi >> 2;
        }

        freq_mhz=client->tfreq*1E3;
        /* the beam code is 13 bits, pAD0 thru pAD12.  This code
           uses bits 0-7 of CH0, PortA, and bits 0-4 of CH0, PortB
           to output the beam code. Note: The beam code is an address
           of the EEPROMs in the phasing cards.  This code is broadcast
           to ALL phasing cards.  If you are witing the EEPROM, then this
           be the beam code you are writing
        */
        /* Lookup beamcode using phasing card lookup table */
	/* 
         * MSI phasing cards programmed with 
         * frequency optimal attenuatin values
         * in different bands.
         */ 
        code=lookup_beamcode_by_freq(client->radar-1,freq_mhz,beamnm);

        if (verbose > 1) 
          printf("  Selected Beamcode: %d\n",code);	

        if (verbose > 1) printf("Reversing Code: %d\n",code);	
    // bit reverse the code
        code=reverse_bits(code);

    // check if beam code is reasonable
        if ( (code>8192) | (code<0) ){
                fprintf(stderr,"INVALID BEAM CODE - must be between 0 and 8192\n");
                return -1;
        }
    if (client->radar==1) {
      if (verbose > 1) printf("Selecting Radar 1 client block");
      portA=PA_GRP_0;
      portB=PB_GRP_0;
      portC=PC_GRP_0;
    }
    if (client->radar==2) {
      if (DEVICE_ID==0x0c78) {
        if (verbose > 1) printf("Selecting Radar 2 port block");
        portA=PA_GRP_2;
        portB=PB_GRP_2;
        portC=PC_GRP_2;
      } else {
        if (verbose > 1) printf("Radar 2 port block not available..using Radar 1 ports instead");
        portA=PA_GRP_0;
        portB=PB_GRP_0;
        portC=PC_GRP_0;
      }
    } 
#ifdef __QNX__
   // set CH0, Port A to lowest 8 bits of beam code and output on PortA
       temp=code & 0xff;
       out8(base+portA,temp);
   // set CH0, Port B to upper 5 bits of beam code and output on PortB
       temp=code & 0x1f00;
       temp=temp >> 8;
       out8(base+portB,temp);

   // verify that proper beam code was sent out
       temp=in8(base+portB);
       temp=(temp & 0x1f) << 8;
       temp=temp+in8(base+portA);
       if (temp==code) return 0;
       else{
               fprintf(stderr,"BEAM CODE OUTPUT ERROR - requested code not sent\n");
               return -1;
       }


/*
    // set CH0, Port A to lowest 8 bits of beam code and output on PortA

        temp=code & 0xff;
        out8(base+portA,temp);
    // save the upper bit (TM status) of Port B
        oldB=in8(base+portB);
        oldB=oldB & 0x80;
    // set CH0, lowest 5 bits on Port B to upper 5 bits of beam code
        temp=code & 0x1f00;
        temp=temp >> 8;
    // set CH0, bits 5-6 on Port B to lo bits of beamnum
        lo=lo << 5;
    // write code segment and saved bits back to Port B
        temp=oldB+temp+lo;
        out8(base+portB,temp);
        if (verbose > 1) printf("Port B code %d\n",temp);
    // save the lowest 6 bits of Port C
        oldC=in8(base+portC);
        oldC=oldC & 0x3F;
    // set CH0, bits 6-7 on Port C to hi bits of beamnum
        hi=hi << 6;
    // write code segment and saved bits back to Port B
        temp=oldC+hi;
        out8(base+portC,temp);
        if (verbose > 1 ) printf("Port C code %d\n",temp);

    // verify that proper beam code was sent out
        lo=in8(base+portB);
        lo=(lo & 0x60) >> 5;
        hi=in8(base+portC);
        hi=(hi & 0xC0) >> 4;
        temp=hi+lo;
        if (temp==beamnm) return 0;
        else{
                fprintf(stderr,"BEAMNM OUTPUT ERROR - requested beamnm not sent \n");
                fprintf(stderr,"%d %d %d %d\n",hi,lo,temp,beamnm);

                return -1;
        }

    // verify that proper code was sent out
        temp=in8(base+portB);
        temp=(temp & 0x1f) << 8;
        temp=temp+in8(base+portA);
        if (temp==code) return 0;
        else{
                fprintf(stderr,"BEAM CODE OUTPUT ERROR - requested code not sent \n");
                return -1;
        }
*/
#endif
        return code;
       
}



