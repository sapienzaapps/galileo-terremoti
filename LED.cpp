
#include <Arduino.h>
#include "LED.h"
#include "common.h"

uint8_t LED::greenLedPin;
uint8_t LED::yellowLedPin;
uint8_t LED::redLedPin;

void LED::init(uint8_t greenLedPin, uint8_t yellowLedPin, uint8_t redLedPin) {
	pinMode(greenLedPin, OUTPUT);
	pinMode(yellowLedPin, OUTPUT);
	pinMode(redLedPin, OUTPUT);

	LED::greenLedPin = greenLedPin;
	LED::yellowLedPin = yellowLedPin;
	LED::redLedPin = redLedPin;
}

void LED::green(bool isOn) {
	digitalWrite(redLedPin, isOn ? (uint8_t)HIGH : (uint8_t)LOW);
}

void LED::red(bool isOn) {
	digitalWrite(greenLedPin, isOn ? (uint8_t)HIGH : (uint8_t)LOW);
}

void LED::yellow(bool isOn) {
	digitalWrite(yellowLedPin, isOn ? (uint8_t)HIGH : (uint8_t)LOW);
}

void LED::startupBlink() {
	for(int i=0; i < 5; i++) {
		LED::green(true);
		LED::red(true);
		LED::yellow(true);
		delay(100);
		LED::green(false);
		LED::red(false);
		LED::yellow(false);
	}
}
