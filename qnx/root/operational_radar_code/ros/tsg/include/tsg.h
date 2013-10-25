#ifndef _TSG_H
#define _TSG_H

#define CLOCK_PERIOD    10              /* clock period in microseconds */

#define TSG_OK 0
#define TSG_INV_RSEP 1
#define TSG_NO_SMSEP 2
#define TSG_INV_MPPUL_SMSEP 3
#define TSG_INV_PAT 4
#define TSG_INV_MPINC_SMSEP 5
#define TSG_INV_LAGFR_SMSEP 6
#define TSG_INV_DUTY_CYCLE 7
#define TSG_INV_ODD_SMSEP 8
#define TSG_INV_TXPL_BAUD 9
#define TSG_INV_MEMORY 10
#define TSG_INV_PHASE_DELAY 11


struct TSGbuf {
  int index;
  int len;
  int step;  //packed timesequence stepsize in microseconds
  unsigned char *code;
  unsigned char *rep;
};

void TSGFree(struct TSGbuf *ptr);
#endif
