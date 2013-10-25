/* maketsg.c
   ==========
   Author: R.J.Barnes
*/

/*
 Copyright 2004 The Johns Hopkins University/Applied Physics Laboratory.
 All rights reserved.
 
 This material may be used, modified, or reproduced by or for the U.S.
 Government pursuant to the license rights granted under the clauses at DFARS
 252.227-7013/7014.
 
 For any other permissions, please contact the Space Department
 Program Office at JHU/APL.
 
 This Distribution and Disclaimer Statement must be included in all copies of
 "Radar Operating System" (hereinafter "the Program").
 
 The Program was developed at The Johns Hopkins University/Applied Physics
 Laboratory (JHU/APL) which is the author thereof under the "work made for
 hire" provisions of the copyright law.  
 
 JHU/APL assumes no obligation to provide support of any kind with regard to
 the Program.  This includes no obligation to provide assistance in using the
 Program or to provide updated versions of the Program.
 
 THE PROGRAM AND ITS DOCUMENTATION ARE PROVIDED AS IS AND WITHOUT ANY EXPRESS
 OR IMPLIED WARRANTIES WHATSOEVER.  ALL WARRANTIES INCLUDING, BUT NOT LIMITED
 TO, PERFORMANCE, MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE
 HEREBY DISCLAIMED.  YOU ASSUME THE ENTIRE RISK AND LIABILITY OF USING THE
 PROGRAM TO INCLUDE USE IN COMPLIANCE WITH ANY THIRD PARTY RIGHTS.  YOU ARE
 ADVISED TO TEST THE PROGRAM THOROUGHLY BEFORE RELYING ON IT.  IN NO EVENT
 SHALL JHU/APL BE LIABLE FOR ANY DAMAGES WHATSOEVER, INCLUDING, WITHOUT
 LIMITATION, ANY LOST PROFITS, LOST SAVINGS OR OTHER INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, ARISING OUT OF THE USE OR INABILITY TO USE THE
 PROGRAM."
 
 
 
 
 
 
*/

#include <stdio.h>
#include <stdlib.h>

#include "tsg.h"

/*
 $Log: maketsg.c,v $
 Revision 1.4  2009/10/05 18:54:04  jspaleta
 jspaleta: delay for dds

 Revision 1.3  2009/08/14 23:29:20  jspaleta
 jspaleta: well now

 Revision 1.2  2009/07/17 20:33:30  jspaleta
 jspaleta: Ready to test timing card

 Revision 1.1.1.1  2009/07/13 22:20:49  jspaleta
 initial import into CVS

 Revision 1.10  2007/05/24 13:55:21  code
 Fixed implementation of gort.

 Revision 1.9  2007/05/24 13:38:05  code
 Added a fix to deal with gating versus triggering using the scope sync.
 The GC214 is gated, the GC214TS is triggered.

 Revision 1.8  2007/05/23 21:25:30  code
 Corrected the scope sync.

 Revision 1.7  2007/05/18 18:38:17  code
 Added scope sync.

 Revision 1.6  2006/11/07 22:35:46  code
 Fixed code that delays the start of the pulse sequence.

 Revision 1.5  2006/01/18 18:22:28  barnes
 Modification to keep the attenuators on until any impulse from the T/R
 switch has passed - 30 microseconds added to t4.

 Revision 1.4  2005/07/27 18:53:35  barnes
 Fixed code for changing delay after TX pulse.

 Revision 1.3  2004/06/16 22:53:48  barnes
 Added a new field to the parameter block so that a different delay between
 the receiver off and TX pulse can be set - This fix is for syowa.

 Revision 1.2  2004/05/08 17:13:26  barnes
 Code audit.

 Revision 1.1  2004/03/13 19:33:50  barnes
 Initial revision

*/


#define S_bit 0x01
#define A_bit 0x08
#define R_bit 0x02
#define X_bit 0x04
#define P_bit 0x10

#define Null_code 0x00
#define S_code S_bit
#define A_code A_bit
#define AS_code (A_code | S_bit)
#define AR_code (A_bit | R_bit)
#define ARS_code (AR_code | S_bit)
#define ARX_code (AR_code | X_bit)
#define ARXS_code (ARX_code | S_bit)

