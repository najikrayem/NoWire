#ifndef NOWIRE_SHAREDFN_H
#define NOWIRE_SHAREDFN_H 

#include<Arduino.h>


uint32_t __CRC24(uint32_t data){
    //CRC: HD=9, CRC=24, LEN=32 (max 39), polynomial = 0xed93bb
    const uint32_t polynomial = 0xed93bb00;     //msb removed
    const uint32_t msbMask = 0x80000000;
    for(uint8_t i = 0; i < 32; i++){
        if(data & msbMask){
            data = data << 1;
            data = data ^ polynomial;
        } else {
            data = data << 1;
        }
    }

    #if 0 
        Serial.print("CRC: "); Serial.println(data>>8);
    #endif

    return (data >> 8);
}



#endif
