#ifndef NOWIRE_RX_H
#define NOWIRE_RX_H

#define DEV_FLAG 0

#include <Arduino.h>
#include <NoWireConfig.h>

class NoWireReciever
{
public:

    //methods
    void attachToPin(uint8_t newPin);
    uint8_t getPin();
    uint32_t recieveFourBytes();
    #if DEV_FLAG
        void printBitStream();
    #endif


    //make a singlton object
    NoWireReciever(const NoWireReciever &) = delete;
    NoWireReciever &operator=(const NoWireReciever &) = delete;
    static NoWireReciever &getInstance();    

private:

    //constructor
    NoWireReciever();

    //data
    volatile uint8_t __pin_irs;

    volatile int8_t __sampleArray1_irs[RX_SAMPLE_ARRAY_LENGTH];   //buffer 1
    volatile bool __sampleArray1Full_irs = false;

    volatile int8_t __sampleArray2_irs[RX_SAMPLE_ARRAY_LENGTH];   //buffer 1
    volatile bool __sampleArray2Full_irs = false;

    volatile uint16_t __queueIndex_irs = 0;

    //methods
    bool __sampleRecieverPeriodically(uint32_t hertz);
    int8_t __getBit();
    bool __checkPacketSyntaxAndCRC(uint64_t packetBits);
    double __goertzel(uint32_t targetFreq, uint32_t sampleFreq, size_t blockSize, const int8_t *block);
    void copyBuffer(volatile int8_t * const buffer, int8_t *copyDestination);
    friend void __sampleRecieverPeriodically_helper();

};

extern NoWireReciever &NoWireRx;

#endif