#ifndef NOWIRE_CONFIG_H
#define NOWIRE_CONFIG_H

#include <Arduino.h>

/* THIS WORKS */

#define DEFAULT_TX_PIN 13
#define DEFAULT_RX_PIN 14

#define SAMPLE_FREQ 4000
#define FREQUENCY_1 500     //Must be smaller than FREQUENCY_2
#define FREQUENCY_2 800
#define THRESHOLD_1 45   //FREQUENCY_1 : 0.0 / FREQUENCY_2 : 65
#define THRESHOLD_2 45   //FREQUENCY_1 : 63 / FREQUENCY_2 : 1.0
#define RX_SAMPLE_ARRAY_LENGTH 200

#define MODULATION_PERIOD 100000   //microseconds



/*

#define DEFAULT_TX_PIN 13
#define DEFAULT_RX_PIN 14

#define MIN_CYCLES_PER_FRAME 64
#define FREQUENCY_1 1000     
#define THRESHOLD_1 45   //FREQUENCY_1 : 0.0 / FREQUENCY_2 : 65
#define THRESHOLD_2 45   //FREQUENCY_1 : 63 / FREQUENCY_2 : 1.0


#define FREQUENCY_2 (FREQUENCY_1 * 1.1d)
#define SAMPLE_FREQ ((unsigned long int)((double)FREQUENCY_2 * 2.2d))
#define MODULATION_PERIOD ((unsigned long long int)((1000000.0d / (double)FREQUENCY_1) * MIN_CYCLES_PER_FRAME * 2)) //microseconds
#define RX_SAMPLE_ARRAY_LENGTH ((unsigned long int)(((double)SAMPLE_FREQ/1000.0d) * ((double)MODULATION_PERIOD / (2.0d * 1000.0d))))

*/

//Special Chars
#define SOH 0x0F
#define EOT 0x00


#endif



