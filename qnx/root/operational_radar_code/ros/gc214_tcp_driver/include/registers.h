/* registers.h
   ===========
   Authors: Todd Peterson, R.J.Barnes
*/


/*
 $Log: registers.h,v $
 Revision 1.1  2009/10/19 22:55:59  jspaleta
 jspaleta: old school gc214 driver

 Revision 1.1  2005/07/25 15:11:31  barnes
 Initial revision

*/


#ifndef registers_h
#define registers_h

#define DEVICE_ID 0x9080
#define VENDOR_ID 0x10b5

#define offset_GC214						0x80000
#define offset_GC4016						0x80000
#define offset_PCI9080_CFG					0x00000

#define reg_GC214_CFG						0x00
#define reg_GC214_STAT						0x04
#define reg_GC214_FSTAT						0x08
#define reg_GC214_ICR						0x0c
#define reg_GC214_ISR						0x10
#define reg_GC214_CLKDLY					0x14
#define reg_GC214_TCD						0x18
#define reg_GC214_CHCTRL                                        0x20
#define reg_GC214_CH1ID                                         0x30
#define reg_GC214_CH2ID                                         0x34
#define reg_GC214_CH3ID                                         0x38
#define reg_GC214_CH4ID                                         0x3c
#define reg_GC214_CH1FS                                         0x40
#define reg_GC214_CH2FS                                         0x44
#define reg_GC214_CH3FS                                         0x48
#define reg_GC214_CH4FS                                         0x4c
#define reg_GC214_SMSK                                          0x50
#define reg_GC214_RTSSOEN                                       0x60
#define reg_GC214_RTSS1T                                        0x64
#define reg_GC214_RTSS2T                                        0x68
#define reg_GC214_RTS                                           0x6c
#define reg_GC214_RTSRD                                         0x70
#define reg_GC214_SCMD                                          0x74
#define reg_GC214_RSTCMD                                        0x78
#define reg_GC214_FRST                                          0x7c


#define reg_GC4016_RESAMPCOEFFS					0x300
#define reg_GC4016_NCHANOUT					0x500
#define reg_GC4016_NMULT					0x501
#define reg_GC4016_FILTSLCT					0x502
#define reg_GC4016_FINALSHFT					0x503
#define reg_GC4016_CHANMAP					0x504
#define reg_GC4016_ADDTO					0x505
#define reg_GC4016_RESAMPCLKDVD					0x506
#define reg_GC4016_RATIOMAP					0x507
#define reg_GC4016_RATIO0					0x510
#define reg_GC4016_RATIO1					0x514
#define reg_GC4016_RATIO2					0x518
#define reg_GC4016_RATIO3					0x51c
#define reg_GC4016_CHANOUT					0x520
#define reg_GC4016_GLOBALRST				0x540
#define reg_GC4016_STAT						0x541
#define reg_GC4016_PAGE						0x542
#define reg_GC4016_CHECKSUM					0x543
#define reg_GC4016_GENSYNC					0x544
#define reg_GC4016_CNTSYNC					0x545
#define reg_GC4016_CNTBYTE0					0x546
#define reg_GC4016_CNTBYTE1					0x547
#define reg_GC4016_TRICNTRL					0x550
#define reg_GC4016_OUTFORMAT					0x551
#define reg_GC4016_OUTMODE					0x552
#define reg_GC4016_OUTFRAMECNTRL				0x553
#define reg_GC4016_OUTWDSIZE					0x554
#define reg_GC4016_OUTCLKCNTRL					0x555
#define reg_GC4016_SERMUXCNTRL					0x556
#define reg_GC4016_OUTTAGA					0x557
#define reg_GC4016_OUTTAGB					0x558
#define reg_GC4016_OUTTAGC					0x559
#define reg_GC4016_OUTTAGD					0x55a
#define reg_GC4016_MASKREVIS					0x55b
#define reg_GC4016_MISC						0x55c

#define offset_GC4016_A 0x000
#define offset_GC4016_B 0x080
#define offset_GC4016_C 0x100
#define offset_GC4016_D 0x180
//#define offset_GC4016_A 0x100
//#define offset_GC4016_B 0x180
//#define offset_GC4016_C 0x200
//#define offset_GC4016_D 0x280

#define reg_GC4016_CFIRCOEFFS				0x100
#define reg_GC4016_PFIRCOEFFS				0x120
#define reg_GC4016_PHASE				0x160
#define reg_GC4016_FREQ					0x162
#define reg_GC4016_CHRESET				0x170
#define reg_GC4016_FREQSYNC				0x171
#define reg_GC4016_NCOSYNC				0x172
#define reg_GC4016_ZEROPAD				0x173
#define reg_GC4016_DECSYNC				0x174
#define reg_GC4016_DECRATIO				0x175
#define reg_GC4016_CICSCALE				0x177
#define reg_GC4016_SPLITIQ				0x178
#define reg_GC4016_CFIR					0x179
#define reg_GC4016_PFIR					0x17a
#define reg_GC4016_INPUT				0x17b
#define reg_GC4016_PEAKCNTRL				0x17c
#define reg_GC4016_PEAKCOUNT				0x17d
#define reg_GC4016_FINEGAIN				0x17e

/* 
  define PLX PCI9080
  PCI CONFIGURATION REGISTERS
*/

