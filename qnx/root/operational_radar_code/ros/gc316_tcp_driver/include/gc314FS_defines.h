#define MAX_CARDS 2 
#define MAX_INPUTS 3 
#define FIFO_LVL	   512
#define SAMPLE_FREQ        100000 //Khz
#define REVERSE_IQ_ORDER 0

typedef struct _copy_chuck{ 
        int length;
        int start;
        int channel;
        int buffer;
        double frequency;
        int radar;
        int beamdir;
} t_copy_chunk;

