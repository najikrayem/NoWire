
#include <NoWireConfig.h>

void printParams(){
    Serial.print("FREQUENCY_2: ");Serial.println(FREQUENCY_2);
    Serial.print("SAMPLE_FREQ: ");Serial.println(SAMPLE_FREQ);
    Serial.print("MODULATION_PERIOD: ");Serial.println(MODULATION_PERIOD);
    Serial.print("RX_SAMPLE_ARRAY_LENGTH: ");Serial.println(RX_SAMPLE_ARRAY_LENGTH);
	Serial.print("Expected bit rate: ");Serial.println(1000000 / MODULATION_PERIOD);
};

#define TransmitterCode 1

#if TransmitterCode

#include <Arduino.h>
#include <NoWireTransmitter.h>

void setup() {
	Serial.begin(115200);
	delay(50);
	NoWireTx.attachToPin(LED_BUILTIN);
	printParams();
}

uint32_t i = 0;
void loop() {
	NoWireTx.sendFourBytes(i);
	i++;
}

#else	//receiver code

#include <Arduino.h>
#include <NoWireReciever.h>

void setup() {
	// put your setup code here, to run once:
	NoWireRx.attachToPin(DEFAULT_RX_PIN);
	Serial.begin(115200);
	delay(50);
	printParams();
}

void loop() {
	int64_t recievedBytes = NoWireRx.recieveFourBytes();

	//Serial.print("Recived Bytes: ");
	if(recievedBytes >= 0){
		Serial.println(recievedBytes);
	}
	
}

#endif
