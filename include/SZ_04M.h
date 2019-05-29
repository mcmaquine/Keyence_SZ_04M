#include <mbed.h>

/* 
    Ver0.1 Only for "NOT USED" Operation Mode
    Class driver for Keyence SZ_04M.
    Implementation based on SZ Series Ver.2 User's Manual
    Please see manual page 4.6 for correct pins instantiation and wiring
*/

#ifndef _SZ_04M_H_
    #define _SZ_04M_H

#define SZON    0
#define SZOFF   1

class SZ_04M{
    public:
        //First 8 Pins are INPUTS, the five following pins are OUTPUTS
        //The order is the same as shown on page 4.6
        SZ_04M(PinName _ossd1, PinName _ossd2, PinName _aux1, PinName _aux2,
        PinName _aux3, PinName _aux4, PinName _aux5, PinName _aux6,
        PinName _reinit, PinName _bank_1, PinName _bank_2,
        PinName _bank_3, PinName _bank_4 );

        ~SZ_04M();

        void init();
        //Set all outputs LSB: entry 1      MSB: Entry 6
        void setOutputs( unsigned char val );
        unsigned char readInputs();
        void releaseInterlock();                //set output to release interlockgit c
        void nextBank();
        void selectBank(unsigned char bank);    //Between 1 ~ 4;
        unsigned char getLastStatus();          //return last sensor status
        bool isDetecting();                     //TRUE if there is something on protection zone, FALSE otherwise
        bool isReadyToUnlock();                 //TRUE if is ready, FALSE otherwise
        bool checkInternalError();              //check internal errors and try solution, return TRUE if it achieve a solution, FALSE otherwise
        unsigned char activatedBank();          //check which bank is active 0 if none
        bool isErrorOn();                       //check if SZ_04M is on error
        bool isOperational();

    private:

        Timeout toff;
        Timeout interval;

        DigitalIn ossd1;
        DigitalIn ossd2;
        InterruptIn aux1;
        InterruptIn aux2;
        DigitalIn aux3;
        DigitalIn aux4;
        DigitalIn aux5;
        DigitalIn aux6;

        DigitalOut  reinit;
        DigitalOut  bank_1;
        DigitalOut  bank_2;
        DigitalOut  bank_3;
        DigitalOut  bank_4;

        unsigned char lastStatus; //last status read by AUX 2 (page 2.41)
        unsigned char incomingStatus;
        bool onInfo; //shows if AUX 1 is ON (false is ON true is OFF)
        bool detecting;
        bool releasing; //true if releaseInterlock proces started

        void startOnAux1();
        void endOnAux1();

        void countEdgeOnAux2();

        void timeout_isr_();
        void interval_isr(); 

        //error checking methods
        unsigned char totalActiveBanks();
};
#endif

/*                  *
 * Impelmentation   *
*/                  
SZ_04M::SZ_04M (PinName _ossd1, PinName _ossd2, PinName _aux1, PinName _aux2,
        PinName _aux3, PinName _aux4, PinName _aux5, PinName _aux6,
        PinName _reinit, PinName _bank_1, PinName _bank_2, PinName _bank_3,
        PinName _bank_4) : ossd1( _ossd1 ), ossd2( _ossd2 ), aux1( _aux1 ),
        aux2( _aux2 ), aux3( _aux3 ), aux4( _aux4 ), aux5( _aux5 ),
        aux6( _aux6 ), reinit( _reinit ), bank_1( _bank_1 ), bank_2( _bank_2 ),
        bank_3( _bank_3 ), bank_4( _bank_4 ){}

SZ_04M::~SZ_04M(){}

void SZ_04M::init(){
    reinit = SZOFF;
    bank_1 = SZON;
    bank_2 = SZOFF;
    bank_3 = SZOFF;
    bank_4 = SZOFF;

    ossd1.mode( PullUp );
    ossd2.mode( PullUp );
    aux1.mode( PullUp );
    aux2.mode( PullUp );
    aux3.mode( PullUp );
    aux4.mode( PullUp );
    aux5.mode( PullUp );
    aux6.mode( PullUp );

    aux1.rise( callback( this, &SZ_04M::endOnAux1 ) );
    aux1.fall( callback( this, &SZ_04M::startOnAux1 ) );

    aux2.fall( callback( this, &SZ_04M::countEdgeOnAux2 ) );

    releasing = false;
    reinit = SZOFF;
};

void SZ_04M::setOutputs( unsigned char val ){
    //Following order on manual Serie SZ ver2 page 4.6
    //MSB: Entry 6 EDM OSSD 3/4      LSB: Entry 1 reinit
    //Byte structure
    //| NC | NC | NC | banco_4 | banco_3 | banco_2 | banco_1 | reinit |

    reinit =   0x01 & val;// >> 0
    bank_1     = ( 0x02 & val ) >> 1;
    bank_2     = ( 0x04 & val ) >> 2;
    bank_3     = ( 0x08 & val ) >> 3;
    bank_4     = ( 0x10 & val ) >> 4;
};

