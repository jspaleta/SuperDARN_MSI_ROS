#include "bc635_registers.h"

void ack_dpram_data(struct DEV_reg *DEVreg)
{
        DEVreg->ack=DATUM_ACK_BIT_REC;
        DEVreg->ack=DATUM_ACK_BIT_CMD;
        while( !( DEVreg->ack & DATUM_ACK_BIT_REC ) ) delay( 10 );
}

void write_dpram_data(struct DEV_reg *DEVreg, char *addr, char *data, int len )
{
        for( ; len > 0; len-- )
                *(addr++) = *(data++);
        ack_dpram_data(DEVreg); 
}
