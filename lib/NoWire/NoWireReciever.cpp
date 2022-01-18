#include <NoWireReciever.h>
#include <NoWireShared.h>

volatile int8_t __sampleArray1_irs[RX_SAMPLE_ARRAY_LENGTH] = {0};
volatile bool __sampleArray1Full_irs = false;
volatile int8_t __sampleArray2_irs[RX_SAMPLE_ARRAY_LENGTH] = {0};
volatile bool __sampleArray2Full_irs = false;
volatile uint16_t __queueIndex_irs = 0;
volatile uint8_t __pin_irs = DEFAULT_RX_PIN;
IntervalTimer __rxSamplerTimer;
void __sampleRecieverPeriodically_helper();

NoWireReciever::NoWireReciever():
__pin(DEFAULT_RX_PIN)
{
}

void NoWireReciever::attachToPin(uint8_t newPin){
    pinMode(newPin, INPUT);
    __pin = newPin;
    noInterrupts();
    __pin_irs = newPin;
    interrupts();
}

uint8_t NoWireReciever::getPin(){
    return __pin;
}

// void NoWireReciever::reportMissedSample(){
//     //TODO
//     return;
// }

#if DEV_FLAG
void NoWireReciever::printBitStream(){
    __sampleRecieverPeriodically(SAMPLE_FREQ);
    while(true){
        int8_t bit;
        bit = __getBit();
        if(bit == 1 || bit == 0){
            Serial.println(bit);
        }
        #if 0
            Serial.println(bit);
        #endif
        #if 0
            delayMicroseconds(400);
        #endif
    }
}
#endif

// uint8_t NoWireReciever::getSampleArray1Index(){
//     uint8_t val;
//     noInterrupts();
//     val = __sampleArray1Index;
//     interrupts();
//     return val;
// }

// uint8_t NoWireReciever::getSampleArray2Index(){
//     uint8_t val;
//     noInterrupts();
//     val = __sampleArray2Index;
//     interrupts();
//     return val;
// }

// void NoWireReciever::setSampleArray1(uint8_t index, uint8_t val){
//     //noInterrupts();
//     __sampleArray1[index] = val;
//     //interrupts();
// }

// void NoWireReciever::setSampleArray2(uint8_t index, uint8_t val){
//     //noInterrupts();
//     __sampleArray2[index] = val;
//     //interrupts();
// }

// void NoWireReciever::incrementSampleArray1Index(){
//     noInterrupts();
//     __sampleArray1Index++;
//     interrupts();
// }

// void NoWireReciever::incrementSampleArray2Index(){
//     noInterrupts();
//     __sampleArray2Index++;
//     interrupts();
// }

NoWireReciever &NoWireReciever::getInstance() {
  static NoWireReciever instance;
  return instance;
}

void __sampleRecieverPeriodically_helper(){

    uint8_t sample = digitalReadFast(__pin_irs);

    #if 0
        Serial.println(sample);
    #endif

    #if 0
        Serial.println(micros());
    #endif

    if((__queueIndex_irs < RX_SAMPLE_ARRAY_LENGTH) && !__sampleArray1Full_irs){

        __sampleArray1_irs[__queueIndex_irs] = sample;
        __queueIndex_irs++;

        if(__queueIndex_irs == RX_SAMPLE_ARRAY_LENGTH){
            __sampleArray1Full_irs = true;
        }
    }
    else if((__queueIndex_irs < (RX_SAMPLE_ARRAY_LENGTH*2)) && !__sampleArray2Full_irs){
        __sampleArray2_irs[__queueIndex_irs-RX_SAMPLE_ARRAY_LENGTH] = sample;
        __queueIndex_irs++;

        if(__queueIndex_irs == (RX_SAMPLE_ARRAY_LENGTH*2)){
            __sampleArray2Full_irs = true;
            __queueIndex_irs = 0;
        }
    }
};
bool NoWireReciever::__sampleRecieverPeriodically(uint32_t hertz){
    if (hertz == 0){
        __rxSamplerTimer.end();
        return true;
    }

    double samplePeriod = ((double)1000000/(double)hertz);//in micro seconds

    return __rxSamplerTimer.begin(__sampleRecieverPeriodically_helper,samplePeriod);
}

