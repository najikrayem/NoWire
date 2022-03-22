#include <NoWireReciever.h>
#include <NoWireShared.h>
#include <IntervalTimer.h>

NoWireReciever::NoWireReciever():
__pin_irs(DEFAULT_RX_PIN),   //no need to call CLR_GIB here because singtlon oject is just being init
__sampleArray1_irs{0},
__sampleArray2_irs{0}
{
    pinMode(DEFAULT_RX_PIN, INPUT);
}

void NoWireReciever::attachToPin(uint8_t newPin){
    pinMode(newPin, INPUT);
    CLR_GIB;
    __pin_irs = newPin;
    RSTR_GIB;
}

uint8_t NoWireReciever::getPin(){
    CLR_GIB;
    uint8_t pin = __pin_irs;
    RSTR_GIB;
    return pin;
}

NoWireReciever &NoWireReciever::getInstance() {
  static NoWireReciever instance;
  return instance;
}

static IntervalTimer __rxSamplerTimer;
void __sampleRecieverPeriodically_helper(){

    int8_t sample = digitalReadFast(NoWireRx.__pin_irs);
    if(sample == 0) sample = -1;

    //no need to CLE_GIB to access __queueIndex_irs and __sampleArray1Full_irs
    //because they are private and only accesable through this through this isr
    if((NoWireRx.__queueIndex_irs < RX_SAMPLE_ARRAY_LENGTH) && !NoWireRx.__sampleArray1Full_irs){

        NoWireRx.__sampleArray1_irs[NoWireRx.__queueIndex_irs] = sample;
        ++NoWireRx.__queueIndex_irs;

        if(NoWireRx.__queueIndex_irs == RX_SAMPLE_ARRAY_LENGTH){
            NoWireRx.__sampleArray1Full_irs = true;
        }
    }
    else if((NoWireRx.__queueIndex_irs < RX_SAMPLE_ARRAY_LENGTH*2) && !NoWireRx.__sampleArray2Full_irs){
        NoWireRx.__sampleArray2_irs[NoWireRx.__queueIndex_irs-RX_SAMPLE_ARRAY_LENGTH] = sample;
        ++NoWireRx.__queueIndex_irs;

        if(NoWireRx.__queueIndex_irs == RX_SAMPLE_ARRAY_LENGTH*2){
            NoWireRx.__sampleArray2Full_irs = true;
            NoWireRx.__queueIndex_irs = 0;
        }
    }
};
bool NoWireReciever::__sampleRecieverPeriodically(uint32_t hertz){
    if (hertz == 0){
        __rxSamplerTimer.end();
        return true;
    }

    double samplePeriod = (1000000.0/hertz); //in micro seconds

    return __rxSamplerTimer.begin(__sampleRecieverPeriodically_helper,samplePeriod);
}

void NoWireReciever::copyBuffer(volatile int8_t * const buffer, int8_t *copyDestination){
    //no need to CLR_GIB here because the array is protected by the 
    //__sampleArray2Full_irs mutex
    for(size_t i = 0; i < RX_SAMPLE_ARRAY_LENGTH; i++){
        copyDestination[i] = buffer[i];
    }
}

int8_t NoWireReciever::__getBit(){
    CLR_GIB;
    bool __sampleArray1Full = __sampleArray1Full_irs;
    bool __sampleArray2Full = __sampleArray2Full_irs;
    RSTR_GIB;

    //-1 : not set, 0 : 0, 1 : 1
    int8_t bit = -1;
    static int8_t bitBuffer[2] = {-1,-1};

    int8_t bufferCopy[RX_SAMPLE_ARRAY_LENGTH];
    
    if(__sampleArray1Full){ //solve the first queue
        copyBuffer(__sampleArray1_irs, bufferCopy);
        CLR_GIB;
        __sampleArray1Full_irs = false;
        RSTR_GIB;
    }
    else if(__sampleArray2Full){    //solve the second queue
        copyBuffer(__sampleArray2_irs, bufferCopy);
        CLR_GIB;
        __sampleArray2Full_irs = false;
        RSTR_GIB;
    }
    else{   //no buffer is full
        return -1;
    }

    double result1 = __goertzel(FREQUENCY_1, SAMPLE_FREQ, RX_SAMPLE_ARRAY_LENGTH, bufferCopy);
    double result2 = __goertzel(FREQUENCY_2, SAMPLE_FREQ, RX_SAMPLE_ARRAY_LENGTH, bufferCopy);

    if (result1 >= THRESHOLD_1 && result2 < result1){
        bit = 1;
    }
    else if (result2 >= THRESHOLD_2 && result1 < result2){
        bit = 0;
    }
    else {
        bit = -1;
    }

    //shift bit buffer
    bitBuffer[0] = bitBuffer[1];
    bitBuffer[1] = bit;


    //  [-1 , # ] return -1, -> [ - , # ]

    //  [ 0 , 0 ] return  0, -> [ - , - ]
    //  [ 0 , # ] return  0, -> [ - , # ]

    //  [ 1 , 1 ] return  1, -> [ - , - ]
    //  [ 1 , # ] return  1, -> [ - , # ]
    if(bitBuffer[0] == -1){
        return -1;
    }
    else{
        if(bitBuffer[0] == bitBuffer[1]){
            bitBuffer[0] = -1;
            bitBuffer[1] = -1;
            return bit;
        }
        else{
            int8_t returnVal = bitBuffer[0];
            bitBuffer[0] = -1;
            return returnVal;
        }
    }

}

uint32_t NoWireReciever::recieveFourBytes(){

    __sampleRecieverPeriodically(SAMPLE_FREQ);

    static uint64_t packetBitsBuffer = 0;

    uint8_t i = 0;
    while(i < 127){
        int8_t bit = __getBit();
        #if 0
            Serial.println(bit);
        #endif
        if(bit != -1){
            ++i;
            packetBitsBuffer = (packetBitsBuffer<<1) | bit;
            if(__checkPacketSyntaxAndCRC(packetBitsBuffer)){
                __sampleRecieverPeriodically(0);
                uint32_t returnVal = (uint32_t)(packetBitsBuffer >> 28);
                packetBitsBuffer = 0;
                return returnVal;
            }
        }
    }
    __sampleRecieverPeriodically(0);
    return -1;
}

bool NoWireReciever::__checkPacketSyntaxAndCRC(uint64_t packetBits){
    if((packetBits >> 60) == SOH){
        //true SOH
        if((uint8_t)(packetBits << 4) == (uint8_t)(EOT << 4)){
            //true eot
            if(((uint32_t)(packetBits >> 4) & 0xFFFFFF) == __CRC24((uint32_t)(packetBits >> 28))){
                //true crc
                return true;
            }
        }
    }
    return false;
}

double NoWireReciever::__goertzel(uint32_t targetFreq, uint32_t sampleFreq, size_t blockSize,const int8_t *block){

    uint32_t Kterm = 0.5 + (((float)blockSize*targetFreq)/sampleFreq);
    double Wterm = (2.0*PI/blockSize)*Kterm;
    double coeff = 2.0*cos(Wterm);

    double q0 = 0.0;
    double q1 = 0.0;
    double q2 = 0.0;

    for (size_t i = 0; i < blockSize; i++){
        q0 = (coeff * q1) - q2 + block[i];
        q2 = q1;
        q1 = q0;
    }

    return  (q1*q1) + (q2*q2) - (q1 * q2 * coeff);

}

NoWireReciever &NoWireRx = NoWireRx.getInstance();
