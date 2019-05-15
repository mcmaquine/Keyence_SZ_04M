#include <mbed.h>
#include "SZ_04M.h"

/*
InterruptIn in(PA_10);
//DigitalIn in(PA_8);
DigitalOut LED(LED1);
DigitalOut out(PC_4);*/
InterruptIn B1(BUTTON1);
InterruptIn State(PA_9);
DigitalOut BUZZ(PC_7);

void init(); //initialize nucleo
void flip(); //change LED state
void changeBank();


/***************************************************************
**********  TESTADO NA NUCLEO F401  ***************************/
SZ_04M sensor(PA_3, PA_2, PA_10, PB_3, PB_5, PB_4, PB_10, PA_8,
PB_13, PB_14, PB_15, PB_1, PB_2);
/***************************************************************
***************************************************************/

Serial pc(USBTX, USBRX);

int main() {
  sensor.init();
  sensor.selectBank( 2 );
  init(); //setup code here, to run once.
  pc.printf( "STARTING\n" );
  while(1) {
    BUZZ = 1;
    pc.printf("%x Status: %d IsDetecting %d\n", sensor.readInputs(), sensor.getLastStatus(), sensor.isDetecting() );
    wait(.5);
    BUZZ=0;
    while( !sensor.checkInternalError() ){} //wait until problem resolved

    if( sensor.isReadyToUnlock() ) sensor.releaseInterlock();
  }
}

void init(){
  B1.mode(PullUp);
  B1.rise(&flip);
  State.mode( PullUp );
  State.rise( &changeBank );
  BUZZ = 0;
}

void flip(){
  sensor.releaseInterlock();
  pc.printf("Interlock function called\n");
}

void changeBank(){
  sensor.nextBank();
}