#define TEN_MICROSEC (10/CLOCK_PERIOD)
#define END_DELAY (10000/CLOCK_PERIOD)	/* time from last pulse to end of tm seq */
#define R_to_X_min 60/CLOCK_PERIOD	/* 60 microsec minimum spacing between AR and ARX */
#define A_to_R_min 10/CLOCK_PERIOD/* 10 microsec minimum spacing between A and AR */

#define MIN2(x,y)  (((x) <(y)) ? (x) : (y))

#define MIN3(a,b,c)  (((a) < MIN2( (b), (c))) ? (a) : (MIN2( (b), (c))))

/*	The method of creating a timing sequence is to set up a "state" machine
	The following table defines the allowed states. */

enum TSGstates {
	Null,	/* all bits 0 */
	S_state,	/* sample only */
	A1,		/* turn on attenuators before pulse */
	AS1,	/* turn on attenuators + take sample */
	AR1,	/* turn off receiver before pulse */
	ARS1,	/* turn off receiver + take sample */
	ARX,	/* transmit */
	ARXS,	/* transmit and sample */
	AR2,	/* turn off transmit */
	ARS2,	/* turn off transmit + take sample */
	A2,		/* turn on receiver */
	AS2,	/* turn on receiver + take sample */
	END_state	/* exit */
};

/*      For each state, we define the following quantities: 1) the control code
	that is sent to the digital I/O for the state, 2) the maximum duration
	the state can last, 3) the 3 possible transitions from the present
	state to the next.  The 3 possibilities correspond to the conditions:
	time_to_next_sample_state_change < time_to_next_pulse_state_change
	Note: the duration of a state may be less than the maximum, since a
	particular pulse state may be interrupted by a sample.  */

struct TSGsdef {
	 int rfvect;		/* code bits asserted when in this state */
	 int duration;	        /* maximum duration of this state */
	 int next_state[3];	/* possible transitions */
} states[13] = 
        /* Null */   {{Null_code, 0, {S_state,  AS1,     A1}},
	/* S_st */   {S_code,    0,  {Null,     A1,      AS1}},
	/* A1   */   {A_code,	 0,  {AS1,      ARS1,    AR1}},
	/* AS1  */   {AS_code,	 0,  {A1,       AR1,     ARS1}},
	/* AR1  */   {AR_code,	 0,  {ARS1,     ARXS,    ARX}},
	/* ARS1 */   {ARS_code,	 0,  {AR1,      ARX,     ARXS}},
	/* ARX  */   {ARX_code,	 0,  {ARXS,     ARS2,    AR2}},
	/* ARXS */   {ARXS_code, 0,  {ARX,      AR2,     ARS2}},
	/* AR2	*/   {AR_code,	 0,  {ARS2,     AS2,     A2}},
	/* ARS2 */   {ARS_code,	 0,  {AR2,      A2,      AS2}},
	/* A2   */   {A_code,	 0,  {AS2,      S_state, Null}},
	/* AS2  */   {AS_code,	 0,  {A2,       Null,    S_state}},
        /* end  */   {Null_code, 1,  {Null,     Null,    Null}}};


struct TSGcnt {
  int last;
  int n_smp;
};

void TSGWrBuf(struct TSGbuf *tsg,
	       int code,int delay,struct TSGcnt *cnt) {
  while (delay > 255) {	
	(tsg->code)[tsg->len] = (char) code;
	(tsg->rep)[tsg->len] = (char) 255;
	tsg->len++;
	delay = delay - 255;
  }
  (tsg->code)[tsg->len] = (char) code;
  (tsg->rep)[tsg->len] = (char) delay;
  tsg->len++;  
  if ((code & S_bit) && (cnt->last==0)) cnt->n_smp++;
  cnt->last=code & S_bit;
  return;
}

void TSGFree(struct TSGbuf *ptr) {
  struct TSGprm *tsgprm;
  if (ptr->code !=NULL) free(ptr->code);
  if (ptr->rep !=NULL) free(ptr->rep);
}

