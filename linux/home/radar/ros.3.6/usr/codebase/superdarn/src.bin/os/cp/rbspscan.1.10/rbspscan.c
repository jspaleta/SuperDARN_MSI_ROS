/* rbspscan.c
 ============
 Author: Kevin Sterne

 This code uses the 'Option 2' beam progression in which the west, meridional, and
 east beams are skipped in the regular field of view scan.  As a bit of a trick,
 the beam progression goes for a forward radar:

 westbm, fovbm, meribm, eastbm, fovbm, fovbm, westbm, fovbm, meribm, eastbm, fovbm, fovbm, westbm, ...

 It was noticed that this kind of pattern still allows for 5 repetitions of the
 mini-scan beams for the traditional 16 beams radars.  This code also does not
 synchronize the start of the beam sounding to an integer time interval.

 This code is largely based off of the themisscan.c RCP.  The credits for this go to:
 Author: J.Spaleta
 With Modifications by M. McClorey

 */

/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.1-beta-18-gf704e97.txt for more information.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>
#include "rtypes.h"
#include "option.h"
#include "rtime.h"
#include "dmap.h"
#include "limit.h"
#include "radar.h"
#include "rprm.h"
#include "iq.h"
#include "rawdata.h"
#include "fitblk.h"
#include "fitdata.h"
#include "fitacf.h"
#include "errlog.h"
#include "freq.h"
#include "tcpipmsg.h"
#include "rmsg.h"
#include "rmsgsnd.h"
#include "radarshell.h"
#include "build.h"
#include "global.h"
#include "reopen.h"
#include "setup.h"
#include "sync.h"
#include "site.h"
#include "sitebuild.h"
#include "siteglobal.h"
#include "rosmsg.h"
#include "tsg.h"


char *ststr=NULL;
char *dfststr="tst";
void *tmpbuf;
size_t tmpsze;
char progid[80]={"rbspscan"};
char progname[256];
int arg=0;
struct OptionData opt;

char *roshost=NULL;
char *droshost={"127.0.0.1"};

int baseport=44100;

struct TCPIPMsgHost errlog={"127.0.0.1",44100,-1};
struct TCPIPMsgHost shell={"127.0.0.1",44101,-1};

int tnum=4;
struct TCPIPMsgHost task[4]={
	{"127.0.0.1",1,-1}, /*  iqwrite */
	{"127.0.0.1",2,-1}, /* rawacfwrite */
	{"127.0.0.1",3,-1}, /* fitacfwrite */
	{"127.0.0.1",4,-1}  /* rtserver */
};

