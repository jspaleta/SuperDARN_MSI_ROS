/* themisscan.c
 ============
 Author: J.Spaleta
 With Modifications by M. McClorey
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
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with RST. If not, see <http://www.gnu.org/licenses/>.



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

char progid[80]={"themisscan"};
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
	{"127.0.0.1",1,-1}, /* iqwrite */
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
/*	char tempLog[40];	Beam log output? Is this necessary? */
/*	char diagtxt[1024]="";  */

	int exitpoll=0;
	int scannowait=0;

	int scnsc=120;
	int scnus=0;
	int skip;
	int skipsc= 3;
	int skipus= 0;
	int cnt=0;
	int i,n;

	unsigned char discretion=0;

	int status=0;
	int fixfrq=-1;   /* fixfrq variable to override clear frequency search */
	int cbm=10;	/* default camping beam */
	int tempF, tempB;

	/* new variables for dynamically creating beam sequences */
	int *intgt;
	int *fbms;						/* forward  scanning beams */
	int *bbms;						/* backward scanning beams */
	int nintgs;						/* number of integration periods per scan */
	int integ_durr = 3;		/* Themis instruments make measurements every 3 s */

  /* standard radar defaults */
	cp=3300;							/* themisscan cpid */
	intsc=2;						/* Set intsc and intus slightly less than */
	intus=900000;						/* integ_durr to allow for overhead compuation  -KTS 23Oct2013 */
	mppul=8;
	mplgs=23;
	mpinc=1500;
	dmpinc=1500;
	nrang=100;
	rsep=45;
	txpl=300;             /* note: recomputed below */
/*	dfrq=10200;   */

	/* ========= PROCESS COMMAND LINE ARGUMENTS ============= */

	OptionAdd(&opt,"di",    'x',&discretion); /* Discretionary flag */

	OptionAdd(&opt,"frang", 'i',&frang);
	OptionAdd(&opt,"rsep",  'i',&rsep);
	OptionAdd(&opt,"nrang",   'i',&nrang);

	OptionAdd(&opt,"dt",    'i',&day);	/* Set day starting hour */
	OptionAdd(&opt,"nt",    'i',&night);    /* Set night starting hour */
	OptionAdd(&opt,"df",    'i',&dfrq);     /* Set day frequency */
	OptionAdd(&opt,"nf",    'i',&nfrq);	/* Set night frequency */
	OptionAdd(&opt,"xcf",   'i',&xcnt);	/* Set number of scans between elevation measurement */

	OptionAdd(&opt,"sb",    'i',&sbm);	/* Set first beam of scan */
	OptionAdd(&opt,"eb",    'i',&ebm);	/* Set last beam of scan */

	OptionAdd(&opt,"ep",    'i',&errlog.port);
	OptionAdd(&opt,"sp",    'i',&shell.port);

	OptionAdd(&opt,"bp",    'i',&baseport);

	OptionAdd(&opt,"stid",  't',&ststr);	/* Set which radar the cp is to run on */

	OptionAdd(&opt,"fixfrq",'i',&fixfrq);		/* fix the transmit frequency */
	OptionAdd(&opt,"camp",  'i',&cbm);			/* camping beam number */

	/* Process all of the command line options
	Important: need to do this here because we
	need stid and ststr   */

	arg=OptionProcess(1,argc,argv,&opt,NULL);


	if (ststr==NULL) ststr=dfststr;

	/* If the roshost hasn't been set, get it from the
	   environment variables */
	if (roshost==NULL) roshost=getenv("ROSHOST");
	/* If the roshost still isn't set, put in a default
	   value that is hardcoded above */
	if (roshost==NULL) roshost=droshost;

	if ((errlog.sock=TCPIPMsgOpen(errlog.host,errlog.port))==-1) {
		fprintf(stderr,"Error connecting to error log.\n");
	}

	if ((shell.sock=TCPIPMsgOpen(shell.host,shell.port))==-1) {
		fprintf(stderr,"Error connecting to shell.\n");
	}

	for (n=0;n<tnum;n++) task[n].port+=baseport;

	/* Location: ~/rst/usr/codebase/superdarn/src.lib/os/ops.1.10/src/setup.c */
	OpsStart(ststr);

	status=SiteBuild(ststr,NULL); /*second argument is version string */

	if (status==-1) {
		fprintf(stderr,"Could not identify station.\n");
		exit(1);
	}

	/* dump beams to log file */
