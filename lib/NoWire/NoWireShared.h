#ifndef NOWIRE_SHAREDFN_H
#define NOWIRE_SHAREDFN_H 

#include<Arduino.h>

inline bool disableInterrupts() {
                uint32_t primask;
                __asm__ volatile("mrs %0, primask\n" : "=r" (primask)::);
                __disable_irq();
                return (primask == 0) ? true : false;
}

inline void enableInterrupts(bool doit) {
                if (doit) __enable_irq();
}

#define CLR_GIB bool sreg_backup=disableInterrupts()       //clear global interrupt bit
#define RSTR_GIB enableInterrupts(sreg_backup)                 //restore interrupt bit


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
