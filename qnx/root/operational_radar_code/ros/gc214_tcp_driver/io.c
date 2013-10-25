
#include <stdlib.h>
#include <stdio.h>


void write8(unsigned int reg1, signed char val){
	unsigned char *reg;
	reg=(unsigned char *) reg1;
	*(reg)=(char)val;
}

void write16(unsigned int reg1, signed short val){
	signed short *reg;
	reg=(signed short *) reg1;
	*(reg)=(short)val;
}

void write32(unsigned int reg1, signed int val){
	unsigned int *reg;
	reg=(unsigned int *) reg1;
	*(reg)=(int)val;
}

char read8(unsigned int reg1){
	unsigned int *reg;
	signed char val;
	reg=(unsigned int *) reg1;
	val=(char)(*reg);
	return val;
}

signed short read16(unsigned int reg1){
	unsigned int *reg;
	signed short val;
	reg=(unsigned int *) reg1;
	val=(short)*(reg);
	return val;
}

int read32(unsigned int reg1){
	unsigned int *reg;
	signed int val;
	reg=(unsigned int *) reg1;
	val=(int)*(reg);
	return val;
}