unsigned char SZ_04M::readInputs(){
    //Following same page as depicted on setOutputs()
    //LSB: OSSD1        MSB: Exit AUX 6 Total 8 bits
    // Byte structure
    //| AUX6 | AUX5 | AUX4 | AUX3 | AUX2 | AUX1 | OSSD1 | OSSD2 |
    unsigned char val = 0;

    //    add      and     shift
    val += !( aux6 ); val = val << 1;
    val += !( aux5 ); val = val << 1;
    val += !( aux4 ); val = val << 1;
    val += !( aux3 ); val = val << 1;
    val += !( aux2.read() ); val = val << 1;
    val += !( aux1.read() ); val = val << 1;

    val += !( ossd2 ); val = val << 1;
    val += !( ossd1 );

    return val; 
}

void SZ_04M::releaseInterlock(){
    if( !releasing )
    {
        reinit = SZON;
        toff.attach( callback(this, &SZ_04M::timeout_isr_ ), .100);
        releasing = true;
    }    
};

void SZ_04M::nextBank(){
    unsigned char val = 
        bank_4 << 3 | bank_3 << 2 | bank_2 << 1 | bank_1;

    switch ( val )
    {
        case 0x01:
            bank_1 = SZOFF;
            bank_2 = SZON;
            break;
        
        case 0x02:
            bank_2 = SZOFF;
            bank_3 = SZON;
            break;
        
        case 0x04:
            bank_3 = SZOFF;
            bank_4 = SZOFF;
            break;
        
        case 0x08:   //backs to 1
            bank_4 = SZOFF;
            bank_1 = SZON;
            break;

        default:
            bank_1 = SZON;
            bank_2 = SZOFF;
            bank_3 = SZOFF;
            bank_4 = SZOFF;
            break;
    }
};

void SZ_04M::selectBank( unsigned char bank ){
    switch (bank)
    {
        case 1:
            bank_1 = SZON;
            bank_2 = SZOFF;
            bank_3 = SZOFF;
            bank_4 = SZOFF;
            break;
        
        case 2:
            bank_1 = SZOFF;
            bank_2 = SZON;
            bank_3 = SZOFF;
            bank_4 = SZOFF;
            break;
        
        case 3:
            bank_1 = SZOFF;
            bank_2 = SZOFF;
            bank_3 = SZON;
            bank_4 = SZOFF;
            break;

        case 4:
            bank_1 = SZOFF;
            bank_2 = SZOFF;
            bank_3 = SZOFF;
            bank_4 = SZON;
            break;
    
        default:
            bank_1 = SZON;
            bank_2 = SZOFF;
            bank_3 = SZOFF;
            bank_4 = SZOFF;
            break;
    }
};

void SZ_04M::startOnAux1(){
    incomingStatus = 0;
    onInfo = true;
};

void SZ_04M::endOnAux1(){
    onInfo = false;
    lastStatus = incomingStatus;
};

void SZ_04M::countEdgeOnAux2(){
    incomingStatus++;
};

unsigned char SZ_04M::getLastStatus(){
    return lastStatus;
};

void SZ_04M::timeout_isr_(){
    reinit = SZOFF;
    interval.attach( callback( this, &SZ_04M::interval_isr), .500);
    toff.detach();
};

void SZ_04M::interval_isr(){
    releasing = false;
    interval.detach();
};

bool SZ_04M::isDetecting(){
    return ( !aux5.read() == SZON ) ? true : false;
};

bool SZ_04M::isReadyToUnlock(){
    return ( aux6.read() == SZON ) ? true : false;
};

bool SZ_04M::checkInternalError(){
    bool val = false;
    //for now only error number 15

    switch ( lastStatus )
    {
        case 15:
            if ( totalActiveBanks() > 1 ){
                selectBank( 2 );
                releaseInterlock();
            }
            selectBank( 2 );
            val = true;
            break;

        case 16: //Physical problem
            val = false;
            break;
    
        default: //1, 4, 5, 6, 7 is OK
        //do nothing
            val = true;
            break;
    }

    if( aux3  ) releaseInterlock();

    return val;
}

unsigned char SZ_04M::totalActiveBanks(){
    unsigned char total = 0;

    if( bank_1 == SZON ) total++;
    if( bank_2 == SZON ) total++;
    if( bank_3 == SZON ) total++;
    if( bank_4 == SZON ) total++;

    return total;
}

unsigned char SZ_04M::activatedBank(){
    if( bank_1 == SZON) return 1;
    else if( bank_2 == SZON ) return 2;
    else if( bank_3 == SZON ) return 3;
    else if( bank_4 == SZON ) return 4;
    else return 0;
}

bool SZ_04M::isErrorOn(){
    return ( aux3.read() == SZOFF ) ? true : false;
}

bool SZ_04M::isOperational(){
    return ( ossd1.read() == SZON ) ? true : false;
}