struct GC4016Global {
   char nchanout;       /* 0x27 */
   char nmult;          /* 0x46 */
   char filtslct;       /* 0x00 */
   char finalshft;      /* 0x16 */
   char chanmap;        /* 0xe4 */
   char addto;          /* 0x00 */
   char resampclkdvd;   /* 0x00 */
   char ratiomap;       /* 0xe4 */
   int ratio0;          /* 0x04000000 */
   int ratio1;          /* 0x04000000 */
   int ratio2;          /* 0x04000000 */   
   int ratio3;          /* 0x04000000 */  
   char cntbyte0;       /* 0x00 */
   char cntbyte1;       /* 0x00 */
   char tricntrl;       /* 0xff */
   char outformat;      /* 0x49 */
   char outmode;        /* 0x6c */
   char outframecntrl;  /* 0xc7 */
   char outwdsize;      /* 0xef */
   char outclkcntrl;    /* 0x01 */ 
   char sermuxcntrl;    /* 0xe4 */
   char outtaga;        /* 0x10 */
   char outtagb;        /* 0x32 */
   char outtagc;        /* 0x54 */
   char outtagd;        /* 0x76 */
};

struct GC4016Channel { 
  int chreset;          /* 0x0c*/
  int phase;            /* 0x0000 */
  int freq;             /* 0x2f430313 */
  int decratio;         /* 0x703f */
  int cicscale;         /* 0x64 */
  int splitiq;          /* 0x00 */
  int cfir;             /* 0x00 */
  int pfir;             /* 0x00 */
  int input;            /* 0x00 */
  int peakcntrl;        /* 0x00 */
  int peakcount;        /* 0x00 */
  int finegain;         /* 0x00 */
  double rate;         
  int match;         
  int samples;         
            
};


void setupGC4016(long baseGC214,struct GC4016Global *ptr,short *resampcoeffs);
void releaseGC4016(long baseGC124);
void setupGC4016channel(long baseGC214,int channel,
                        struct GC4016Channel *ptr,int *cfircoeff,
                        int *pfircoef);