#define reg_PCI9080_PCIIDR					0x00
#define reg_PCI9080_PCICR					0x04
#define reg_PCI9080_PCISR					0x06
#define reg_PCI9080_PCIREV					0x08
#define reg_PCI9080_PCICCR					0x09
#define reg_PCI9080_PCICLSR					0x0c
#define reg_PCI9080_PCILTR					0x0d
#define reg_PCI9080_PCIHTR					0x0e
#define reg_PCI9080_PCIBISTR					0x0f
#define reg_PCI9080_PCIBAR0					0x10
#define reg_PCI9080_PCIBAR1					0x14
#define reg_PCI9080_PCIBAR2					0x18
#define reg_PCI9080_PCIBAR3					0x1c
#define reg_PCI9080_PCIBAR4					0x20
#define reg_PCI9080_PCIBAR5					0x24
#define reg_PCI9080_PCICIS					0x28
#define reg_PCI9080_PCISVID					0x2c
#define reg_PCI9080_PCISID					0x2e
#define reg_PCI9080_PCIERBAR					0x30
#define reg_PCI9080_PCIILR					0x3c
#define reg_PCI9080_PCIIPR					0x3d
#define reg_PCI9080_PCIMGR					0x3e
#define reg_PCI9080_PCIMLR					0x3f

/* 
  define PLX PCI9080
  LOCAL CONFIGURATION REGISTERS
*/

#define reg_PCI9080_LAS0RR					0x00
#define reg_PCI9080_LAS0BA					0x04
#define reg_PCI9080_MARBR					0x08
#define reg_PCI9080_BIGEND					0x0c
#define reg_PCI9080_EROMRR					0x10
#define reg_PCI9080_EROMBA					0x14
#define reg_PCI9080_LBRD0					0x18
#define reg_PCI9080_DMRR					0x1c
#define reg_PCI9080_DMLBAM					0x20
#define reg_PCI9080_DMLBAI					0x24
#define reg_PCI9080_DMPBAM					0x28
#define reg_PCI9080_DMCFGA					0x2c
#define reg_PCI9080_LAS1RR					0xf0
#define reg_PCI9080_LAS1BA					0xf4
#define reg_PCI9080_LBRD1					0xf8

/* 
  define PXL PCI9080
  RUNTIME REGISTERS
*/

#define reg_PCI9080_MBOX0					0x40
#define reg_PCI9080_MBOX1					0x44
#define reg_PCI9080_MBOX2					0x48
#define reg_PCI9080_MBOX3					0x4c
#define reg_PCI9080_MBOX4					0x50
#define reg_PCI9080_MBOX5					0x54
#define reg_PCI9080_MBOX6					0x58
#define reg_PCI9080_MBOX7					0x5c
#define reg_PCI9080_P2LDBELL					0x60
#define reg_PCI9080_L2PDBELL					0x64
#define reg_PCI9080_INTCSR					0x68
#define reg_PCI9080_CNTRL					0x6c
#define reg_PCI9080_PCIHIDR					0x70
#define reg_PCI9080_PCIHREV					0x74

/* 
   define PXL PCI9080
   RUNTIME REGISTERS
*/

#define reg_PCI9080_DMAMODE0				0x80
#define reg_PCI9080_DMAPADR0				0x84
#define reg_PCI9080_DMALADR0				0x88
#define reg_PCI9080_DMASIZ0				0x8c
#define reg_PCI9080_DMADPR0				0x90
#define reg_PCI9080_DMAMODE1				0x94
#define reg_PCI9080_DMAPADR1				0x98
#define reg_PCI9080_DMALADR1				0x9c
#define reg_PCI9080_DMASIZ1				0xa0
#define reg_PCI9080_DMADPR1				0xa4
#define reg_PCI9080_DMACSR0				0xa8
#define reg_PCI9080_DMACSR1				0xa9
#define reg_PCI9080_DMAARB				0xac
#define reg_PCI9080_DMATHR				0xb0

/* define masks */

#define mask_GC214_CFG						0x000001f3
#define mask_GC214_STAT						0x000000f3
#define mask_GC214_FSTAT					0x1f1f1f1f
#define mask_GC214_ICR						0x00000fff
#define mask_GC214_ISR						0x00000fff
#define mask_GC214_CLKDLY					0x000003ff
#define mask_GC214_TCD                                          0xffffffff

#define mask_GC214_CHCTRL                                       0x0000000f
#define mask_GC214_CH1ID                                        0xffffffff
#define mask_GC214_CH2ID                                        0xffffffff
#define mask_GC214_CH3ID                                        0xffffffff
#define mask_GC214_CH4ID                                        0xffffffff
#define mask_GC214_CH1FS                                        0xffffffff
#define mask_GC214_CH2FS                                        0xffffffff
#define mask_GC214_CH3FS                                        0xffffffff
#define mask_GC214_CH4FS                                        0xffffffff
#define mask_GC214_SMSK                                         0xffffffff
#define mask_GC214_RTSSOEN                                      0x00000003
#define mask_GC214_RTSS1T                                       0xffffffff
#define mask_GC214_RTSS2T                                       0xffffffff
#define mask_GC214_RTS                                          0xffffffff
#define mask_GC214_RTSRD                                        0xffffffff
#define mask_GC214_SCMD                                         0x00000033
#define mask_GC214_RSTCMD                                       0x00000003
#define mask_GC214_FRST                                         0x0000000f

#endif
