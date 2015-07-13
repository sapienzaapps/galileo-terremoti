//
// Created by enrico on 09/07/15.
//

#ifndef GALILEO_TERREMOTI_LED_H
#define GALILEO_TERREMOTI_LED_H

#include <stdint.h>

typedef enum {
	LED_RED,
	LED_GREEN,
	LED_YELLOW
} LedColor;

typedef enum {
	LED_ON,
	LED_OFF
} LedStatus;

class LED {
public:
	static void init(uint8_t greenLedPin, uint8_t yellowLedPin, uint8_t redLedPin);
	static void set(LedColor led, bool isOn);
	static void on(LedColor led);
	static void off(LedColor led);
private:
	static uint8_t greenLedPin;
	static uint8_t yellowLedPin;
	static uint8_t redLedPin;
};


#endif //GALILEO_TERREMOTI_LED_H
