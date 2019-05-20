//#include "main.h"
#include "SerialLineCommand.h"

//Comandos padrão
#define INIT "init"
#define ENTERDEBUG "debug"
#define HELP "help"

//Comandos DEBUG

#define NOT_DEBUG "Not in debug mode"

bool isDEBUG = false;

SerialLineCommand line(&pc);

void SerialCall(){
  line.run();
}

void initCmd(){
    pc.printf("\n************************************************\n");
    pc.printf("*                                              *\n");
    pc.printf("*  Project: Automatic Guided Vehicle           *\n");
    pc.printf("*  Module: Keyence Laser Curtain SZ-04M        *\n");
    pc.printf("*  Version: 0.2                                *\n");
    pc.printf("*  Instituto Centro de Tecnologia em Software  *\n");
    pc.printf("************************************************\n");
}
void helpCmd(){
    pc.printf("Normal commands:\n");
    pc.printf("\t%s\n", INIT);
    pc.printf("\t%s\n", ENTERDEBUG);
    pc.printf("\t%s\n", HELP);

    pc.printf("\nDebug commands:\n");
}

void debugCmd(){ 
    char *param = line.getParam();
    if(!strcmp(param, "on")) isDEBUG = 1;
    else if (!strcmp(param, "off")) isDEBUG = 0;
    isDEBUG? pc.printf("Debug mode on") : pc.printf("Debug mode off");
}

void selectBank(){
    char *param = line.getParam();
    if(!strcmp(param, "1")){
        sensor.selectBank( 1 );
        pc.printf("BANK 1 selected");
    }
    else if (!strcmp(param, "2")){
        sensor.selectBank( 2 );
        pc.printf("BANK 2 selected");
    }
    else if (!strcmp(param, "3")){
        sensor.selectBank( 3 );
        pc.printf("BANK 3 selected");
    }
    else if (!strcmp(param, "4")){
        sensor.selectBank( 4 );
        pc.printf("BANK 4 selected");
    }
    else pc.printf("Out of Range");
}

void reset(){
    sensor.releaseInterlock();
    pc.printf("Interlock function called\n");
}

void changeBank(){
  sensor.nextBank();
}

void status(){
    pc.printf("%x Status: %d IsDetecting %d\n", sensor.readInputs(), sensor.getLastStatus(), sensor.isDetecting() );
}

void isReadyToUnlock(){
    if( sensor.isReadyToUnlock() )
        pc.printf("YES");
    else
        pc.printf("NO");    
}

void readInputs(){
    pc.printf("Inputs %d", sensor.readInputs() );
}
/****************************************
 * CHAMAR NO MAIN ESTA CLASSE
 ****************************************/
void initLineCommand(){
    line.addCommand("init", initCmd);
    line.addCommand(HELP, helpCmd);
    line.addCommand(ENTERDEBUG, debugCmd);
    line.addCommand("nbnk", changeBank);
    line.addCommand("rls", reset);
    line.addCommand("stat", status);
    line.addCommand("ulck", isReadyToUnlock);
    line.addCommand("bank", selectBank);
    line.addCommand("read", readInputs );
    initCmd();
    line.printLine();
    pc.attach(SerialCall);
}