/* site.fhe.h
   ==========

   Copied from site.ade.h.  -KTS 02Oct2013

*/


#ifndef _SITEFHE_H
#define _SITEFHE_H

int SiteFheStart(char *host);
int SiteFheKeepAlive();
int SiteFheSetupRadar();
int SiteFheStartScan();
int SiteFheStartIntt(int intsc,int intus);
int SiteFheFCLR(int stfreq,int edfreq);
int SiteFheTimeSeq(int *ptab);
int SiteFheIntegrate(int (*lags)[2]);
int SiteFheEndScan(int bsc,int bus);
void SiteFheExit(int signum);

#endif

