#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "_prog_conventions.h"
#include "_regs_GC314FS.h"
#include "_regs_GC4016.h"
#include "_global.h"

int _config_GC4016(char *BASE1, int print, unsigned long *GC4016, int *resampcoeffs, unsigned int *resampratio){

	int		i=0,j,k, skip=26;
	int		test[3]={3,2,6};
	unsigned int	GC314FS_GCoffset[3]={0x800,0x1000,0x1800};
	unsigned int 	chan_control_map[4]={0x00,0x80,0x100,0x180};
	unsigned int	*coeffs;
	struct		timespec start, stop;

	unsigned int	*to_write;	
    /* PROGRAM GC4016-1 *****************************************************************/
	clock_gettime(CLOCK_REALTIME, &start);
	for(k=0;k<3;k++){
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_GENERAL_SYNC,			0x00);
		//write08(BASE1, GC314FS_GCoffset[k]+GC4016_GLOBAL_RESET,			0xf8);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_N_CHANNELS_OUT,		0x2f);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_MISCELLANEOUS_CONTROL,	0x02);
		//write08(BASE1, GC314FS_GCoffset[k]+GC4016_N_MULTIPLES,			0x07);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_N_MULTIPLES,			0x07);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_FILTER_SELECT,		0xe4);
		//write08(BASE1, GC314FS_GCoffset[k]+GC4016_FINAL_SHIFT,			0x14);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_FINAL_SHIFT,			0x17);
		//write08(BASE1, GC314FS_GCoffset[k]+GC4016_FINAL_SHIFT,			0x1a); // ADDED BY TODD FOR INCREASED GAIN
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_CHANNEL_MAP,			0xe4);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_ADD_TO,			0x70);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_RESAMPLER_CLOCK_DIVIDE,	0x00);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_RATIO_MAP,			0xe4);
		//write32(BASE1, GC314FS_GCoffset[k]+GC4016_RATIO_0,			0x05000000);
		//write32(BASE1, GC314FS_GCoffset[k]+GC4016_RATIO_1,			0x04000000);
		//write32(BASE1, GC314FS_GCoffset[k]+GC4016_RATIO_2,			0x04000000);
		//write32(BASE1, GC314FS_GCoffset[k]+GC4016_RATIO_3,			0x04000000);
		//printf("RESAMPRATIO[0]=%x\n", resampratio[0]);
		write32(BASE1, GC314FS_GCoffset[k]+GC4016_RATIO_0,			resampratio[0]);
		write32(BASE1, GC314FS_GCoffset[k]+GC4016_RATIO_1,			resampratio[1]);
		write32(BASE1, GC314FS_GCoffset[k]+GC4016_RATIO_2,			resampratio[2]);
		write32(BASE1, GC314FS_GCoffset[k]+GC4016_RATIO_3,			resampratio[3]);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_GENERAL_SYNC,			0x00);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_COUNT_SYNC,			0x00);
		write16(BASE1, GC314FS_GCoffset[k]+GC4016_COUNTER,			0xffff);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_TRISTATE_CONTROLS,		0xff);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_FORMAT,		0x4e);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_MODE,			0x6c);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_FRAME_CONTROL,		0xc0);
		//write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_WORD_SIZES,		0xef);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_WORD_SIZES,		0xe8);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_CLOCK_CONTROL,		0x03);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_SERIAL_MUX_CONTROL,		0xe4);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_TAG_A,			0x10);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_TAG_B,			0x32);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_TAG_C,			0x54);
		write08(BASE1, GC314FS_GCoffset[k]+GC4016_OUTPUT_TAG_D,			0x76);
		
		//*((uint32*)(BASE1+GC314FS_GCoffset[k]+GC4016_RESAMP_COEFFS))		=1024;
		for(i=0;i<128;i++){
			*((uint32*)(BASE1+GC314FS_GCoffset[k]+GC4016_RESAMP_COEFFS+4*i))=resampcoeffs[i];
		}
		// configure individual channels on GC4016-1
/*		for(j=0;j<4;j++){
			write16(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_PHASE,			0x0000);
			write16(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_FREQ_LSB,		0x0000);
			write16(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_FREQ_MSB,		0x4000);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_CH_RESET,		0x0c);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_FREQ_SYNC,	0x77);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_NCO_SYNC,		0x72);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_ZERO_PAD_MODE_CTRL,	0x00);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_DEC_FLUSH_SYNC,	0x22);
			write16(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_DEC_RATIO,	0x703f);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_CIC_SCALE,	0x64);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_SPLIT_IQ,		0x00);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_CFIR,		0x00);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_PFIR,		0x00);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_INPUT,		0x00);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_PEAK_CTRL,	0x00);
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_PEAK_COUNT,	0x00);
			write16(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_FINE_GAIN,	0x0300);

			coeffs=GC_vals->cfir_coeffs[0];
			for(i=0;i<6;i++){
				write32(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_CFIR_COEFFS+4*i,	coeffs[i]);
			}
			coeffs=GC_vals->pfir_coeffs[0];
			for(i=0;i<16;i++){
				write32(BASE1, GC314FS_GCoffset[k]+chan_control_map[j]+GC4016_PFIR_COEFFS+4*i,	coeffs[i]);
			}
		}
*/
		// write the control registers of the GC4016 individual channels
		for (i=0;i<4;i++){
			to_write=GC4016[i];
			for (j=0; j<32; j++){
				*((uint32*)(BASE1+GC314FS_GCoffset[k]+chan_control_map[i]+4*j))=to_write[j];
			}	
			write08(BASE1, GC314FS_GCoffset[k]+chan_control_map[i]+GC4016_INPUT, k|0x10);
		}
	}
    /**********************************************************************************/

	return 1;

}
