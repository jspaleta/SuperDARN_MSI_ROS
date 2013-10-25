#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include "tsg.h"
#include "rtypes.h"

#ifndef _CONTROL_PROGRAM_H
#define _CONTROL_PROGRAM_H

#define ROS_IP "127.0.0.1"
#define ROS_PORT 45000


struct TRTimes {
  int32_t length;
  uint32_t *start_usec;  /* unsigned int32 pointer */
  uint32_t *duration_usec; /* unsigned int32 pointer */
};

struct SeqPRM {
  uint32_t index;
  uint32_t len;
  uint32_t step;  //packed timesequence stepsize in microseconds
  uint32_t samples;
  uint32_t smdelay;
};


//
//  unsigned int main_p; /* unsigned int pointer */
//  unsigned int  main_address;
//  unsigned int back_p; /* unsigned int pointer */
//  unsigned int  back_address;
//  unsigned int agc_p;  /* int pointer */
//  unsigned int lopwr_p; /* int pointer */
//

/*  struct TRTimes tr_times;*/
struct DataPRM {
  uint32_t event_secs;
  uint32_t event_nsecs;
  int32_t samples;
  int32_t shm_memory;
  int32_t status;
  int32_t frame_header;
  int32_t bufnum;
};

struct RadarPRM {
     int32_t site;
     int32_t radar; //required: DO NOT SET MANUALLY
     int32_t channel; //required: DO NOT SET MANUALLY
};

struct RXFESettings {
     uint32_t ifmode;  // IF Enabled
     uint32_t amp1;    // Stage 1 Amp 20 db before IF mixer
     uint32_t amp2;    // Stage 2 Amp 10 db after IF mixer
     uint32_t amp3;    // Stage 3 Amp 10 db after IF mixer
     uint32_t att1;    // 1/2 db Attenuator
     uint32_t att2;    //  1  db Attenuator
     uint32_t att3;    //  2  db Attenuator 
     uint32_t att4;    //  4  db Attenuator
};
/*
struct ChannelStatus {
     int32 active;
};
*/

struct SiteSettings {
     uint32_t num_radars;
     uint32_t ifmode;
     uint32_t use_beam_table;
     char name[80];
     char beam_table_1[256];
     char beam_table_2[256];
     struct RXFESettings rf_settings;  /* reciever front end settings for this site */
     struct RXFESettings if_settings;  /* reciever front end settings for this site */
};


struct ControlPRM {
     int32_t radar; //required: DO NOT SET MANUALLY
     int32_t channel; //required: DO NOT SET MANUALLY
     int32_t local;  //if local use shared memory for data handling else use tcp
     int32_t priority; //optional: valid 0-99: lower value higher priority when running multiple operational programs
     int32_t current_pulseseq_index; //required: registered pulse sequence to use 
// transmit beam definition one of these needs to be non-zero
     int32_t tbeam;  //required: valid 0-31: defines standard look directions 
     uint32_t tbeamcode; //optional: used for special beam directions, used only if beam is invalid value.
// Imaging transmit beam options
     float tbeamazm; //optional: used for imaging radar
     float tbeamwidth; //optional: used for imaging radar
//transmit setup
     int32_t tfreq;  //required: transmit freq in kHz 
     int32_t trise;  // required: rise time in microseconds
//reciever setup
     int32_t number_of_samples;  //required: number of recv samples to collect 
     int32_t buffer_index; //required: valid 0-1: DMA buffer to use for recv
     float baseband_samplerate; //required: normally equals (nbaud/txpl) but can be changed for oversampling needs
     int32_t filter_bandwidth; //required: normally equals basebad_samplerate but can be changed for oversampling needs
     int32_t match_filter;  // required: valid 0-1: whether to use match filter, normally equal 1 
     int32_t rfreq;  //optional: if invalid value tfreq is used
// reciever beam definitions: only used if tbeam is invalid
     int32_t rbeam;  //optional: valid 0-31: defines standard look directions: if invalid tbeam is used 
     uint32_t rbeamcode; //optional: used for special beam directions, used only if rbeam and tbeam is invalid value.
// Imaging receiver beam options
     float rbeamazm; //optional: used for imaging radar
     float rbeamwidth; //optional: used for imaging radar
// ROS Feedback
     int32_t status; // coded value: non-zero values will code to an error msg
     char name[80]; //optional: but a very good idea to set
     char description[120]; //optional: but a very good idea to set
};

struct ROSMsg {
     int32_t status;
     char type;
};

struct CLRFreqPRM {
     int32_t start; //In kHz
     int32_t end;  //in kHz
     float filter_bandwidth;  //in kHz  typically c/(rsep*2)
     float pwr_threshold;  //  typical value 0.9: power at best available frequency must be 90% or less of current assigned 
     int32_t nave;  // Number of passes to average.
};
//ROSMsg type definitions:
#define SET_RADAR_CHAN 'R'
#define SET_INACTIVE 'a'
#define SET_ACTIVE 'A'
#define GET_SITE_SETTINGS 's'
#define UPDATE_SITE_SETTINGS 'S'
#define QUERY_INI_SETTING 'i'
#define SET_SITE_IFMODE 'I'
#define GET_PARAMETERS 'c'
#define SET_PARAMETERS 'C'

#define PING '='
#define OKAY '^'
#define NOOP '~'
#define QUIT '.'

#define REGISTER_SEQ '+'
#define REMOVE_SEQ '-'

#define REQUEST_ASSIGNED_FREQ '>'
#define REQUEST_CLEAR_FREQ_SEARCH '<'
#define LINK_RADAR_CHAN 'L'

#define SET_READY_FLAG '1'
#define UNSET_READY_FLAG '!'
#define SET_PROCESSING_FLAG '2'
#define UNSET_PROCESSING_FLAG '@'

#define GET_DATA 'd'

#endif
