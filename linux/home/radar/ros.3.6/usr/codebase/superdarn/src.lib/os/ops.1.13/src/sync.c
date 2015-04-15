/* sync.c
   ====== 
   Author: R.J.Barnes
*/

/*
 LICENSE AND DISCLAIMER
 
 Copyright (c) 2012 The Johns Hopkins University/Applied Physics Laboratory
 
 This file is part of the Radar Software Toolkit (RST).
 
 RST is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version.
 
 RST is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with RST.  If not, see <http://www.gnu.org/licenses/>.
 
 
 
*/

#include <math.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>
#include "rtime.h"
#include "rtypes.h"
#include "dmap.h"
#include "limit.h"
#include "rprm.h"
#include "rawdata.h"
#include "fitblk.h"
#include "fitdata.h"
#include "radar.h"
#include "global.h"


#define USEC 1000000.0

int OpsDayNight() {
  if (day < night) {
    if ((day <= hr) && (hr < night)) return 1;
    else return 0;
  } else {
    if ((night <= hr) && (hr < day)) return 0;
  }
  return 1;
}
 
int OpsFindSkip(int bsc,int bus) {
  
  int64_t tv;
  int64_t bv;
  int64_t remain;
  int64_t iv;
  int64_t skip;
  int nbm;
  nbm=fabs(ebm-sbm);
  TimeReadClock(&yr,&mo,&dy,&hr,&mt,&sc,&us);
  iv=intsc*1000000+intus;
  bv=bsc*1000000+bus;
  /* tv : usecs since top of the hour  */
  tv=mt*60+sc;
  tv*=1E6;
  tv+=us; 
  /* tv : plus half-ish an integration period   */
  tv+=iv/2-100000; 
  /* skip: first modulo by scan usecs */
  remain=tv % bv; 
  /* skip: now find number of integration periods that fit */
  skip=remain/iv; 
/* JDS: allow out of range skip values so you can jump right to wait for boundary
 *      when you have end of scan actions like sounding or special camp beams
 */       
/*
  if (skip>nbm) skip=0;
  if (skip<0) skip=0;
*/
  return (int)skip;
}

 
