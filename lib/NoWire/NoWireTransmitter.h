
#ifndef NOWIRE_TX_h
#define NOWIRE_TX_h

#include <Arduino.h>

class NoWireTransmitter{
public:

    //methods
    void attachToPin(uint8_t newPin);
    void sendFourBytes(uint32_t bytes);
    uint8_t getPin();

    //make a singlton object
    NoWireTransmitter(const NoWireTransmitter &) = delete;
    NoWireTransmitter &operator=(const NoWireTransmitter &) = delete;
    static NoWireTransmitter &getInstance();

private:
    //constructor
    NoWireTransmitter();

    //data
    volatile uint8_t __pin_irs;

    //methods
    void __sendBits(uint64_t bits);
    bool __produceFrequency_nonblocking(uint32_t hertz);
    friend void __produceFrequency_nonblocking_helper();
    
};

extern NoWireTransmitter &NoWireTx;

#endif