//
// Created by enrico on 09/07/15.
//

#ifndef GALILEO_TERREMOTI_LED_H
#define GALILEO_TERREMOTI_LED_H

#include <stdint.h>

class LED {
public:
	static void init(uint8_t greenLedPin, uint8_t yellowLedPin, uint8_t redLedPin);
	static void green(bool);
	static void red(bool);
	static void yellow(bool);
	static void startupBlink();
	static void tick();
	static void setLedAnimation(bool);
	static bool getLedAnimation();
	static void dispose();
	static void set(uint8_t pin, bool isOn);
	static void setLedBlinking(uint8_t pin);
	static void clearLedBlinking();
	static bool get(uint8_t pin);
private:
	static uint8_t greenLedPin;
	static uint8_t yellowLedPin;
	static uint8_t redLedPin;
	static volatile bool ledAnimation;
};


#endif //GALILEO_TERREMOTI_LED_H
