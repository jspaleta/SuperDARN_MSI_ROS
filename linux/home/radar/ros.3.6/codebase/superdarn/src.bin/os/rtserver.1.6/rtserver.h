#ifndef _RTSERVER_H
#define _RTSERVER_H
int operate(int sock, int port,int run);
int initialize(int run,int argc, char *argv[]);
int main(int argc, char *argv[]);
struct RadarParm* getRadarParm();
struct FitData* getFitData();
struct FitRange* return_rng_xrng(int index,int xflag);
struct FitNoise* return_noise();
struct FitElv* return_elv(int index);
int16 return_lag(int index1,int index2);
int16 return_pulse(int index);
int get_outpipe();


#endif
