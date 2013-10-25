#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "_prog_conventions.h"
#include "_regs_GC314FS.h"
#include "_regs_GC4016.h"
#include "_global.h"


short	cfir_coeffs[12]={	12,	-93,	-62,	804,	1283,	-1273,
					-5197,	-3512,	8332,	24823,	32767,	0};	
short	pfir_coeffs[32]={	132,	182,	1,	-309,	-330,	119,
					560,	343,	-424,	-832,	-278,	696,
					960,	149,	-916,	-1132,	-212,	1075,
					1514,	421,	-1518,	-2374,	-673,	2529,
					3817,	726,	-4796,	-6888,	-650,	12967,
					26898,	32767};
short	resamp_coeffs[256];

float	blackman_cfir[11]={	0,	0.0092,	0.0402,	0.1014, 0.2008,	0.3400, 
					0.5098,	0.6892,	0.8492,	0.9602, 1.0};
float	blackman_pfir[32]={	0, 	0.0009, 0.0038, 0.0086, 0.0156, 0.0251,
					0.0374, 0.0527, 0.0715, 0.0939, 0.1203,
					0.1508, 0.1856, 0.2247, 0.2680, 0.3151,
					0.3657, 0.4194, 0.4754, 0.5330, 0.5912,
					0.6493, 0.7060, 0.7604, 0.8115, 0.8582,
					0.8995, 0.9346, 0.9627, 0.9883, 0.9958,
					1};
				

int _set_GC4016_vals(int BW, int fout, int freq, struct GC4016_reg_vals *GC_vals, struct GC4016_channel_regs *GC4016){

	int 		i, n;
	int 		FREQ;


	float		cfir_pass=0.1, cfir_stop=0.3, pfir_pass=0.12, pfir_stop=0.3;
	int		decimation, big_scale, shift=4, scale=0, coarse;
	float		cic_gain, cic_gain_temp, wc, cfir_gain, pfir_gain, gain, fine_gain, pi=3.14159;

	// parameters for each channel to get appropriate performance
	// calculate decimation
	decimation=(int)( cfir_pass*(int)FCLOCK/(float)BW + 0.5);
	if (decimation < 8) decimation = 8;
	if (decimation > 4096) decimation = 4096;
	// calculate BIG_SCALE
	big_scale=(int)((62-5*log10(decimation)/log10(2)-shift-scale)/6);
	if (big_scale>7) big_scale=7;
	if (big_scale<0) big_scale=0;
	// caclulate CIC_SCALE
	cic_gain=pow((float)decimation, 5)*pow(2, shift+scale+6*big_scale-62);
	cic_gain_temp=1/cic_gain;
	// calculate coarse gain
	coarse=(int)(log10(cic_gain_temp)/log10(2));
	if (coarse<0) coarse=0;
	if (coarse>7) coarse=7;
	// calculate the cfir_coeffs
	cfir_pass= (float)decimation*(float)BW/(float)FCLOCK;
	wc=pi*((float)cfir_pass-(float)cfir_stop)/2;
	cfir_gain=0;
	for (n=0;n<11;n++){
		cfir_coeffs[n]=(int)(32767*blackman_cfir[n]*(pi/wc)*sin(wc*(n-10))/(pi*(n-10))+0.5);
		//printf("cfir coeff %d = %d\n", n, cfir_coeffs[n]);
		cfir_gain+=cfir_coeffs[n];
	}
	cfir_coeffs[10]=32767;
	cfir_gain/=65536;
	// calculate the cfir_coeffs
	pfir_pass= (float)decimation*(float)BW/(float)FCLOCK/2;
	wc=pi*((float)pfir_pass-(float)pfir_stop)/2;
	pfir_gain=0;
	for (n=0;n<32;n++){
		pfir_coeffs[n]=(int)(32767*blackman_pfir[n]*(pi/wc)*sin(wc*(n-31))/(pi*(n-31))+0.5);
		//printf("pfir coeff %d = %d\n", n, pfir_coeffs[n]);
		pfir_gain+=pfir_coeffs[n];
	}
	pfir_coeffs[31]=32767;
	pfir_gain/=65536;
	gain=cfir_gain*pfir_gain*cic_gain*pow(2,coarse);
	fine_gain=(int)(1024/gain);	
	if (fine_gain>16384) fine_gain=16384;
	if (fine_gain<1) fine_gain=1;

	printf("decimation= %d cfir_pass= %f pfir_pass= %f  big_scale=%d coarse=%d\n", decimation, cfir_pass, pfir_pass, big_scale, coarse);

	FREQ=4294967296*freq/FCLOCK;	

	// set the resampler coeffs to an impulse response for resampler bypass
	for (i=0;i<256;i++){
		resamp_coeffs[i]=0;
	}
	resamp_coeffs[0]=1024;

	// pass back the addresses of the coeff arrays for addressing by other parts of the driver
	GC_vals->cfir_coeffs[0]=cfir_coeffs;
	GC_vals->pfir_coeffs[0]=pfir_coeffs;
	GC_vals->resamp_coeffs=resamp_coeffs;

// *******************************************************************************************************
	for (i=0;i<11;i++){
		GC4016->cfir_coeffs[i]=cfir_coeffs[i];
	}
	for (i=0;i<5;i++){
		GC4016->FILL[i]=-1;
	}
	for (i=0;i<32;i++){
		GC4016->pfir_coeffs[i]=pfir_coeffs[i];
	}
	GC4016->phase		=0x0000;
	GC4016->freq_lsb	=(unsigned short)(FREQ & 0x0000ffff);
	GC4016->freq_msb	=(unsigned short)( (FREQ >>16) & 0x0000ffff);	
	for (i=0;i<5;i++){
		GC4016->FILL2[i]=-1;
	}
	GC4016->ch_reset	=0x0c;	
	GC4016->freq_sync	=0x77;	
	GC4016->nco_sync	=0x72;	
	GC4016->zero_pad_mode_ctrl	=0x00;	
	GC4016->dec_flush_sync	=0x22;	
	GC4016->dec_ratio_lsb	=(char)( (decimation-1) & 0xff );	
	GC4016->dec_ratio_msb	=0x70 + (char)( ((decimation-1)>>8) & 0xf);	
	GC4016->cic_scale	=((0x1<<6)+(big_scale<<3)+scale) & 0x7f;	
	GC4016->split_IQ	=0x00;	
	GC4016->cfir		=0x00 | (coarse<<4);	
	GC4016->pfir		=0x00;	
	GC4016->input		=0x00;	
	GC4016->peak_ctrl	=0x00;	
	GC4016->peak_count	=0x00;	
	GC4016->fine_gain	=fine_gain;	
	return 1;
}