int main(int argc,char *argv[]) {

	/*
   * commentary here: SGS
   * It seems that the mode should be decoupled from the pulse sequence.
   * The pulse table and lag table should be externally defined with some
   * way of determining the time it takes for a given sequence.
   */
	int ptab[8] = {0,14,22,24,27,31,42,43};

	int lags[LAG_SIZE][2] = {
		{ 0, 0},		/*  0 */
		{42,43},		/*  1 */
		{22,24},		/*  2 */
		{24,27},		/*  3 */
		{27,31},		/*  4 */
		{22,27},		/*  5 */

		{24,31},		/*  7 */
		{14,22},		/*  8 */
		{22,31},		/*  9 */
		{14,24},		/* 10 */
		{31,42},		/* 11 */
		{31,43},		/* 12 */
		{14,27},		/* 13 */
		{ 0,14},		/* 14 */
		{27,42},		/* 15 */
		{27,43},		/* 16 */
		{14,31},		/* 17 */
		{24,42},		/* 18 */
		{24,43},		/* 19 */
		{22,42},		/* 20 */
		{22,43},		/* 21 */
		{ 0,22},		/* 22 */

		{ 0,24},		/* 24 */

		{43,43}};		/* alternate lag-0  */

	char logtxt[1024];
	char tempLog[40];
/*	char diagtxt[1024]="";   This variable is unused?  */

	int exitpoll=0;
	int scannowait=0;
	int scnsc=120;
	int scnus=0;
	int skip;
	int cnt=0;
	int i,n;
	unsigned char discretion=0;
	unsigned char trig=0;					/* used to delcare triggered run of cp */
	int status=0;
	int fixfrq=0;
	int isecond, ithird;     				/* used in beam progression */
	int tempF, tempB;

	/* new variables for dynamically creating beam sequences */
	int *intgt;
	int *fbms;						/* forward  scanning beams */
	int *bbms;						/* backward scanning beams */
	int meribm=10;						/* meridional beam */
	int westbm=9;						/* west beam */
	int eastbm=11;						/* east beam */
	int nintgs;						/* number of integration periods per scan */

  /* standard radar defaults */
	cp=200;			/* Semi-official cpid for rbspscan as of 26Oct2012 -KTS */
	intsc=3;
	intus=200000;
	mppul=8;
	mplgs=23;
	mpinc=1500;
	dmpinc=1500;
	nrang=100;
	rsep=45;
	txpl=300;             /* note: recomputed below */

	/* ========= PROCESS COMMAND LINE ARGUMENTS ============= */

	OptionAdd(&opt,"di",    'x',&discretion);
	OptionAdd(&opt,"frang", 'i',&frang);
	OptionAdd(&opt,"rsep",  'i',&rsep);
	OptionAdd(&opt,"dt",    'i',&day);
	OptionAdd(&opt,"nt",    'i',&night);
	OptionAdd(&opt,"df",    'i',&dfrq);
	OptionAdd(&opt,"nf",    'i',&nfrq);
	OptionAdd(&opt,"xcf",   'i',&xcnt);
	OptionAdd(&opt,"nrang",   'i',&nrang);
	OptionAdd(&opt,"sb",    'i',&sbm);
	OptionAdd(&opt,"eb",    'i',&ebm);
	OptionAdd(&opt,"ep",    'i',&errlog.port);
	OptionAdd(&opt,"sp",    'i',&shell.port);
	OptionAdd(&opt,"bp",    'i',&baseport);
	OptionAdd(&opt,"stid",  't',&ststr);
	OptionAdd(&opt,"fixfrq",'i',&fixfrq);		/* fix the transmit frequency */
	OptionAdd(&opt,"meribm",'i',&meribm);		/* meridional beam */
	OptionAdd(&opt,"westbm",'i',&westbm);		/* west beam */
	OptionAdd(&opt,"eastbm",'i',&eastbm);		/* east beam */
	OptionAdd(&opt,"trig",	'x',&trig);		/* trigger flag */
	OptionAdd(&opt,"c",	'i',&cnum);		/* trigger flag */


	/* Process all of the command line options
	IMPORTANT: need to do this here because we need stid and ststr   */
	arg=OptionProcess(1,argc,argv,&opt,NULL);

	/* number of integration periods possible in scan time */
	/* basing this off of intsc and intus to reflect the integration
	   used by the radar -KTS 25Sept2012 */
	nintgs = floor((scnsc+scnus*1e-6)/(intsc+(intus*1e-6)));

	/* Makes sure there is an equal number of miniscan beams */
	/* Taking one off for buffer at the end of the scan */
	nintgs = nintgs - nintgs%6 - 1;

	/* arrays for integration start times and beam sequences */
	intgt = (int *)malloc(nintgs*sizeof(int));
	fbms  = (int *)malloc(nintgs*sizeof(int));
	bbms  = (int *)malloc(nintgs*sizeof(int));

	if (ststr==NULL) ststr=dfststr;
        if (roshost==NULL) roshost=getenv("ROSHOST");
        if (roshost==NULL) roshost=droshost;


	if ((errlog.sock=TCPIPMsgOpen(errlog.host,errlog.port))==-1) {
		fprintf(stderr,"Error connecting to error log.\n");
	}
	if ((shell.sock=TCPIPMsgOpen(shell.host,shell.port))==-1) { 
		fprintf(stderr,"Error connecting to shell.\n");
	}

	for (n=0;n<tnum;n++) task[n].port+=baseport;

	/* rst/usr/codebase/superdarn/src.lib/os/ops.1.10/src/setup.c */
	OpsStart(ststr);

	status=SiteBuild(ststr,NULL);

	if (status==-1) {
		fprintf(stderr,"Could not identify station.\n");
		exit(1);
	}

	/* IMPORTANT: sbm and ebm are reset by this function */
	SiteStart(roshost);

	/* Reprocess the command line to restore desired parameters */
	arg=OptionProcess(1,argc,argv,&opt,NULL);

	/* If backward is set for West radar, start and end beams need to be reversed for the
	 * beam assigning code that follows until "End of Dartmouth Mods".  Usual SuperDARN
	 * logic follows that for West radars sbm >= ebm and East radars sbm <= ebm.
	 * However, the code below for assigning the beam number arrays, fbms & bbms,
	 * does not follow usual SuperDARN logic and always assumes sbm <= ebm.  -KTS 2/20/2012 */
	if (backward == 1) {
		i = sbm;
		sbm = ebm;
		ebm = i;
	}

	/* Creating beam progression arrays fbms and bbms. ASSUMES that forward
         * scan radars will use west beam first, and backward scan radars will
	 * use east beam first.  -KTS 09Oct2012 */
/*	fbms[0] = westbm;
	bbms[0] = eastbm;	Think this is unnecessary with loop below set right -KTS 31Oct2012 */

	tempB = ebm;
	tempF = sbm;
	for(i = 0; i<nintgs; i++){
		isecond = (i-2)%6;
		ithird  = (i-3)%6;
		/* Every 0th, 6th, 12th, etc sounding is a mini-scan beam */
		if(i%6 == 0) {
			fbms[i] = westbm;
			bbms[i] = eastbm;
		/* Every 6th sounding starting with 2 is a mini-scan beam */
		} else if (isecond == 0) {
			fbms[i] = meribm;
			bbms[i] = meribm;
		/* Every 6th sounding starting with 3 is a mini-scan beam */
		} else if (ithird == 0) {
			fbms[i] = eastbm;
			bbms[i] = westbm;
		} else if (tempF<=ebm) {

			/* Here is where mini-scan beams are skipped in the FoV scan.  The logic
			   works out that if a start or end beam is in the mini-scan, then it must
			   be treated differently.  -KTS 10Oct2012 */

		/* If tempF is any of the mini-scan beams, then increment it again */
			if (tempF == westbm)
				tempF++;
			if (tempF == meribm)
				tempF++;
			if (tempF == eastbm)
				tempF++;
			fbms[i] = tempF;
			tempF++;

		/* If tempB is any of the mini-scan beams, then decrement it again */
			if (tempB == eastbm)
				tempB--;
			if (tempB == meribm)
				tempB--;
			if (tempB == westbm)
				tempB--;
			bbms[i] = tempB;
			tempB--;

		} else {

		/* In case the calculation earlier gets messed up, set the beam number
		   to something useful.  Otherwise, leaving the array value unassigned
		   can be bad!!! -KTS 31Oct2012 */

			bbms[i] = 7;
			fbms[i] = 7;
		}
	}

	/* not sure if -nrang commandline option works */

	strncpy(combf,progid,80);

	/* rst/usr/codebase/superdarn/src.lib/os/ops.1.10/src */
	OpsSetupCommand(argc,argv);
	OpsSetupShell();

	RadarShellParse(&rstable,"sbm l ebm l dfrq l nfrq l dfrang l nfrang l"
					" dmpinc l nmpinc l frqrng l xcnt l", &sbm,&ebm, &dfrq,&nfrq,
					&dfrang,&nfrang, &dmpinc,&nmpinc, &frqrng,&xcnt);

	status=SiteSetupRadar();

	printf("Initial Setup Complete: Station ID: %s  %d\n",ststr,stid);

	fprintf(stderr,"Status:%d\n",status);

	if (status !=0) {
		ErrLog(errlog.sock,progname,"Error locating hardware.");
		exit (1);
	}

	if (trig) cp = 220;
	if (discretion) cp = -cp;

	txpl=(rsep*20)/3;		/* computing TX pulse length */

	sprintf(progname,"rbspscan");

	OpsLogStart(errlog.sock,progname,argc,argv);
	OpsSetupTask(tnum,task,errlog.sock,progname);

	for (n=0;n<tnum;n++) {
		RMsgSndReset(task[n].sock);
		RMsgSndOpen(task[n].sock,strlen( (char *) command),command);
	}

	printf("Preparing OpsFitACFStart Station ID: %s  %d\n",ststr,stid);

	OpsFitACFStart();

	printf("Preparing SiteTimeSeq Station ID: %s  %d\n",ststr,stid);

	tsgid=SiteTimeSeq(ptab);	/* get the timing sequence */

	/* OpsFindSkip assumes that beam arrays, fbms and bbms, equal the number of
	 * beams in the field of view.  Rbspscan's mini-scan beams make the size
	 * of the beam arrays larger than the field of view.  Since this is skipped
	 * the bulk of the OpsFindSkip code has been copied below.  -KTS 12Oct2012 */
	/* skip=OpsFindSkip(scnsc,scnus); */

	int tv;
	int bv;
	int iv;
	TimeReadClock(&yr,&mo,&dy,&hr,&mt,&sc,&us);
	iv= intsc*1000000 + intus;
	bv= scnsc*1000000 + scnus;
	tv=(mt* 60 + sc)* 1000000 + us + iv/2 - 100000;
	skip=floor((tv % bv)/iv)+2;
	if (skip> nintgs-1) skip=0;
	if (skip<0) skip=0;

	if (backward) {
		bmnum= bbms[skip];
	} else {
		bmnum= fbms[skip];
	}

	printf("Entering Scan loop Station ID: %s  %d\n",ststr,stid);

	do {

		printf("Entering Site Start Scan Station ID: %s  %d\n",ststr,stid);
		if (SiteStartScan() !=0) continue;

		if (OpsReOpen(2,0,0) !=0) {
			ErrLog(errlog.sock,progname,"Opening new files.");
			for (n=0;n<tnum;n++) {
				RMsgSndClose(task[n].sock);
				RMsgSndOpen(task[n].sock,strlen( (char *) command),command);
			}
		}

		scan=1;

		ErrLog(errlog.sock,progname,"Starting scan.");

		if (xcnt>0) {
			cnt++;
			if (cnt==xcnt) {
				xcf=1;
				cnt=0;
			} else xcf=0;
		} else xcf=0;

		do {
			TimeReadClock(&yr,&mo,&dy,&hr,&mt,&sc,&us);

			if (OpsDayNight()==1) {
				stfrq=dfrq;
				mpinc=dmpinc;
				frang=dfrang;
			} else {
				stfrq=nfrq;
				mpinc=nmpinc;
				frang=nfrang;
			}

			sprintf(logtxt,"Integrating beam:%d intt:%ds.%dus (%d:%d:%d:%d)",bmnum,
					intsc,intus,hr,mt,sc,us);
			ErrLog(errlog.sock,progname,logtxt);

			ErrLog(errlog.sock,progname,"Starting Integration.");

			printf("Entering Site Start Intt Station ID: %s  %d\n",ststr,stid);
			SiteStartIntt(intsc,intus);

			ErrLog(errlog.sock,progname,"Doing clear frequency search.");

			sprintf(logtxt, "FRQ: %d %d", stfrq, frqrng);
			ErrLog(errlog.sock,progname, logtxt);

			tfreq=SiteFCLR(stfrq,stfrq+frqrng);
			if ( (fixfrq > 8000) && (fixfrq < 25000) ) tfreq= fixfrq;

			sprintf(logtxt,"Transmitting on: %d (Noise=%g)",tfreq,noise);
			ErrLog(errlog.sock,progname,logtxt);

			nave=SiteIntegrate(lags);
			if (nave<0) {
				sprintf(logtxt,"Integration error:%d",nave);
				ErrLog(errlog.sock,progname,logtxt);
				continue;
			}
			sprintf(logtxt,"Number of sequences: %d",nave);
			ErrLog(errlog.sock,progname,logtxt);

			OpsBuildPrm(prm,ptab,lags);
			OpsBuildIQ(iq,&badtr);
			OpsBuildRaw(raw);

			FitACF(prm,raw,fblk,fit);

			msg.num=0;
			msg.tsize=0;

			tmpbuf=RadarParmFlatten(prm,&tmpsze);
			RMsgSndAdd(&msg,tmpsze,tmpbuf,PRM_TYPE,0);

			tmpbuf=IQFlatten(iq,prm->nave,&tmpsze);
			RMsgSndAdd(&msg,tmpsze,tmpbuf,IQ_TYPE,0);

			RMsgSndAdd(&msg,sizeof(unsigned int)*2*iq->tbadtr,
				   	(unsigned char *) badtr,BADTR_TYPE,0);

			RMsgSndAdd(&msg,strlen(sharedmemory)+1,(unsigned char *) sharedmemory,
				   	IQS_TYPE,0);

			tmpbuf=RawFlatten(raw,prm->nrang,prm->mplgs,&tmpsze);
			RMsgSndAdd(&msg,tmpsze,tmpbuf,RAW_TYPE,0);

			tmpbuf=FitFlatten(fit,prm->nrang,&tmpsze);
			RMsgSndAdd(&msg,tmpsze,tmpbuf,FIT_TYPE,0);

			RMsgSndAdd(&msg,strlen(progname)+1,(unsigned char *) progname,
				   	NME_TYPE,0);

			for (n=0;n<tnum;n++) RMsgSndSend(task[n].sock,&msg);

			for (n=0;n<msg.num;n++) {
				if (msg.data[n].type==PRM_TYPE) free(msg.ptr[n]);
				if (msg.data[n].type==IQ_TYPE) free(msg.ptr[n]);
				if (msg.data[n].type==RAW_TYPE) free(msg.ptr[n]);
				if (msg.data[n].type==FIT_TYPE) free(msg.ptr[n]);
			}

			RadarShell(shell.sock,&rstable);

			if (exitpoll !=0) break;
			scan=0;
			if (skip == (nintgs-1)) break;
			skip++;
			if (backward) {
				bmnum = bbms[skip];
			} else {
				bmnum = fbms[skip];
			}

		} while (1);

		ErrLog(errlog.sock,progname,"Waiting for scan boundary.");
		if ((exitpoll==0) && (scannowait==0)) SiteEndScan(scnsc,scnus);

		/* Here skip is the counter for moving through the beam arrays, bbms and fbms.
		   because the initial skip code has moved outside of the loop to keep from
		   errorneously setting the beam, skip much be reset here.  -KTS 16Oct2012  */
		skip=0;
		/* Subsequently, need to set bmnum to the right value. -KTS 16Oct2012 */
		if (backward) {
			bmnum = bbms[skip];
		} else {
			bmnum = fbms[skip];
		}

	} while (exitpoll==0);

	for (n=0;n<tnum;n++) RMsgSndClose(task[n].sock);

	ErrLog(errlog.sock,progname,"Ending program.");

	SiteExit(0);

	return 0;
}

