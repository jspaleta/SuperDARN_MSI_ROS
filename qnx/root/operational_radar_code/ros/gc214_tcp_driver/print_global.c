#include "gc4016.h"
void print_global(struct GC4016Global *ptr) {
  printf("nchanout: %x\n",ptr->nchanout);     
  printf("nmulti: %x\n",  ptr->nmult);
  printf("filtslct: %x\n",ptr->filtslct);
  printf("finalshft: %x\n",ptr->finalshft);
  printf("chnmap %x\n",ptr->chanmap); 
  printf("addto: %x\n",ptr->addto); 
  printf("resampclkdvd: %x\n",ptr->resampclkdvd);
  printf("ratiomap: %x\n",ptr->ratiomap);
  printf("ratio0: %x\n",ptr->ratio0);
  printf("ratio1: %x\n",ptr->ratio1);
  printf("ratio2: %x\n",ptr->ratio2);
  printf("ratio3: %x\n",ptr->ratio3);
  printf("cntbyte0: %x\n",ptr->cntbyte0);
  printf("cntbyte1: %x\n",ptr->cntbyte1);
  printf("tricntrl: %x\n",ptr->tricntrl);
  printf("outformat: %x\n",ptr->outformat);
  printf("outmode: %x\n",ptr->outmode);
  printf("outframecntrl: %x\n",ptr->outframecntrl);
  printf("outwdsize: %x\n",ptr->outwdsize);
  printf("outclkcntrl: %x\n",ptr->outclkcntrl);
  printf("sermuxcntrl: %x\n",ptr->sermuxcntrl);
  printf("outtaga: %x\n",ptr->outtaga);
  printf("outtagb: %x\n",ptr->outtagb);
  printf("outtagc: %x\n",ptr->outtagc);
  printf("outtagd: %x\n",ptr->outtagd);
}

