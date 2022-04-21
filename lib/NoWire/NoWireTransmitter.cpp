#include <NoWireTransmitter.h>
#include <NoWireShared.h>
#include <NoWireConfig.h>

NoWireTransmitter::NoWireTransmitter():
__pin_irs(DEFAULT_TX_PIN)
{}

void NoWireTransmitter::attachToPin(uint8_t newPin){
    pinMode(newPin, OUTPUT);
    CLR_GIB;
    __pin_irs = newPin;
    RSTR_GIB;
}

uint8_t NoWireTransmitter::getPin(){
    uint8_t pin;
    CLR_GIB;
    pin = __pin_irs;
    RSTR_GIB;
    return pin;
}

static IntervalTimer __waveTimer;
void __produceFrequency_nonblocking_helper(){
    digitalToggleFast(NoWireTx.__pin_irs);
};
bool NoWireTransmitter::__produceFrequency_nonblocking(uint32_t hertz){
    static uint32_t cachedHertz = 0;
    static double togglePeriod = 0;

    if (hertz == 0){
        digitalWriteFast(NoWireTx.__pin_irs, 0);
        __waveTimer.end();
        return true;
    }

    if(cachedHertz != hertz){
        togglePeriod = (1000000.0/hertz)/2.0;//in micro seconds
        cachedHertz = hertz;
        return __waveTimer.begin(__produceFrequency_nonblocking_helper,togglePeriod);
    }

    return true;
}

void NoWireTransmitter::__sendBits(uint64_t bits){

    const uint64_t msbMask = (uint64_t)1 << 63;

    for(uint8_t i = 0; i < 64; i++){
        if(bits & msbMask){
            __produceFrequency_nonblocking(FREQUENCY_1);
        }else{
           __produceFrequency_nonblocking(FREQUENCY_2); 
        }
        delayMicroseconds(MODULATION_PERIOD);
        bits = bits << 1;
    }
    __produceFrequency_nonblocking(0);
}

void NoWireTransmitter::sendFourBytes(uint32_t bytes){
    uint64_t packetBits = 0;
    packetBits = (packetBits | (((uint64_t)SOH) << 60));
    packetBits = (packetBits | (((uint64_t)bytes) << 28));
    packetBits = (packetBits | (((uint64_t)__CRC24(bytes)) << 4));
    packetBits = (packetBits | ((uint64_t)EOT));

    __sendBits(packetBits);
}

NoWireTransmitter &NoWireTransmitter::getInstance() {
  static NoWireTransmitter instance;
  return instance;
}

NoWireTransmitter &NoWireTx = NoWireTx.getInstance();
