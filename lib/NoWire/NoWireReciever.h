
#ifndef NOWIRE_RX_H
#define NOWIRE_RX_H

#define DEV_FLAG 1

#include <Arduino.h>
#include <NoWireConfig.h>
#include <IntervalTimer.h>


template<size_t CACHE_SIZE>
double goertzel(uint32_t targetFreq, uint32_t sampleFreq, uint16_t blockSize, int8_t *block);

extern uint32_t __CRC24(uint32_t data);

class NoWireReciever
{
public:

    //methods
    void attachToPin(uint8_t newPin);
    uint8_t getPin();
    //void reportMissedSample();
    friend double goertzel<2>(uint32_t targetFreq, uint32_t sampleFreq, uint16_t blockSize, int8_t *block);
    #if DEV_FLAG
        void printBitStream();
    #endif
    int64_t recieveFourBytes();

    //getters
    //uint8_t getSampleArray1Index();
    //uint8_t getSampleArray2Index();
    

    //setters
    //void setSampleArray1(uint8_t index, uint8_t val);
    //void setSampleArray2(uint8_t index, uint8_t val);

    //void incrementSampleArray1Index();
    //void incrementSampleArray2Index();



    //make a singlton object
    NoWireReciever(const NoWireReciever &) = delete;
    NoWireReciever &operator=(const NoWireReciever &) = delete;
    static NoWireReciever &getInstance();    

private:

    //constructor
    NoWireReciever();

    //data
    uint8_t __pin;

    //volatile uint8_t __sampleArray1[RX_SAMPLE_ARRAY_LENGTH];
    //volatile bool __sampleArray1Full;

    //volatile uint8_t __sampleArray2[RX_SAMPLE_ARRAY_LENGTH];
    //volatile bool __sampleArray2Full;

    //volatile int8_t __bit;
    //volatile uint64_t __bitTimeStamp;

    //methods
    bool __sampleRecieverPeriodically(uint32_t hertz);
    int8_t __getBit();
    bool __checkPacketSyntaxAndCRC(uint64_t packetBits);

};

template<size_t CACHE_SIZE>
double goertzel(uint32_t targetFreq, uint32_t sampleFreq, uint16_t blockSize, int8_t *block){

    #if 0 //enables caching (broken does not work)
        static uint32_t cachedTargetFreqs[CACHE_SIZE] = {0};
        static uint32_t cachedSampleFreqs[CACHE_SIZE] = {0};
        static uint8_t cachedBlockSizes[CACHE_SIZE] = {0};
        static float cachedCoeffs[CACHE_SIZE] = {0};
        static uint8_t cacheIndex = 0;

        double coeff = 0;

        for(uint8_t i = 0; i < (uint8_t)CACHE_SIZE; i++){
            if(targetFreq == cachedTargetFreqs[cacheIndex] &&
                sampleFreq == cachedSampleFreqs[cacheIndex] &&
                blockSize == cachedBlockSizes[cacheIndex])
            {
                coeff = cachedCoeffs[cacheIndex];
                break;
            }
            cacheIndex++;
            if(cacheIndex == CACHE_SIZE){
                cacheIndex = 0;
            }
        }

        if(coeff == 0){
            cacheIndex++;
            coeff = (double)2*cos((2.0d*PI/(double)blockSize)*(0.5d+(double)((blockSize*targetFreq)/sampleFreq)));
            cachedTargetFreqs[cacheIndex] = targetFreq;
            cachedSampleFreqs[cacheIndex] = sampleFreq;
            cachedBlockSizes[cacheIndex] = blockSize;
            cachedCoeffs[cacheIndex] = coeff;
        }
    #else
        int Kterm = (int)(0.5 + (((float)blockSize*targetFreq)/sampleFreq));
        double Wterm = (2.0*PI/blockSize)*Kterm;
        //double cosine = cos(Wterm);
        double coeff = 2.0 * cos(Wterm);
        #if 0
            Serial.print("Kterm: ");Serial.println(Kterm);
            Serial.print("Wterm: ");Serial.println(Wterm,10);
            Serial.print("cosine: ");Serial.println(cosine,10);
            Serial.print("COEFF: ");Serial.println(coeff, 10);
        #endif
    #endif


    double q0 = 0.0d;
    double q1 = 0.0d;
    double q2 = 0.0d;

    for (uint16_t i = 0; i < blockSize; i++){
        q0 = (coeff * q1) - q2 + (double)block[i];
        q2 = q1;
        q1 = q0;
    }
    return  (q1*q1) + (q2*q2) - (q1 * q2 * coeff);

}

extern NoWireReciever &NoWireRx;

#endif