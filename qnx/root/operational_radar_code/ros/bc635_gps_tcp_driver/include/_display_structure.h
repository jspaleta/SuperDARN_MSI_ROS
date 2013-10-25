#ifndef _DISPLAY_STRUCTURE_H
#define _DISPLAY_STRUCTURE_H

struct display_struct{
	
	int	lock;
	int	gps_lock;
	int	phase_lock;
	int	reference_lock;
	int	gpssecond;
	int	gpsnsecond;
	int	syssecond;
	int	sysnsecond;
	int	lasttriggersecond;
	int	lasttriggernsecond;
	int	triggermode;
	int	ratesynthrate;


};

#endif 
