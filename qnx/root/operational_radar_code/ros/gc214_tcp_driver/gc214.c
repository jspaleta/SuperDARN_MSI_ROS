#include "registers.h"

	
void setupGC214(long base_GC214,int bcnt,int metcnt) {
  int temp;
  write32(base_GC214+offset_GC214+reg_GC214_RSTCMD,0x00000003);
  write32(base_GC214+offset_GC214+reg_GC214_CLKDLY,0x000001ff); 
  write32(base_GC214+offset_GC214+reg_GC214_CH1ID,0x00010000);  
  write32(base_GC214+offset_GC214+reg_GC214_CH2ID,0x00020000);  
  write32(base_GC214+offset_GC214+reg_GC214_CH3ID,0x00030000);  
  write32(base_GC214+offset_GC214+reg_GC214_CH4ID,0x00040000);  
  write32(base_GC214+offset_GC214+reg_GC214_CH1FS,bcnt);  
  write32(base_GC214+offset_GC214+reg_GC214_CH2FS,bcnt);  
  write32(base_GC214+offset_GC214+reg_GC214_CH3FS,metcnt);  
  write32(base_GC214+offset_GC214+reg_GC214_CH4FS,metcnt);  
  write32(base_GC214+offset_GC214+reg_GC214_CFG,0x00000057);
  write32(base_GC214+offset_GC214+reg_GC214_SMSK,0x00000001);
  write32(base_GC214+offset_GC214+reg_GC214_SCMD,0x00000001);
}


void enableGC214(long base_GC214) {
  write32(base_GC214+offset_GC214+reg_GC214_SCMD,0x00000000);
  read32(base_GC214+offset_GC214+reg_GC214_STAT);
  read32(base_GC214+offset_GC214+reg_GC214_ISR);
  write32(base_GC214+offset_GC214+reg_GC214_ICR,0x00000000);
  write32(base_GC214+offset_GC214+reg_GC214_CHCTRL,0x00000000);
  write32(base_GC214+offset_GC214+reg_GC214_SMSK,0x000000fc);
  write32(base_GC214+offset_GC214+reg_GC214_SCMD,0x00000010);
  write32(base_GC214+offset_GC214+reg_GC214_RSTCMD,0x00000002);
  write32(base_GC214+offset_GC214+reg_GC214_CHCTRL,0x0000000f);
  write32(base_GC214+offset_GC214+reg_GC214_SMSK,0x00000fff);
}

void triggerGC214(long base_GC214) {
  write32(base_GC214+offset_GC214+reg_GC214_SCMD,0x0000010);
}

int pollGC214_CH1ID(long base_GC214) {
  int r;
  int temp;
  temp=read32(base_GC214+offset_GC214+reg_GC214_CH1ID);
  r=(int)(temp&0x0000ffff);
  return r;
}

int pollGC214_CH3ID(long base_GC214) {
  int r;
  int temp;
  temp=read32(base_GC214+offset_GC214+reg_GC214_CH3ID);
  r=(int)(temp&0x0000ffff);
  return r;
}

int  pollGC214FIFO(long base_GC214) {
  int r;
  r=read32(base_GC214+offset_GC214+reg_GC214_STAT);
  return r;
}

void stopGC214(long base_GC214){
  int tempsmsk,tempscmd;
  write32(base_GC214+offset_GC214+reg_GC214_CHCTRL,0x00000000);  
  write32(base_GC214+offset_GC214+reg_GC214_SMSK,0x00000004);  
  write32(base_GC214+offset_GC214+reg_GC214_SCMD,0x00000010);

}
