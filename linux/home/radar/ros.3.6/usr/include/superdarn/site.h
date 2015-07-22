/* site.h
   ======
   Author: R.J.Barnes
*/


#ifndef _SITE_H
#define _SITE_H

struct SiteLibrary {
  int (*start)(char *,char *);
  int (*setupradar)();
  int (*startscan)();
  int (*wait)();
  int (*startintt)(int,int);
  int (*fclr)(int,int);
  int (*tmseq)(int *);
  int (*integrate)(int (*lags)[2]);
  int (*endscan)(int,int);
  void (*exit)(int);
};

int SiteStart(char *host,char *ststr);
int SiteSetupRadar();
int SiteStartScan();
int SiteWait(int secs,int usecs);
int SiteStartIntt(int intsc,int intus);
int SiteFCLR(int stfreq,int edfreq);
int SiteTimeSeq(int *ptab);
int SiteIntegrate(int (*lags)[2]);
int SiteEndScan(int bndsc,int bndus);
void SiteExit(int signo);


#endif

