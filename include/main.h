
#include <mbed.h>
#include "SZ_04M.h"


//InterruptIn in(PA_10);
//DigitalIn in(PA_8);
DigitalOut LED(LED2);
//DigitalOut out(PC_4);
//DigitalOut BUZZ(PC_7);


/***************************************************************
**********  TESTADO NA NUCLEO F767  ***************************/
SZ_04M sensor( PG_9, PG_14, PF_15, PE_13, PF_14, PE_11,
PE_9, PF_13, PA_15, PB_12, PB_13, PB_15,PC_6);
/***************************************************************
***************************************************************/

Serial pc(USBTX, USBRX);
#include"serialCommand.h"