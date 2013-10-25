#ifndef gc314fs_h
#define gc314fs_h
//DEFINE THE NUMBER OF GC314FS CARDS IN THE SYSTEM
#define NUMBER_OF_CARDS		2
#define SYNC_MASTER		-1
#define SYNC_TERM	        -1

//CHANNEL IDENTIFIERS
#define CHANNEL_A		0
#define CHANNEL_B		1
#define CHANNEL_C		2
#define CHANNEL_D		3

//SETTING FLAGS
#define MATCHED			1
#define NOTMATCHED		0
#define DMA_BUF_SIZE		1048576
#define GC314_ON		1
#define GC314_OFF		0
#define GC314_PULSE		3
#define SYNC1_SIA_ONETIME	11
#define SYNC1_SIB_ONETIME	12
#define SYNC1_RTSC_CLEAR	13
#define SYNC1_SIA_HOLD		14
#define SYNC1_SIA_CONTINUOUS	15

//COMMAND DEFINITIONS
#define BOARD_RESET 		100
#define CHANNEL_ON		116
#define CHANNEL_OFF		117
#define GET_BUFFER_ADDRESS	107
#define LOAD_GC4016S		113
#define SET_CFIR_CHA		102
#define SET_EXTERNAL_TRIGGER	110
#define SET_FILTERS		103
#define SET_FREQUENCY		105
#define SET_GLOBAL_RESET	109
#define SET_OUTPUT_RATE		104
#define SET_SAMPLES		115
#define SET_SYNC1		112
#define SET_SYNC_MASK		111
#define SET_RDA			114
#define START_COLLECTION	118
#define UPDATE_CHANNEL		106
#define ABORT_CHANNEL		120
#define WAIT_FOR_DATA		108

//COMMAND FUNCTION DEFINITIONS
//#define GC314_BOARD_RESET __DION(_DCMD_MISC,BOARD_RESET)
#define GC314_BOARD_RESET		__DIOTF(_DCMD_MISC,BOARD_RESET,int)
#define GC314_CHANNEL_ON		__DIOTF(_DCMD_MISC,CHANNEL_ON,int)
#define GC314_CHANNEL_OFF		__DIOTF(_DCMD_MISC,CHANNEL_OFF,int)
#define GC314_GET_BUFFER_ADDRESS	__DIOTF(_DCMD_MISC,GET_BUFFER_ADDRESS,int)
#define GC314_LOAD_GC4016S		__DIOTF(_DCMD_MISC,LOAD_GC4016S,int)
#define GC314_SET_CFIR_CHA 		__DIOTF(_DCMD_MISC,SET_CFIR_CHA,int)
#define GC314_SET_EXTERNAL_TRIGGER	__DIOTF(_DCMD_MISC,SET_EXTERNAL_TRIGGER,int)
#define GC314_SET_FILTERS 		__DIOTF(_DCMD_MISC,SET_FILTERS,int)
#define GC314_SET_FREQUENCY 		__DIOTF(_DCMD_MISC,SET_FREQUENCY,int)
#define GC314_SET_GLOBAL_RESET 		__DIOTF(_DCMD_MISC,SET_GLOBAL_RESET,int)
#define GC314_SET_OUTPUT_RATE	 	__DIOTF(_DCMD_MISC,SET_OUTPUT_RATE,int)
#define GC314_SET_SAMPLES	 	__DIOTF(_DCMD_MISC,SET_SAMPLES,int)
#define GC314_SET_SYNC1		 	__DIOTF(_DCMD_MISC,SET_SYNC1,int)
#define GC314_SET_SYNC_MASK	 	__DIOTF(_DCMD_MISC,SET_SYNC_MASK,int)
#define GC314_SET_RDA		 	__DIOTF(_DCMD_MISC,SET_RDA,int)
#define GC314_START_COLLECTION	 	__DIOTF(_DCMD_MISC,START_COLLECTION,int)
#define GC314_UPDATE_CHANNEL	 	__DIOTF(_DCMD_MISC,UPDATE_CHANNEL,int)
#define GC314_ABORT_CHANNEL	 	__DIOTF(_DCMD_MISC,ABORT_CHANNEL,int)
#define GC314_WAIT_FOR_DATA	 	__DIOTF(_DCMD_MISC,WAIT_FOR_DATA,int)
	
//ERROR FLAGS
#define GC314_WRONG_CHANNEL		301

//DEFINE DATA STRUCTURES
struct S_buffer_address {
	int		channel;
	int		chip;
	unsigned long	address;
       };
struct S_external_trigger_data{
	int	state;
	};
struct S_filter_data {
	int 	channel;
	float 	bandwidth;
	int 	matched;
       };
struct S_frequency_data {
	int 	channel;
	int 	freq;
       };
struct S_global_reset_data{
	int	state;
	};
struct S_output_rate_data {
	int 	channel;
	double 	output_rate;
       };
struct S_samples_data {
	int 	channel;
	int 	samples;
       };
struct S_sync1_data{
	unsigned int	state;
	};
struct S_sync_mask_data{
	unsigned int	state;
	};
struct S_RDA_data {
	int 	channel;
       };
struct S_wait_for_data {
	int	channel;
	int	samples_to_wait_for;
	float	wait_timeout;
       };

	

#endif

