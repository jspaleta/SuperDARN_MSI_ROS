#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "_prog_conventions.h"
#include "_regs_GC314FS.h"
#include "_regs_GC4016.h"
#include "_filter_coeffs.h"

int _config_GC4016(char *BASE1, int print, int samples_to_collect){

	int		i=0,j,k;
	//unsigned int	chan_control_map, GC314FS_GCoffset;
	int		test[3]={3,2,6};
	unsigned int	GC314FS_GCoffset[3]={0x800,0x1000,0x1800};
	unsigned int 	chan_control_map[4]={0x00,0x80,0x100,0x180};
	//unsigned int 	chan_control_map;

    /* PROGRAM GC4016-1 *****************************************************************/
	for(k=0;k<3;k++){
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_GENERAL_SYNC,			0x00);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_GLOBAL_RESET,			0xf8);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_N_CHANNELS_OUT,			0x23);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_MISCELLANEOUS_CONTROL,		0x02);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_N_MULTIPLES,			0x07);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_FILTER_SELECT,			0x00);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_FINAL_SHIFT,			0x14);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_CHANNEL_MAP,			0xe4);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_ADD_TO,				0x70);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_RESAMPLER_CLOCK_DIVIDE,		0x00);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_RATIO_MAP,			0xe4);
		write32(BASE1, GC314FS_GCoffset[k]+GC4016_RATIO_0,				0x04000000);
		write32(BASE1, GC314FS_GCoffset[k]+GC4016_RATIO_1,				0x04000000);
		write32(BASE1, GC314FS_GCoffset[k]+GC4016_RATIO_2,				0x04000000);
		write32(BASE1, GC314FS_GCoffset[k]+GC4016_RATIO_3,				0x04000000);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_GENERAL_SYNC,			0x00);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_COUNT_SYNC,			0xc0);
		write16(BASE1, GC314FS_GCoffset[k]+GC4016_COUNTER,				0xffff);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_TRISTATE_CONTROLS,		0xff);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_FORMAT,			0x4e);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_MODE,			0x6c);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_FRAME_CONTROL,		0xc0);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_WORD_SIZES,		0xef);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_CLOCK_CONTROL,		0x03);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_SERIAL_MUX_CONTROL,		0xe4);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_TAG_A,			0x10);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_TAG_B,			0x32);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_TAG_C,			0x54);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_TAG_D,			0x76);
		for(i=0;i<256;i++){
			write16(BASE1, GC314FS_GCoffset[k]+GC4016_RESAMP_COEFFS+2*i,	0x0000);
			//*((uint16*)(BASE1+GC314FS_GCoffset+GC4016_RESAMP_COEFFS+2*i))	= 0;
		}
		write16(BASE1, GC314FS_GCoffset[k]+GC4016_RESAMP_COEFFS,			1024);
		// configure individual channels on GC4016-1
		for(j=0;j<4;j++){
			write16(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_PHASE,			0x0000);
			write16(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_FREQ_LSB,		0x0000);
			write16(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_FREQ_MSB,		0x4000);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_CH_RESET,		0x0c);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_FREQ_SYNC,		0x77);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_NCO_SYNC,		0x72);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_ZERO_PAD_MODE_CTRL,	0x00);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_DEC_FLUSH_SYNC,		0x22);
			write16(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_DEC_RATIO,		0x703f);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_CIC_SCALE,		0x64);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_SPLIT_IQ,		0x00);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_CFIR,			0x00);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_PFIR,			0x00);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_INPUT,			0x00);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_PEAK_CTRL,		0x00);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_PEAK_COUNT,		0x00);
			write16(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_FINE_GAIN,		0x0300);
			for(i=0;i<11;i++){
				write16(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_CFIR_COEFFS+2*i,	cfir_coeffs[i]);
				//*((uint16*)(BASE1+GC314FS_GCoffset+chan_control_map+GC4016_CFIR_COEFFS+2*i))	= cfir_coeffs[i];
			}
			for(i=0;i<32;i++){
				write16(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_PFIR_COEFFS+2*i,	pfir_coeffs[i]);
				//*((uint16*)(BASE1+GC314FS_GCoffset+chan_control_map+GC4016_PFIR_COEFFS+2*i))	= pfir_coeffs[i];
			}
		}
	}
    /**********************************************************************************/

	// activate SYNC 1
	write32(BASE1, GC314FS_GSM,					0x00008000);
	write32(BASE1, GC314FS_GCSR,					0x01200814);
	// set GC4016s to 16 bit mode
	write32(BASE1, GC314FS_GCSR,					0x01200814);
	// release the gc4016 resets
	write08(BASE1, GC314FS_GC1offset+GC4016_GLOBAL_RESET,		0x08);
	write08(BASE1, GC314FS_GC2offset+GC4016_GLOBAL_RESET,		0x08);
	write08(BASE1, GC314FS_GC3offset+GC4016_GLOBAL_RESET,		0x08);
	// de-activate SYNC 1
	write32(BASE1, GC314FS_GCSR,					0x01000814);
	// set RDA mode to continuous
	write32(BASE1, GC314FS_IWBCSR,					0x0002);
	write16(BASE1, GC314FS_R1ACSR,					0x0002);
	write16(BASE1, GC314FS_R1BCSR,					0x0002);
	write16(BASE1, GC314FS_R1CCSR,					0x0002);
	write16(BASE1, GC314FS_R1DCSR,					0x0002);
	write16(BASE1, GC314FS_R2ACSR,					0x0002);
	write16(BASE1, GC314FS_R2BCSR,					0x0002);
	write16(BASE1, GC314FS_R2CCSR,					0x0002);
	write16(BASE1, GC314FS_R2DCSR,					0x0002);
	write16(BASE1, GC314FS_R3ACSR,					0x0002);
	write16(BASE1, GC314FS_R3BCSR,					0x0002);
	write16(BASE1, GC314FS_R3CCSR,					0x0002);
	write16(BASE1, GC314FS_R3DCSR,					0x0002);
	// disable external trigger
	write32(BASE1, GC314FS_GCSR,					0x01000814);
	// disable IWB bypass
	write32(BASE1, GC314FS_IWBCSR,					0x10043);
	// set IWB source as A/D 1 input 
	write32(BASE1, GC314FS_IWBCSR,					0x1004b);
	// select GC4016s to be driven with data straight from A/Ds
	write32(BASE1, GC314FS_IWBCSR,					0x1004b);
	// set FIFO levels
	write32(BASE1, GC314FS_IWBLVL,					8192);
	write32(BASE1, GC314FS_R1ALVL,					512);
	write32(BASE1, GC314FS_R1BLVL,					512);
	write32(BASE1, GC314FS_R1CLVL,					512);
	write32(BASE1, GC314FS_R1DLVL,					512);
	write32(BASE1, GC314FS_R2ALVL,					512);
	write32(BASE1, GC314FS_R2BLVL,					512);
	write32(BASE1, GC314FS_R2CLVL,					512);
	write32(BASE1, GC314FS_R2DLVL,					512);
	write32(BASE1, GC314FS_R3ALVL,					512);
	write32(BASE1, GC314FS_R3BLVL,					512);
	write32(BASE1, GC314FS_R3CLVL,					512);
	write32(BASE1, GC314FS_R3DLVL,					512);
	// set number of samples to be collected during burst collection
	write32(BASE1, GC314FS_IWBDWS,					8192);
	write32(BASE1, GC314FS_R1ADWS,					samples_to_collect);
	write32(BASE1, GC314FS_R1BDWS,					samples_to_collect);
	write32(BASE1, GC314FS_R1CDWS,					samples_to_collect);
	write32(BASE1, GC314FS_R1DDWS,					samples_to_collect);
	write32(BASE1, GC314FS_R2ADWS,					samples_to_collect);
	write32(BASE1, GC314FS_R2BDWS,					samples_to_collect);
	write32(BASE1, GC314FS_R2CDWS,					samples_to_collect);
	write32(BASE1, GC314FS_R2DDWS,					samples_to_collect);
	write32(BASE1, GC314FS_R3ADWS,					samples_to_collect);
	write32(BASE1, GC314FS_R3BDWS,					samples_to_collect);
	write32(BASE1, GC314FS_R3CDWS,					samples_to_collect);
	write32(BASE1, GC314FS_R3DDWS,					samples_to_collect);
	// set number of samples to skip before data collection
	write32(BASE1, GC314FS_IWBSKIP,					0);
	write32(BASE1, GC314FS_R1ASKIP,					0);
	write32(BASE1, GC314FS_R1BSKIP,					0);
	write32(BASE1, GC314FS_R1CSKIP,					0);
	write32(BASE1, GC314FS_R1DSKIP,					0);
	write32(BASE1, GC314FS_R2ASKIP,					0);
	write32(BASE1, GC314FS_R2BSKIP,					0);
	write32(BASE1, GC314FS_R2CSKIP,					0);
	write32(BASE1, GC314FS_R2DSKIP,					0);
	write32(BASE1, GC314FS_R3ASKIP,					0);
	write32(BASE1, GC314FS_R3BSKIP,					0);
	write32(BASE1, GC314FS_R3CSKIP,					0);
	write32(BASE1, GC314FS_R3DSKIP,					0);

	write32(BASE1, GC314FS_R1AFS,					0);

	return 1;

}
