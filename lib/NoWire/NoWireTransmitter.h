
#ifndef NOWIRE_TX_h
#define NOWIRE_TX_h

#include <Arduino.h>
#include <NoWireConfig.h>
#include <IntervalTimer.h>


extern uint32_t __CRC24(uint32_t data);

class NoWireTransmitter{
public:

    //methods
    void attachToPin(uint8_t newPin);
    uint8_t getPin();


    //make a singlton object
    NoWireTransmitter(const NoWireTransmitter &) = delete;
    NoWireTransmitter &operator=(const NoWireTransmitter &) = delete;
    static NoWireTransmitter &getInstance();
    void sendFourBytes(uint32_t bytes);
    
private:
    //constructor
    NoWireTransmitter();

    //data
    uint8_t pin;

    //methods
    void __produceFrequency_blocking(uint32_t hertz, uint64_t nanoSecs);
    void __sendBits(uint64_t bits);
    bool __produceFrequency_nonblocking(uint32_t hertz);
    
};

extern NoWireTransmitter &NoWireTx;

#endif