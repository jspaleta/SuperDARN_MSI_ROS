#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "_prog_conventions.h"
#include "_regs_GC314FS.h"
#include "_regs_GC4016.h"
#include "_filter_coeffs.h"

int _config_GC4016(char *BASE1){

	int		i=0;
	unsigned int	chan_control_map;
	int		test[3]={3,2,6};
	struct		timespec sleep;

	sleep.tv_sec=0;
	sleep.tv_nsec=50000;

	// set GEN_SYNC registers to 0
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_GENERAL_SYNC))		= 0;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_GENERAL_SYNC))		= 0;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_GENERAL_SYNC))		= 0;
	// write GLOBAL_RESET register for configuration
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_GLOBAL_RESET))		= 0xf8;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_GLOBAL_RESET))		= 0xf8;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_GLOBAL_RESET))		= 0xf8;

    /* PROGRAM GC4016-1 *****************************************************************/
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_N_CHANNELS_OUT))		= 0x23;
	printf("N_CHANNELS_OUT:		0x%x\n", *((uint08*)(BASE1+GC314FS_GC1offset+GC4016_N_CHANNELS_OUT)));
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_MISCELLANEOUS_CONTROL))	= 0x02;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_N_MULTIPLES))		= 0x07;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_FILTER_SELECT))		= 0x00;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_FINAL_SHIFT))		= 0x14;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_CHANNEL_MAP))		= 0xe4;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_ADD_TO))			= 0x70;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_RESAMPLER_CLOCK_DIVIDE))	= 0x00;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_RATIO_MAP))			= 0xe4;
	*((uint32*)(BASE1+GC314FS_GC1offset+GC4016_RATIO_0))			= 0x04000000;
	*((uint32*)(BASE1+GC314FS_GC1offset+GC4016_RATIO_1))			= 0x04000000;
	*((uint32*)(BASE1+GC314FS_GC1offset+GC4016_RATIO_2))			= 0x04000000;
	*((uint32*)(BASE1+GC314FS_GC1offset+GC4016_RATIO_3))			= 0x04000000;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_GENERAL_SYNC))		= 0x00;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_COUNT_SYNC))			= 0xc0;
	*((uint16*)(BASE1+GC314FS_GC1offset+GC4016_COUNTER))			= 0xffff;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_TRISTATE_CONTROLS))		= 0xff;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_OUTPUT_FORMAT))		= 0x4e;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_OUTPUT_MODE))		= 0x6c;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_OUTPUT_FRAME_CONTROL))	= 0xc0;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_OUTPUT_WORD_SIZES))		= 0xef;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_OUTPUT_CLOCK_CONTROL))	= 0x03;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_SERIAL_MUX_CONTROL))		= 0xe4;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_OUTPUT_TAG_A))		= 0x10;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_OUTPUT_TAG_B))		= 0x32;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_OUTPUT_TAG_C))		= 0x54;
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_OUTPUT_TAG_D))		= 0x76;
	for(i=0;i<256;i++){
		*((uint16*)(BASE1+GC314FS_GC1offset+GC4016_RESAMP_COEFFS+2*i))	= 0;
	}
	*((uint16*)(BASE1+GC314FS_GC1offset+GC4016_RESAMP_COEFFS))		= 1024;
	// configure individual channels on GC4016-1
	for(chan_control_map=0;chan_control_map<=0x180;chan_control_map+=0x80){
		*((uint16*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_PHASE))			= 0x0000;
		*((uint32*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_FREQ))			= 0x46666666;
		*((uint08*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_CH_RESET))			= 0x0c;
		*((uint08*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_FREQ_SYNC))			= 0x77;
		*((uint08*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_NCO_SYNC))			= 0x72;
		*((uint08*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_ZERO_PAD_MODE_CTRL))	= 0x00;
		*((uint08*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_DEC_FLUSH_SYNC))		= 0x22;
		*((uint16*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_DEC_RATIO))		= 0x703f;
		*((uint08*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_CIC_SCALE))		= 0x64;
		*((uint08*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_SPLIT_IQ))			= 0x00;
		*((uint08*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_CFIR))			= 0x00;
		*((uint08*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_PFIR))			= 0x00;
		*((uint08*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_INPUT))			= 0x00;
		*((uint08*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_PEAK_CTRL))		= 0x00;
		*((uint08*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_PEAK_COUNT))		= 0x00;
		//*((uint16*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_FINE_GAIN))		= 0x042e;
		*((uint16*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_FINE_GAIN))		= 0x0300;
		for(i=0;i<11;i++){
			*((uint16*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_CFIR_COEFFS+2*i))	= cfir_coeffs[i];
		}
		for(i=0;i<32;i++){
			*((uint16*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_PFIR_COEFFS+2*i))	= pfir_coeffs[i];
		}
	}
    /***********************************************************************************/
    /* PROGRAM GC4016-1 *****************************************************************/
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_N_CHANNELS_OUT))		= 0x23;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_MISCELLANEOUS_CONTROL))	= 0x02;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_N_MULTIPLES))		= 0x07;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_FILTER_SELECT))		= 0x00;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_FINAL_SHIFT))		= 0x14;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_CHANNEL_MAP))		= 0xe4;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_ADD_TO))			= 0x70;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_RESAMPLER_CLOCK_DIVIDE))	= 0x00;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_RATIO_MAP))			= 0xe4;
	*((uint32*)(BASE1+GC314FS_GC2offset+GC4016_RATIO_0))			= 0x04000000;
	*((uint32*)(BASE1+GC314FS_GC2offset+GC4016_RATIO_1))			= 0x04000000;
	*((uint32*)(BASE1+GC314FS_GC2offset+GC4016_RATIO_2))			= 0x04000000;
	*((uint32*)(BASE1+GC314FS_GC2offset+GC4016_RATIO_3))			= 0x04000000;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_GENERAL_SYNC))		= 0x00;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_COUNT_SYNC))			= 0xc0;
	*((uint16*)(BASE1+GC314FS_GC2offset+GC4016_COUNTER))			= 0xffff;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_TRISTATE_CONTROLS))		= 0xff;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_OUTPUT_FORMAT))		= 0x4e;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_OUTPUT_MODE))		= 0x6c;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_OUTPUT_FRAME_CONTROL))	= 0xc0;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_OUTPUT_WORD_SIZES))		= 0xef;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_OUTPUT_CLOCK_CONTROL))	= 0x03;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_SERIAL_MUX_CONTROL))		= 0xe4;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_OUTPUT_TAG_A))		= 0x10;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_OUTPUT_TAG_B))		= 0x32;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_OUTPUT_TAG_C))		= 0x54;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_OUTPUT_TAG_D))		= 0x76;
	for(i=0;i<256;i++){
		*((uint16*)(BASE1+GC314FS_GC2offset+GC4016_RESAMP_COEFFS+2*i))	= 0;
	}
	*((uint16*)(BASE1+GC314FS_GC2offset+GC4016_RESAMP_COEFFS))		= 1024;
	// configure individual channels on GC4016-1
	for(chan_control_map=0;chan_control_map<=0x180;chan_control_map+=0x80){
		*((uint16*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_PHASE))			= 0x0000;
		*((uint32*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_FREQ))			= 0x46666666;
		*((uint08*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_CH_RESET))			= 0x0c;
		*((uint08*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_FREQ_SYNC))			= 0x77;
		*((uint08*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_NCO_SYNC))			= 0x72;
		*((uint08*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_ZERO_PAD_MODE_CTRL))	= 0x00;
		*((uint08*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_DEC_FLUSH_SYNC))		= 0x22;
		*((uint16*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_DEC_RATIO))		= 0x703f;
		*((uint08*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_CIC_SCALE))		= 0x64;
		*((uint08*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_SPLIT_IQ))			= 0x00;
		*((uint08*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_CFIR))			= 0x00;

		*((uint08*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_PFIR))			= 0x00;
		*((uint08*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_INPUT))			= 0x00;
		*((uint08*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_PEAK_CTRL))		= 0x00;
		*((uint08*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_PEAK_COUNT))		= 0x00;
		//*((uint16*)(BASE1+GC314FS_GC1offset+chan_control_map+GC4016_FINE_GAIN))		= 0x042e;
		*((uint16*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_FINE_GAIN))		= 0x0300;
		for(i=0;i<11;i++){
			*((uint16*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_CFIR_COEFFS+2*i))	= cfir_coeffs[i];
		}
		for(i=0;i<32;i++){
			*((uint16*)(BASE1+GC314FS_GC2offset+chan_control_map+GC4016_PFIR_COEFFS+2*i))	= pfir_coeffs[i];
		}
	}
    /***********************************************************************************/
    /* PROGRAM GC4016-1 *****************************************************************/
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_N_CHANNELS_OUT))		= 0x23;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_MISCELLANEOUS_CONTROL))	= 0x02;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_N_MULTIPLES))		= 0x07;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_FILTER_SELECT))		= 0x00;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_FINAL_SHIFT))		= 0x14;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_CHANNEL_MAP))		= 0xe4;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_ADD_TO))			= 0x70;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_RESAMPLER_CLOCK_DIVIDE))	= 0x00;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_RATIO_MAP))			= 0xe4;
	*((uint32*)(BASE1+GC314FS_GC3offset+GC4016_RATIO_0))			= 0x04000000;
	*((uint32*)(BASE1+GC314FS_GC3offset+GC4016_RATIO_1))			= 0x04000000;

	*((uint32*)(BASE1+GC314FS_GC3offset+GC4016_RATIO_2))			= 0x04000000;
	*((uint32*)(BASE1+GC314FS_GC3offset+GC4016_RATIO_3))			= 0x04000000;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_GENERAL_SYNC))		= 0x00;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_COUNT_SYNC))			= 0xc0;
	*((uint16*)(BASE1+GC314FS_GC3offset+GC4016_COUNTER))			= 0xffff;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_TRISTATE_CONTROLS))		= 0xff;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_OUTPUT_FORMAT))		= 0x4e;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_OUTPUT_MODE))		= 0x6c;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_OUTPUT_FRAME_CONTROL))	= 0xc0;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_OUTPUT_WORD_SIZES))		= 0xef;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_OUTPUT_CLOCK_CONTROL))	= 0x03;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_SERIAL_MUX_CONTROL))		= 0xe4;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_OUTPUT_TAG_A))		= 0x10;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_OUTPUT_TAG_B))		= 0x32;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_OUTPUT_TAG_C))		= 0x54;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_OUTPUT_TAG_D))		= 0x76;
	for(i=0;i<256;i++){
		*((uint16*)(BASE1+GC314FS_GC3offset+GC4016_RESAMP_COEFFS+2*i))	= 0;
	}
	*((uint16*)(BASE1+GC314FS_GC3offset+GC4016_RESAMP_COEFFS))		= 1024;
	// configure individual channels on GC4016-1
	for(chan_control_map=0;chan_control_map<=0x180;chan_control_map+=0x80){
		*((uint16*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_PHASE))			= 0x0000;
		*((uint32*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_FREQ))			= 0x46666666;
		*((uint08*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_CH_RESET))			= 0x0c;
		*((uint08*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_FREQ_SYNC))			= 0x77;
		*((uint08*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_NCO_SYNC))			= 0x72;
		*((uint08*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_ZERO_PAD_MODE_CTRL))	= 0x00;
		*((uint08*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_DEC_FLUSH_SYNC))		= 0x22;
		*((uint16*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_DEC_RATIO))		= 0x703f;
		*((uint08*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_CIC_SCALE))		= 0x64;
		*((uint08*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_SPLIT_IQ))			= 0x00;
		*((uint08*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_CFIR))			= 0x00;
		*((uint08*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_PFIR))			= 0x00;
		*((uint08*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_INPUT))			= 0x00;
		*((uint08*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_PEAK_CTRL))		= 0x00;
		*((uint08*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_PEAK_COUNT))		= 0x00;
		//*((uint16*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_FINE_GAIN))		= 0x042e;
		*((uint16*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_FINE_GAIN))		= 0x0300;
		for(i=0;i<11;i++){
			*((uint16*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_CFIR_COEFFS+2*i))	= cfir_coeffs[i];
		}
		for(i=0;i<32;i++){
			*((uint16*)(BASE1+GC314FS_GC3offset+chan_control_map+GC4016_PFIR_COEFFS+2*i))	= pfir_coeffs[i];
		}
	}
    /***********************************************************************************/

	// activate SYNC 1
	*((uint32*)(BASE1+GC314FS_GSM))		 			 = 0x00008000;
	*((uint32*)(BASE1+GC314FS_GCSR))				 = 0x01200814;
	// set GC4016s to 16 bit mode
	*((uint32*)(BASE1+GC314FS_GCSR))				 = 0x01200814;
	// release the gc4016 resets
	*((uint08*)(BASE1+GC314FS_GC1offset+GC4016_GLOBAL_RESET))	 = 0x08;
	*((uint08*)(BASE1+GC314FS_GC2offset+GC4016_GLOBAL_RESET))	 = 0x08;
	*((uint08*)(BASE1+GC314FS_GC3offset+GC4016_GLOBAL_RESET))	 = 0x08;
	// de-activate SYNC 1
	*((uint32*)(BASE1+GC314FS_GCSR))				 = 0x01000814;
	// set RDA mode to continuous
	*((uint16*)(BASE1+GC314FS_R1ACSR))				|= 0x0003;
	*((uint16*)(BASE1+GC314FS_R1BCSR))				|= 0x0003;
	*((uint16*)(BASE1+GC314FS_R1CCSR))				|= 0x0003;
	*((uint16*)(BASE1+GC314FS_R1DCSR))				|= 0x0003;
	*((uint16*)(BASE1+GC314FS_R2ACSR))				|= 0x0003;
	*((uint16*)(BASE1+GC314FS_R2BCSR))				|= 0x0003;
	*((uint16*)(BASE1+GC314FS_R2CCSR))				|= 0x0003;
	*((uint16*)(BASE1+GC314FS_R2DCSR))				|= 0x0003;
	*((uint16*)(BASE1+GC314FS_R3ACSR))				|= 0x0003;
	*((uint16*)(BASE1+GC314FS_R3BCSR))				|= 0x0003;
	*((uint16*)(BASE1+GC314FS_R3CCSR))				|= 0x0003;
	*((uint16*)(BASE1+GC314FS_R3DCSR))				|= 0x0003;
	// disable external trigger
	*((uint32*)(BASE1+GC314FS_GCSR))				 = 0x1000814;
	// disable IWB bypass
	*((uint32*)(BASE1+GC314FS_IWBCSR))				 = 0x10043;
	// set IWB source as A/D 1 input 
	*((uint32*)(BASE1+GC314FS_IWBCSR))				 = 0x1004b;
	// select GC4016s to be driven with data straight from A/Ds
	*((uint32*)(BASE1+GC314FS_IWBCSR))				 = 0x1004b;
	// set IWB FIFO level	
	*((uint32*)(BASE1+GC314FS_IWBLVL))				 = 8192;
	// set GC4016 FIFO levels
	*((uint32*)(BASE1+GC314FS_R1ALVL))				 = 512;
	*((uint32*)(BASE1+GC314FS_R1BLVL))				 = 512;
	*((uint32*)(BASE1+GC314FS_R1CLVL))				 = 512;
	*((uint32*)(BASE1+GC314FS_R1DLVL))				 = 512;
	*((uint32*)(BASE1+GC314FS_R2ALVL))				 = 512;
	*((uint32*)(BASE1+GC314FS_R2BLVL))				 = 512;
	*((uint32*)(BASE1+GC314FS_R2CLVL))				 = 512;
	*((uint32*)(BASE1+GC314FS_R2DLVL))				 = 512;
	*((uint32*)(BASE1+GC314FS_R3ALVL))				 = 512;
	*((uint32*)(BASE1+GC314FS_R3BLVL))				 = 512;
	*((uint32*)(BASE1+GC314FS_R3CLVL))				 = 512;
	*((uint32*)(BASE1+GC314FS_R3DLVL))				 = 512;
	// set number of samples to be collected during burst collection
	*((uint32*)(BASE1+GC314FS_IWBDWS))				 = 8192;
	*((uint32*)(BASE1+GC314FS_R1ADWS))				 = 8192;
	*((uint32*)(BASE1+GC314FS_R1BDWS))				 = 8192;
	*((uint32*)(BASE1+GC314FS_R1CDWS))				 = 8192;
	*((uint32*)(BASE1+GC314FS_R1DDWS))				 = 8192;
	*((uint32*)(BASE1+GC314FS_R2ADWS))				 = 8192;
	*((uint32*)(BASE1+GC314FS_R2BDWS))				 = 8192;
	*((uint32*)(BASE1+GC314FS_R2CDWS))				 = 8192;
	*((uint32*)(BASE1+GC314FS_R2DDWS))				 = 8192;
	*((uint32*)(BASE1+GC314FS_R3ADWS))				 = 8192;
	*((uint32*)(BASE1+GC314FS_R3BDWS))				 = 8192;
	*((uint32*)(BASE1+GC314FS_R3CDWS))				 = 8192;
	*((uint32*)(BASE1+GC314FS_R3DDWS))				 = 8192;
	// set number of samples to skip before data collection
	*((uint32*)(BASE1+GC314FS_IWBSKIP))				 = 0;
	*((uint32*)(BASE1+GC314FS_R1ASKIP))				 = 0;
	*((uint32*)(BASE1+GC314FS_R1BSKIP))				 = 0;
	*((uint32*)(BASE1+GC314FS_R1CSKIP))				 = 0;
	*((uint32*)(BASE1+GC314FS_R1DSKIP))				 = 0;
	*((uint32*)(BASE1+GC314FS_R2ASKIP))				 = 0;
	*((uint32*)(BASE1+GC314FS_R2BSKIP))				 = 0;
	*((uint32*)(BASE1+GC314FS_R2CSKIP))				 = 0;
	*((uint32*)(BASE1+GC314FS_R2DSKIP))				 = 0;
	*((uint32*)(BASE1+GC314FS_R3ASKIP))				 = 0;
	*((uint32*)(BASE1+GC314FS_R3BSKIP))				 = 0;
	*((uint32*)(BASE1+GC314FS_R3CSKIP))				 = 0;
	*((uint32*)(BASE1+GC314FS_R3DSKIP))				 = 0;

	return 1;

}
