#include <NoWireTransmitter.h>
#include <NoWireShared.h>

NoWireTransmitter::NoWireTransmitter():
pin(DEFAULT_TX_PIN)
{
}

uint8_t x = 0;

void NoWireTransmitter::attachToPin(uint8_t newPin){
    pinMode(newPin, OUTPUT);
    pin = newPin;
}

uint8_t NoWireTransmitter::getPin(){
    return pin;
}

void NoWireTransmitter::__produceFrequency_blocking(uint32_t hertz, uint64_t DurationNanoSecs){
    //TODO
    while(true);
}

IntervalTimer __waveTimer;
void __produceFrequency_nonblocking_helper(){
    digitalToggleFast(NoWireTx.getPin());
};
bool NoWireTransmitter::__produceFrequency_nonblocking(uint32_t hertz){
    static uint32_t cachedHertz = 0;
    static double togglePeriod = 0;

    if (hertz == 0){
        digitalWriteFast(NoWireTx.getPin(), 0);
        __waveTimer.end();
        return true;
    }

    if(cachedHertz != hertz){
        togglePeriod = ((double)1000000/(double)hertz)/(double)2;//in micro seconds
        cachedHertz = hertz;
    }

    #if 0
        Serial.print("TogglePeriod: ");Serial.println(togglePeriod);
    #endif

    return __waveTimer.begin(__produceFrequency_nonblocking_helper,togglePeriod);
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

    #if 0
        Serial.print("packetBits :");Serial.println(packetBits, BIN);
    #endif

    __sendBits(packetBits);
}

NoWireTransmitter &NoWireTransmitter::getInstance() {
  static NoWireTransmitter instance;
  return instance;
}

NoWireTransmitter &NoWireTx = NoWireTx.getInstance();
