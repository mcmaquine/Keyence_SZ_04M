
#include <mbed.h>
#include "SZ_04M.h"


//InterruptIn in(PA_10);
//DigitalIn in(PA_8);
DigitalOut LED(LED2);
//DigitalOut out(PC_4);
//DigitalOut BUZZ(PC_7);


/***************************************************************
**********  TESTADO NA NUCLEO F767  ***************************/
SZ_04M sensor( PG_11, PG_13, PG_10, PG_15, PE_5, PE_4, PE_2,
PG_3, PE_6, PG_1, PF_9, PF_8, PF_2);
/***************************************************************
***************************************************************/

Serial pc(USBTX, USBRX);
#include"serialCommand.h"