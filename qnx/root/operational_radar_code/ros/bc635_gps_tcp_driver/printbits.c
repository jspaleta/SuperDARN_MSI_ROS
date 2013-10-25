#include <stdio.h>

/* Print n as a binary number */
void printbits(void *stream, int n) {
        unsigned int i;
        i = 1<<(sizeof(n) * 8 - 1);

        while (i > 0) {
                if (n & i)
                        fprintf(stream,"1");
                else
                        fprintf(stream,"0");
                i >>= 1;
        }
}

