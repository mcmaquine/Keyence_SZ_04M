#ifndef SERIALLINECOMMAND_H
#define SERIALLINECOMMAND_H
#include "mbed.h"
#define MAX_LEN 15
#define MAX_COMMAND 20

class SerialLineCommand{
    private:
    bool isParam;
    bool auxEnter;
    char comandBuff[MAX_LEN];
    char paramBuff[MAX_LEN];
    char commands[MAX_COMMAND][MAX_LEN];
    Callback<void()> functions[MAX_COMMAND];
    int numCommands;
    uint8_t BufPos;
    uint8_t ParPos;
    Serial *serial;
    bool hasCommand(){
        while (serial->readable()){
            char c = serial->getc();
            if (c == 8){ //Backspace
                if (isParam){
                    if (ParPos > 0){
                        serial->printf("%c%c%c", 8, 32, 8);
                        ParPos--;
                        paramBuff[ParPos] = '\0';
                    }
                    else if (ParPos == 0) {
                        isParam = false;
                        serial->printf("%c", 8);
                    }
                }
                else {
                    if (BufPos > 0){
                        serial->printf("%c%c%c", 8, 32, 8);
                        BufPos--;
                        comandBuff[BufPos] = '\0';
                    }
                }
                
                    
            }
            else if (c == 13) auxEnter = true; //Retorno de carro
            else if (c == 10){ //ENTER
                if (auxEnter) {
                    if (BufPos > 0) {
                        serial->printf("\n");
                        auxEnter = false;
                        return true;
                    }
                    else printLine();
                }
                auxEnter = false;
            }
            else if (c == 32){ //espaço
                if (BufPos > 0) {
                    isParam = true;
                    serial->printf("%c", c);
                }
            }
            else if (c > 32 && c < 127){ //letras, números e símbolos
                if(isParam){
                    if (ParPos < MAX_LEN-1){
                        paramBuff[ParPos] = c;
                        ParPos++;
                        paramBuff[ParPos] = '\0';
                        serial->printf("%c", c);
                    }
                }
                else{
                    if (BufPos < MAX_LEN-1){
                        comandBuff[BufPos] = c;
                        BufPos++;
                        comandBuff[BufPos] = '\0';
                        serial->printf("%c", c);
                    }
                }
                
            }
            
        }
        return false;
    }
    void execute(){
        //normal commands
        bool executed = false;
        for (int i = 0; i < numCommands; i++){
            if (!strcmp(comandBuff, commands[i])){
                functions[i]();
                executed = true;
                break;
            }
        }
        if(!executed) serial->printf("Command \"%s\" not found", comandBuff);
        for (int i = 0; i < BufPos; i++) comandBuff[i] = '\0';
        for (int i = 0; i < ParPos; i++) paramBuff[i] = '\0';
        ParPos = 0;
        BufPos = 0;
        isParam = false;
        printLine();
    }

    public:
    SerialLineCommand(Serial *s): serial(s){
        numCommands = 0;
        BufPos = 0;
        ParPos = 0;
        isParam = false;
        auxEnter = false;
    }
    void addCommand(char command[], Callback<void()> f){
        strcpy(commands[numCommands], command);
        functions[numCommands] = f;
        numCommands++;
    }
    
    void run(){
        if (hasCommand()) execute();
    }
    char* getParam(){
        return paramBuff;
    }
    
    int getParamInt(){
        return atoi(paramBuff);
    }

    float getParamFloat(){
        return atof(paramBuff);
    }

    char* getCommandsSave(int i){
        if (i < numCommands) return commands[i];
    }
    int getNumCommands(){
        return numCommands;
    }
    void printLine(){
        serial->printf("\n>> ");
    }

};


#endif