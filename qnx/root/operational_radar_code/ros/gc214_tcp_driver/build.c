
#include "gc4016.h"


void buildGC4016Global(struct GC4016Global *ptr) {
  //ptr->nchanout=0x23; /* one filter */     
  ptr->nchanout=0x27; /* two filters */     
  ptr->nmult=0x07;   /* Use this setting with resampler, NMULT=8 */
  ptr->filtslct=0x50;   /*Use this setting with resampler, filter0 for chans A,B, filter 1 for chans C,D */
  ptr->finalshft=0x15;
  ptr->chanmap=0xe4; 
  ptr->addto=0x70; 
  ptr->resampclkdvd=0x00;
  ptr->ratiomap=0xe4;
  ptr->ratio0=0x04000000;
  ptr->ratio1=0x04000000;
  ptr->ratio2=0x04000000;
  ptr->ratio3=0x04000000;
  ptr->cntbyte0=0x00;
  ptr->cntbyte1=0x00;
  ptr->tricntrl=0xff;
  ptr->outformat=0x49;
  ptr->outmode=0x6c;
  ptr->outframecntrl=0xc7;
  ptr->outwdsize=0xef;
  ptr->outclkcntrl=0x01;
  ptr->sermuxcntrl=0xe4;
  ptr->outtaga=0x10;
  ptr->outtagb=0x32;
  ptr->outtagc=0x54;
  ptr->outtagd=0x76;
}

void buildGC4016Channel(struct GC4016Channel *ptr) {
   ptr->chreset=0x0c;     /* USE_SHIFT=1, SHIFT=4 */
   ptr->phase=0x0000;
   ptr->freq=3446578694L;  /* freq */
   ptr->decratio=0x703f;  /* 0x7000 | decimation -1 */
   ptr->cicscale=0x64;    /* cicscale */
   ptr->splitiq=0x00;
   ptr->cfir=0x00;        /* 0x00 | coarse<<4 */
   ptr->pfir=0x00;  
   ptr->input=0x0F;       /* 0x01 */
   ptr->peakcntrl=0x00;
   ptr->peakcount=0x00;
   ptr->finegain=0x042e;  /* finegain*/
   ptr->rate=3333.33;
   ptr->match=0;
   ptr->samples=0;
}


