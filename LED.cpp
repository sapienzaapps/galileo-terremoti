
#include "LED.h"
#include <Arduino.h>

void LED::init(uint8_t greenLedPin, uint8_t yellowLedPin, uint8_t redLedPin) {
	pinMode(greenLedPin, OUTPUT);
	pinMode(yellowLedPin, OUTPUT);
	pinMode(redLedPin, OUTPUT);

	LED::greenLedPin = greenLedPin;
	LED::yellowLedPin = yellowLedPin;
	LED::redLedPin = redLedPin;
}

void LED::set(LedColor led, bool isOn) {
	switch(led) {
		case LED_RED:
			digitalWrite(redLedPin, isOn ? (uint8_t)HIGH : (uint8_t)LOW);
			break;
		case LED_GREEN:
			digitalWrite(greenLedPin, isOn ? (uint8_t)HIGH : (uint8_t)LOW);
			break;
		case LED_YELLOW:
			digitalWrite(yellowLedPin, isOn ? (uint8_t)HIGH : (uint8_t)LOW);
			break;
		default:
			break;
	}
}

void LED::on(LedColor led) {
	LED::set(led, true);
}

void LED::off(LedColor led) {
	LED::set(led, false);
}
