float setCFIR(int *cfircoeffs,float freq_in,float Fpass,float Fstop);
float setPFIR(int *pfircoeffs,float freq_in,float Fpass,float Fstop,int matched); 
float setresamp(short *resampcoeffs,float freq_in,float Fpass,float Fstop,int filtnum,int bypass);
