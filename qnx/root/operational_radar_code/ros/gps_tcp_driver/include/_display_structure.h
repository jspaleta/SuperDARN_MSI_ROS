#ifndef _DISPLAY_STRUCTURE_H
#define _DISPLAY_STRUCTURE_H

struct display_struct{
	
	int	hardware;
	int	antenna;
	int	lock;
	int	gps_lock;
	int	phase_lock;
	int	reference_lock;
	int	sv[6];
	float	signal[6];
	float	lat;
	float	lon;
	float	alt;
	float	mlat;	
	float	mlon;
	float	malt;
	int	poscnt;
	int	gpssecond;
	int	gpsnsecond;
	int	syssecond;
	int	sysnsecond;
	int	lastsetsec;
	int	lastsetnsec;
	int	nextcomparesec;
	int 	nextcomparensec;
	float	drift;
	float	mdrift;
	int	tcpupdate;
	int	tcpconnected;
	int	timecompareupdate;
	int	timecompareupdateerror;
	int	lasttriggersecond;
	int	lasttriggernsecond;
	int	lasttcpmsg;
	int	intervalmode;
	int	scheduledintervalmode;
	int	oneshot;
	float	settimecomparetime;
	int	triggermode;
	int	ratesynthrate;


};

#endif 