int8_t NoWireReciever::__getBit(){
    noInterrupts();
    bool __sampleArray1Full = __sampleArray1Full_irs;
    bool __sampleArray2Full = __sampleArray2Full_irs;
    interrupts();

    //-1 : not set, 0 : 0, 1 : 1
    int8_t bit = -1;
    static int8_t bitBuffer[2] = {-1,-1};
    
    if(__sampleArray1Full){ //solve the first queue
        double result1;
        double result2;

        //copy sample array
        int8_t __sampleArray1[RX_SAMPLE_ARRAY_LENGTH];
        noInterrupts();
        for(uint8_t i = 0; i < RX_SAMPLE_ARRAY_LENGTH; i++){
            __sampleArray1[i] = __sampleArray1_irs[i];
        }
        __sampleArray1Full_irs = false;
        interrupts();

        result1 = goertzel<2>(FREQUENCY_1, SAMPLE_FREQ, RX_SAMPLE_ARRAY_LENGTH, __sampleArray1);
        result2 = goertzel<2>(FREQUENCY_2, SAMPLE_FREQ, RX_SAMPLE_ARRAY_LENGTH, __sampleArray1);

        #if 0
            Serial.println("Queue 1 :");
            Serial.print("\tresult 1 : ");Serial.println(result1, 10);
            Serial.print("\tresult 2 : ");Serial.println(result2, 10);
            // Serial.println("samples:");
            // for(uint16_t i = 0; i < RX_SAMPLE_ARRAY_LENGTH; i++){
            //     Serial.println(__sampleArray1[i]);
            // }
        #endif

        if (result1 >= THRESHOLD_1 && result2 < result1){
            bit = 1;
        }
        else if (result2 >= THRESHOLD_2 && result1 < result2){
            bit = 0;
        }
        else {
            bit = -1;
        }

    }
    else if(__sampleArray2Full){    //solve the second queue
        double result1;
        double result2;

        //copy sample array
        int8_t __sampleArray2[RX_SAMPLE_ARRAY_LENGTH];
        noInterrupts();
        for(uint8_t i = 0; i < RX_SAMPLE_ARRAY_LENGTH; i++){
            __sampleArray2[i] = __sampleArray2_irs[i];
        }
        __sampleArray2Full_irs = false;
        interrupts();

        result1 = goertzel<2>(FREQUENCY_1, SAMPLE_FREQ, RX_SAMPLE_ARRAY_LENGTH, __sampleArray2);
        result2 = goertzel<2>(FREQUENCY_2, SAMPLE_FREQ, RX_SAMPLE_ARRAY_LENGTH, __sampleArray2);

        #if 0
            Serial.println("Queue 2 :");
            Serial.print("\tresult 1 : ");Serial.println(result1, 10);
            Serial.print("\tresult 2 : ");Serial.println(result2, 10);
            Serial.println();
        #endif

        if (result1 >= THRESHOLD_1 && result2 < result1){
            bit = 1;
        }
        else if (result2 >= THRESHOLD_2 && result1 < result2){
            bit = 0;
        }
        else {
            bit = -1;
        }
    }
    else{   //no buffer is full
        return -1;
    }

    //shift bit buffer
    bitBuffer[0] = bitBuffer[1];
    bitBuffer[1] = bit;

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

    // if((bit1==1 && bit2==1) || (bit1==1 && bit2==2) || (bit1==2 && bit2==1)){
    //     bit1=-1;
    //     bit2=-1;
    //     return 1;
    // }
    // if((bit1==0 && bit2==0) || (bit1==0 && bit2==2) || (bit1==2 && bit2==0)){
    //     bit1=-1;
    //     bit2=-1;
    //     return 0;
    // }
    // if(bit1==-1 || bit2==-1){
    //     return -1;
    // }
    // else{
    //     bit1=-1;
    //     bit2=-1;
    //     return 2;
    // }
}

int64_t NoWireReciever::recieveFourBytes(){

    __sampleRecieverPeriodically(SAMPLE_FREQ);

    static uint64_t packetBitsBuffer = 0;

    uint8_t i = 0;
    while(i < 127){
        int8_t bit = __getBit();
        #if 0
            Serial.println(bit);
        #endif
        if(bit != -1){
            i++;
            packetBitsBuffer = (packetBitsBuffer<<1) | bit;
            if(__checkPacketSyntaxAndCRC(packetBitsBuffer)){
                __sampleRecieverPeriodically(SAMPLE_FREQ);
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
    #if 0
        Serial.println((uint64_t)packetBits, BIN);
    #endif
    if((packetBits >> 60) == SOH){
        if((uint8_t)(packetBits << 4) == (uint8_t)(EOT << 4)){
            #if 0
                Serial.println("true EOT");
            #endif
            if(((uint32_t)(packetBits >> 4) & 0xFFFFFF) == __CRC24((uint32_t)(packetBits >> 28))){
                #if 0
                    Serial.println("true CRC");
                #endif
                return true;
            }
            #if 0
                else{
                    Serial.println("wrong CRC");
                    Serial.println(((uint32_t)(packetBits >> 4) & 0xFFFFFF));
                    Serial.println(__CRC24((uint32_t)(packetBits >> 28)));
                }
            #endif
        }
        #if 0
            else{
                Serial.println("wrong EOT");
            }
        #endif
    }
    #if 0
        else{
            Serial.println("wrong SOH");
        }
    #endif
    return false;
}

NoWireReciever &NoWireRx = NoWireRx.getInstance();
