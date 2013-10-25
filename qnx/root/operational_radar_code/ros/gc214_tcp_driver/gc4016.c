
#include <stdlib.h>
#include <stdio.h>
//#include <i86.h>
//#include <fcntl.h>
//#include <math.h>
//#include <sys/types.h>
//#include <sys/name.h>
//#include <sys/kernel.h>
//#include <sys/sendmx.h>
//#include <sys/sched.h>
//#include <sys/mman.h>
//#include <sys/pci.h>

//#include <signal.h>
//#include <time.h>
//#include "dma-alloc.h"
#include "registers.h"
//#include "io.h"
#include "gc4016.h"


/*
 $Log: gc4016.c,v $
 Revision 1.4  2009/10/27 21:39:01  jspaleta
 jspaleta: set resampcoeffs to short

 Revision 1.3  2009/10/26 23:36:28  jspaleta
 jspaleta: working test samples?

 Revision 1.2  2009/10/22 00:22:22  jspaleta
 jspaleta: gc214 stuff

 Revision 1.1  2009/10/19 22:55:59  jspaleta
 jspaleta: old school gc214 driver

 Revision 1.1  2005/07/25 15:12:45  barnes
 Initial revision

*/

void setupGC4016(long baseGC214,struct GC4016Global *ptr,short *resampcoeff) {
  int i,temp;
  
  write8(baseGC214+offset_GC4016+reg_GC4016_GENSYNC,0x00);
  write8(baseGC214+offset_GC4016+reg_GC4016_GLOBALRST,0xF8);
  write8(baseGC214+offset_GC4016+reg_GC4016_NCHANOUT,ptr->nchanout);
  write8(baseGC214+offset_GC4016+reg_GC4016_NMULT,ptr->nmult);
  write8(baseGC214+offset_GC4016+reg_GC4016_FILTSLCT,ptr->filtslct);
  write8(baseGC214+offset_GC4016+reg_GC4016_FINALSHFT,ptr->finalshft);
  write8(baseGC214+offset_GC4016+reg_GC4016_CHANMAP,ptr->chanmap);
  write8(baseGC214+offset_GC4016+reg_GC4016_ADDTO,ptr->addto);
  write8(baseGC214+offset_GC4016+reg_GC4016_RESAMPCLKDVD,ptr->resampclkdvd);
  write8(baseGC214+offset_GC4016+reg_GC4016_RATIOMAP,ptr->ratiomap);
  write32(baseGC214+offset_GC4016+reg_GC4016_RATIO0,ptr->ratio0);
  write32(baseGC214+offset_GC4016+reg_GC4016_RATIO1,ptr->ratio1);
  write32(baseGC214+offset_GC4016+reg_GC4016_RATIO2,ptr->ratio2);
  write32(baseGC214+offset_GC4016+reg_GC4016_RATIO3,ptr->ratio3);
  write8(baseGC214+offset_GC4016+reg_GC4016_CNTSYNC,0x00);
  write8(baseGC214+offset_GC4016+reg_GC4016_CNTBYTE0,ptr->cntbyte0);
  write8(baseGC214+offset_GC4016+reg_GC4016_CNTBYTE1,ptr->cntbyte1);
  write8(baseGC214+offset_GC4016+reg_GC4016_TRICNTRL,ptr->tricntrl);
  write8(baseGC214+offset_GC4016+reg_GC4016_OUTFORMAT,ptr->outformat);
  write8(baseGC214+offset_GC4016+reg_GC4016_OUTMODE,ptr->outmode);
  write8(baseGC214+offset_GC4016+reg_GC4016_OUTFRAMECNTRL,ptr->outframecntrl);
  write8(baseGC214+offset_GC4016+reg_GC4016_OUTWDSIZE,ptr->outwdsize);
  write8(baseGC214+offset_GC4016+reg_GC4016_OUTCLKCNTRL,ptr->outclkcntrl);
  write8(baseGC214+offset_GC4016+reg_GC4016_SERMUXCNTRL,ptr->sermuxcntrl);
  write8(baseGC214+offset_GC4016+reg_GC4016_OUTTAGA,ptr->outtaga);
  write8(baseGC214+offset_GC4016+reg_GC4016_OUTTAGB,ptr->outtagb);
  write8(baseGC214+offset_GC4016+reg_GC4016_OUTTAGC,ptr->outtagc);
  write8(baseGC214+offset_GC4016+reg_GC4016_OUTTAGD,ptr->outtagd);
  write8(baseGC214+offset_GC4016+reg_GC4016_MISC,0x00);

 
  for(i=0;i<256;i++){
    write16(baseGC214+offset_GC4016+reg_GC4016_RESAMPCOEFFS+2*i,
            resampcoeff[i]);
  }

}

void releaseGC4016(long baseGC214) {
  write8(baseGC214+offset_GC4016+reg_GC4016_GLOBALRST,0x08);
}

void setupGC4016channel(long baseGC214,int channel,
                        struct GC4016Channel *ptr,int *cfircoeffs,
                        int *pfircoeffs) {
  int i;
  int offset=0;
  
  if (channel==0) offset=offset_GC4016_A;
  if (channel==1) offset=offset_GC4016_B;
  if (channel==2) offset=offset_GC4016_C;
  if (channel==3) offset=offset_GC4016_D;
  write8(baseGC214+offset_GC4016+offset+reg_GC4016_CHRESET,ptr->chreset);
  write16(baseGC214+offset_GC4016+offset+reg_GC4016_PHASE,ptr->phase);
  write8(baseGC214+offset_GC4016+offset+reg_GC4016_FREQSYNC,0x77);
  //printf("GC214 Driver: GC4016_FREQ %d\n",ptr->freq);
  write32(baseGC214+offset_GC4016+offset+reg_GC4016_FREQ,ptr->freq);
  write8(baseGC214+offset_GC4016+offset+reg_GC4016_NCOSYNC,0x22);
  //write8(baseGC214+offset_GC4016+offset+reg_GC4016_ZEROPAD,0x00);
  write8(baseGC214+offset_GC4016+offset+reg_GC4016_ZEROPAD,0x20);
  write8(baseGC214+offset_GC4016+offset+reg_GC4016_DECSYNC,0x22);
  write16(baseGC214+offset_GC4016+offset+reg_GC4016_DECRATIO,ptr->decratio);
  write8(baseGC214+offset_GC4016+offset+reg_GC4016_CICSCALE,ptr->cicscale);
  write8(baseGC214+offset_GC4016+offset+reg_GC4016_SPLITIQ,ptr->splitiq);
  write8(baseGC214+offset_GC4016+offset+reg_GC4016_CFIR,ptr->cfir);
  write8(baseGC214+offset_GC4016+offset+reg_GC4016_PFIR,ptr->pfir);
  write8(baseGC214+offset_GC4016+offset+reg_GC4016_INPUT,ptr->input);
  write8(baseGC214+offset_GC4016+offset+reg_GC4016_PEAKCNTRL,ptr->peakcntrl);
  write16(baseGC214+offset_GC4016+offset+reg_GC4016_FINEGAIN,ptr->finegain);

  for(i=0;i<11;i++){
    write16(baseGC214+offset_GC4016+offset+reg_GC4016_CFIRCOEFFS+2*i,
            cfircoeffs[i]);
  }
  for(i=0;i<32;i++){
    write16(baseGC214+offset_GC4016+offset+reg_GC4016_PFIRCOEFFS+2*i,
            pfircoeffs[i]);
  }	
}








