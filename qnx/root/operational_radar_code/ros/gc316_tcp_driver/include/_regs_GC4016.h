#ifndef _regs_GC4016

#define _regs_GC4016

/* DEFINE OFFSETS TO CHANNEL CONTROL REGISTERS */
#define chanA_control_map		0x00
#define chanB_control_map		0x80
#define chanC_control_map		0x100
#define chanD_control_map		0x180

/* DEFINE CHANNEL CONTROL REGISTERS */
#define GC4016_CFIR_COEFFS		0x00
#define GC4016_PFIR_COEFFS		0x20
#define GC4016_PHASE			0x60
#define GC4016_FREQ_LSB			0x62
#define GC4016_FREQ_MSB			0x64
#define GC4016_CH_RESET			0x70
#define GC4016_FREQ_SYNC		0x71
#define GC4016_NCO_SYNC			0x72
#define GC4016_ZERO_PAD_MODE_CTRL	0x73
#define GC4016_DEC_FLUSH_SYNC		0x74
#define GC4016_DEC_RATIO		0x75
#define GC4016_CIC_SCALE		0x77
#define GC4016_SPLIT_IQ			0x78
#define GC4016_CFIR			0x79
#define GC4016_PFIR			0x7a
#define GC4016_INPUT			0x7b
#define GC4016_PEAK_CTRL		0x7c
#define GC4016_PEAK_COUNT		0x7d
#define GC4016_FINE_GAIN		0x7e

/* DEFINE RESAMPLER CONTROL REGISTERS */
#define GC4016_RESAMP_COEFFS		0x200
#define GC4016_N_CHANNELS_OUT		0x400
#define GC4016_N_MULTIPLES		0x401
#define GC4016_FILTER_SELECT		0x402
#define GC4016_FINAL_SHIFT		0x403
#define GC4016_CHANNEL_MAP		0x404
#define GC4016_ADD_TO			0x405
#define GC4016_RESAMPLER_CLOCK_DIVIDE	0x406
#define GC4016_RATIO_MAP		0x407
#define GC4016_RATIO_0			0x410
#define GC4016_RATIO_1			0x414
#define GC4016_RATIO_2			0x418
#define GC4016_RATIO_3			0x41c
#define GC4016_CHANNEL_OUTPUT		0x420
#define GC4016_GLOBAL_RESET		0x440
#define GC4016_STATUS			0x441
#define GC4016_PAGE			0x442
#define GC4016_CHECKSUM			0x443
#define GC4016_GENERAL_SYNC		0x444
#define GC4016_COUNT_SYNC		0x445
#define GC4016_COUNTER			0x446
#define GC4016_TRISTATE_CONTROLS	0x450
#define GC4016_OUTPUT_FORMAT		0x451
#define GC4016_OUTPUT_MODE		0x452
#define GC4016_OUTPUT_FRAME_CONTROL	0x453
#define GC4016_OUTPUT_WORD_SIZES	0x454
#define GC4016_OUTPUT_CLOCK_CONTROL	0x455
#define GC4016_SERIAL_MUX_CONTROL	0x456
#define GC4016_OUTPUT_TAG_A		0x457
#define GC4016_OUTPUT_TAG_B		0x458
#define GC4016_OUTPUT_TAG_C		0x459
#define GC4016_OUTPUT_TAG_D		0x45a
#define GC4016_MASK_REVISION		0x45b
#define GC4016_MISCELLANEOUS_CONTROL	0x45c


struct GC4016_reg_vals{
	short	*resamp_coeffs;
	char	n_channels_out;
	char	n_multiples;
	char	filter_select;
	char	final_shift;
	char	channel_map;
	char	add_to;
	char	resampler_clock_divide;
	char	ratio_map;
	int	ratio_0;
	int	ratio_1;
	int	ratio_2;
	int	ratio_3;
	int	channel_output;
	char	global_reset;
	char	status;
	char	page;
	char	checksum;
	char	general_sync;
	char	count_sync;
	short	counter;
	char	tristate_controls;
	char	output_format;
	char	output_mode;
	char	output_frame_control;
	char	output_word_sizes;
	char	output_clock_control;
	char	serial_mux_control;
	char	output_tag_A;
	char	output_tag_B;
	char	output_tag_C;
	char	output_tag_D;
	char	mask_revision;
	char	miscellaneous_control;

	short	*cfir_coeffs[4];
	short	*pfir_coeffs[4];
	short	phase[4];
	short	freq_lsb[4];
	short	freq_msb[4];
	char	ch_reset[4];
	char	freq_sync[4];
	char	nco_sync[4];
	char	zero_pad_mode_ctrl[4];
	char	dec_flush_sync[4];
	short	dec_ratio[4];
	char	cic_scale[4];
	char	split_IQ[4];
	char	cfir[4];
	char	pfir[4];
	char	input[4];
	char	peak_ctrl[4];
	char	peak_count[4];
	short	fine_gain[4];
};

struct GC4016_channel_regs{

	short	cfir_coeffs[11];		//={0,0,0,0,0,0,0,0,0,0};
	short	FILL[5];			//={0,0,0,0};
	short	pfir_coeffs[32];		//={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	short	phase;
	short	freq_lsb;
	short	freq_msb;
	short	FILL2[5];			//={0,0,0,0};
	char	ch_reset;
	char	freq_sync;
	char	nco_sync;
	char	zero_pad_mode_ctrl;
	char	dec_flush_sync;
	char	dec_ratio_lsb;
	char	dec_ratio_msb;
	char	cic_scale;
	char	split_IQ;
	char	cfir;
	char	pfir;
	char	input;
	char	peak_ctrl;
	char	peak_count;
	short	fine_gain;
};
	
#endif
