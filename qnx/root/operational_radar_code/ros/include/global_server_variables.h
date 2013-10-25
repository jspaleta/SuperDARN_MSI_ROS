#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include "tsg.h"
#include "control_program.h"
#include "site.h" 

#ifndef _GLOBAL_SERVER_H
#define _GLOBAL_SERVER_H


#define SITE_NAME "azr"

#define SITE_DIR "/root/operational_radar_code/site_data/"
//#define SITE_DIR "/tmp/site_data/"

#define IF_ENABLED         0 
#define IF_FREQ            71000 // in KHz

#define FULL_CLR_FREQ_START	8000  // in KHz	
#define FULL_CLR_FREQ_END	20000 // in KHz
#define MAX_CLR_WAIT	0     // in secs	

#define RECV_COMPLEX_SAMPLE_SIZE 32  //IQ together 
#define RECV_REAL_OFFSET 0
#define RECV_IMAG_OFFSET 1

#define Max_Control_THREADS     30
#define Max_Viewer_THREADS     5
#define TIMING_HOST_PORT 45001
#define GPS_HOST_PORT 45004
#define DDS_HOST_PORT 45002
#define RECV_HOST_PORT 45006
#define DIO_HOST_PORT 45005
#ifdef __QNX__
  #define DIO_HOST_IP "127.0.0.1"
  #define TIMING_HOST_IP "127.0.0.1"
  #define DDS_HOST_IP "127.0.0.1"
  #define RECV_HOST_IP "127.0.0.1"
  #define GPS_HOST_IP "127.0.0.1"  
#else
  #define DIO_HOST_IP "127.0.0.1"
  #define TIMING_HOST_IP "127.0.0.1"
  #define DDS_HOST_IP "127.0.0.1"
  #define RECV_HOST_IP "127.0.0.1"
  #define GPS_HOST_IP "127.0.0.1"
#endif

#define MAX_SEQS 4
#define CLIENT 0
#define VIEWER 1
#define WORKER 2

#define RECV_SAMPLE_HEADER 2 
#define RECV_CLRFREQ_SAMPLES  2000

typedef struct _fft_index{ 
// Struct to store and order the values of the fft preserving the index in the original array
	double pwr;
	double apwr;
	double freq;
	double detrend;
	int index;
	int valid;
} t_fft_index;

struct Thread_List_Item {
     struct Thread_List_Item *prev;
     struct Thread_List_Item *next;     
     pthread_t id;
     struct timeval timeout;
     struct timeval last_seen;
     char name[80];
     int type;
     void *data;
};



struct ControlState {
/* State Variables for internal ROS Usage */
     int cancelled;
     int socket;
     char ready;
     char processing;
     int linked; //required: DO NOT SET MANUALLY
     int best_assigned_freq; 
     float best_assigned_noise; 
     int current_assigned_freq; 
     float current_assigned_noise; 
     int gpssecond;
     int gpsnsecond;
     double best_assigned_pwr; 
     double current_assigned_pwr; 
     int freq_change_needed; 
     int tx_sideband; //in kHz 
     int rx_sideband; //in kHz
     int N; 
     struct TSGbuf *pulseseqs[MAX_SEQS]; //array of pulseseq pointers
     struct ControlProgram *linked_program;
     struct timeval trigger_timeout;
//     struct timeval last_trigger_event;
     struct Thread_List_Item *thread;
     t_fft_index *fft_array;
};

struct ControlProgram {
// ros state variables
     struct ControlState *state;
     struct DataPRM *data;
     struct CLRFreqPRM clrfreqsearch; 
     struct ControlPRM *parameters;
     struct RadarPRM *radarinfo;
     uint32_t *main;
     uint64_t main_address;
     uint32_t *back;
     uint64_t back_address;
     int active;
};

struct ClrPwr {
// ros state variables
     double freq;
     double pwr;
};

struct BlackList {
// ros state variables
     int start;
     int end;
     uint64_t program;
};
#define TIMING_REGISTER_SEQ '+'
#define DDS_REGISTER_SEQ '+'

#define DIO_CtrlProg_READY '1'
#define DDS_CtrlProg_READY '1'
#define RECV_CtrlProg_READY '1'
#define TIMING_CtrlProg_READY '1'

#define DIO_CtrlProg_END '@'
#define DDS_CtrlProg_END '@'
#define RECV_CtrlProg_END '@'
#define TIMING_CtrlProg_END '@'

#define TIMING_WAIT 'W'

#define DIO_PRETRIGGER '3'
#define DDS_PRETRIGGER '3'
#define RECV_PRETRIGGER '3'
#define TIMING_PRETRIGGER '3'

#define DIO_TRIGGER '4'
#define TIMING_TRIGGER '4'
#define TIMING_GPS_TRIGGER 'G'

#define RECV_POSTTRIGGER '5'
#define TIMING_POSTTRIGGER '5'

#define RECV_GET_DATA 'd'
#define FULL_CLRFREQ '-'
#define RECV_CLRFREQ 'C'
#define DIO_CLRFREQ 'C'
#define DIO_RXFE_RESET 'r'

#define GPS_GET_HDW_STATUS 'S'
#define DIO_GET_TX_STATUS 'S'

#define DDS_RXFE_SETTINGS 'R'
#define RECV_RXFE_SETTINGS 'R'
#define DIO_RXFE_SETTINGS 'R'

#define DIO_TABLE_SETTINGS 'T'

#define GPS_GET_SOFT_TIME 't'
#define GPS_GET_HDW_STATUS 'S'
#define GPS_GET_EVENT_TIME 'e'
#define GPS_SCHEDULE_SINGLE_SCAN 's'
#define GPS_SCHEDULE_REPEAT_SCAN 'r'
#define GPS_TRIGGER_NOW 'n'
#define GPS_SET_TRIGGER_RATE 'R'
#define GPS_MSG_ERROR 'X'

#define MAX_ERROR 0.002
#define TIME_INTERVAL	100000000



struct DriverMsg {
     char type;
     int status;

};

#define DEFAULT_FREQ 13000
#define SIDEBAND 100
struct FreqTable {
  int num;
  int dfrq;
  int *start;
  int *end;
};

struct tx_status {
  int32_t LOWPWR[MAX_TRANSMITTERS];
  int32_t AGC[MAX_TRANSMITTERS];
  int32_t status[MAX_TRANSMITTERS];
};


struct GPSStatus {
	
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
