/* site.mcm.h
   ==========
*/


#ifndef _SITEMCM_H
#define _SITEMCM_H

int SiteMcmStart(char *host);
int SiteMcmKeepAlive();
int SiteMcmSetupRadar();
int SiteMcmStartScan();
int SiteMcmStartIntt(int intsc,int intus);
int SiteMcmFCLR(int stfreq,int edfreq);
int SiteMcmTimeSeq(int *ptab);
int SiteMcmIntegrate(int (*lags)[2]);
int SiteMcmEndScan(int bsc,int bus);
void SiteMcmExit(int signum);

#endif