/*	ErrLog(errlog.sock,progname,logtxt);
	for (i=0; i<nintgs; i++){
		sprintf(tempLog, "%3d", fbms[i]);
		strcat(logtxt, tempLog);
	}
	ErrLog(errlog.sock,progname,logtxt);
	Is this necessary outside of debugging? -KTS 21Oct2013
*/
	/* IMPORTANT: sbm and ebm are reset by this function */
	SiteStart(roshost);

	/* Reprocess the command line to restore desired parameters */
	arg=OptionProcess(1,argc,argv,&opt,NULL);


	/* Create a list of the beams that will be integrated on, alternating
	 * between the camping beam and each other beam in turn */

	/* number of integration periods possible in scan time */
	nintgs = (int)floor((scnsc+scnus*1e-6 - 3)/integ_durr);

	/* arrays for integration start times and beam sequences */
	intgt = (int *)malloc(nintgs*sizeof(int));
	fbms  = (int *)malloc(nintgs*sizeof(int));
	bbms  = (int *)malloc(nintgs*sizeof(int));

	for (i=0; i<nintgs; i++)
		intgt[i] = i*integ_durr;				/* start time of each integration period */

	/* If backward is set for West radar, start and end beams need to be reversed for the
	 * beam assigning code that follows until "End of Dartmouth Mods".  Usual SuperDARN
	 * logic follows that for West radars sbm >= ebm and East radars sbm <= ebm.
	 * However, the Dartmouth code for assigning the beam number arrays, fbms & bbms,
	 * does not follow usual SuperDARN logic and always assumes sbm <= ebm.  -KTS 2/20/2012 */

	if (backward == 1) {
		i = sbm;
		sbm = ebm;
		ebm = i;
	}

	/* Special case for when the first beam of a scan is
	   the camping beam */
	if (sbm == cbm) {
		fbms[0] = sbm;
		for(i = 1; i<nintgs; i++){
			if((i%2 == 1)&&((i+1)/2+sbm<=ebm)) fbms[i]=(i+1)/2+sbm;
			else fbms[i] = cbm;
		}
		for(i = 0; i<nintgs; i++){
			if((i%2 == 0)&&(ebm-(i/2)>sbm))	bbms[i]=ebm-(i/2);
			else bbms[i] = cbm;
		}
	}
	/* Other special case for when the last beam of a
	   scan is the camping beam */
	else if (ebm == cbm){
		bbms[0] = ebm;
		for(i = 1; i<nintgs; i++){
			if((i%2 == 1)&&((ebm-(i+1)/2)>=sbm)) bbms[i]=ebm-(i+1)/2;
			else bbms[i] = cbm;
		}
		for(i = 0; i<nintgs; i++){
			if((i%2 == 0)&&(i/2+sbm<=ebm)) fbms[i]=i/2+sbm;
			else fbms[i] = cbm;
		}
	}
	/* Otherwise, the case below will work for camping
	   beams that are not on the ends of the scan */
	else {
		tempB = ebm;
		tempF = sbm;
		for(i = 0; i<nintgs; i++){
			if((i%2 == 0)&&(tempF<=ebm)) {
				/* exclude the camping beam from being included as a regular beam */
				if (tempF == cbm) tempF++;
				if (tempB == cbm) tempB--;
				fbms[i] = tempF;
				bbms[i] = tempB;
				tempF++;
				tempB--;
			} else {
				fbms[i] = cbm;
				bbms[i] = cbm;
			}
		}
	}
	/* not sure if -nrang commandline option works */

	strncpy(combf,progid,80);

	/* Location of OpsSetupCommand: rst/usr/codebase/superdarn/src.lib/os/ops.1.10/src */
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

	if (discretion) cp = -cp;

	txpl=(rsep*20)/3;		/* computing TX pulse length */

	sprintf(progname,"themisscan");

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

	printf("Entering Scan loop Station ID: %s  %d\n",ststr,stid);
	do {

		printf("Preparing SiteStartScan Station ID: %s  %d\n",ststr,stid);
		if (SiteStartScan() !=0) continue;

		if (OpsReOpen(2,0,0) !=0) {
			ErrLog(errlog.sock,progname,"Opening new files.");
			for (n=0;n<tnum;n++) {
				RMsgSndClose(task[n].sock);
				RMsgSndOpen(task[n].sock,strlen( (char *) command),command);
			}
		}

		/* Initialize the scan variable.  This usually signifies
		   the beginning of the point of repetition. */
		scan=1;

		ErrLog(errlog.sock,progname,"Starting scan.");

		if (xcnt>0) {
			cnt++;
			if (cnt==xcnt) {
				xcf=1;
				cnt=0;
			} else xcf=0;
		} else xcf=0;

		skip=OpsFindSkip(scnsc,scnus);

		/* is this C99? why is this a sequential control structure with variables
		 * being declared and not attached to anything? */
		{
			int tv;
			int bv;
			int iv;
			TimeReadClock(&yr,&mo,&dy,&hr,&mt,&sc,&us);
			iv= skipsc*1000000 + skipus;
			bv= scnsc* 1000000 + scnus;
			tv=(mt* 60 + sc)* 1000000 + us + iv/2 - 100000;
			skip=(tv % bv)/iv;
			if (skip> nintgs-1) skip=0;
			if (skip<0) skip=0;
		}
		if (backward) {
			bmnum= bbms[skip];
		} else {
			bmnum= fbms[skip];
		}

		/* Now entering the loop for each beam sounding.  Each
		   time through the loop is run with a different bmnum
		   which is iterrated at the end of the loop */

		do {

			/* Synchronize to the desired start time */


			/* This will only work, if the total time through the do loop is < 3s */
			/* If this is not the case, decrease the Integration time */
			/* MAX < or <=  3s ? */
			/* once again, don't like this... */

			{
				int t_now;
				int t_dly;
				TimeReadClock( &yr, &mo, &dy, &hr, &mt, &sc, &us);
				t_now= ( (mt* 60 + sc)* 1000 + us/ 1000 ) %
				(scnsc* 1000 + scnus/ 1000);
				t_dly= intgt[skip]* 1000 - t_now;
				if (t_dly > 0) usleep( t_dly);
			}

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
			if (fixfrq>0) {
				stfrq=fixfrq;
				tfreq=fixfrq;
				noise=0;
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

			if(fixfrq<0) {
				tfreq=SiteFCLR(stfrq,stfrq+frqrng);
			}
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
	} while (exitpoll==0);

	for (n=0;n<tnum;n++) RMsgSndClose(task[n].sock);

	ErrLog(errlog.sock,progname,"Ending program.");

	SiteExit(0);

	return 0;
}

