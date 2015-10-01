
#include "../../LED.h"
#include "../../Utils.h"

uint8_t LED::greenLedPin;
uint8_t LED::yellowLedPin;
uint8_t LED::redLedPin;

volatile bool LED::ledAnimation = false;

void LED::init(uint8_t greenLedPin, uint8_t yellowLedPin, uint8_t redLedPin) {
	LED::greenLedPin = greenLedPin;
	LED::yellowLedPin = yellowLedPin;
	LED::redLedPin = redLedPin;
}

void LED::green(bool isOn) {
}

void LED::red(bool isOn) {
}

void LED::yellow(bool isOn) {
}

void LED::setLedBlinking(uint8_t pin) {
}

void LED::clearLedBlinking() {
}

void LED::set(uint8_t pin, bool status) {
}

void LED::startupBlink() {
	for(int i=0; i < 5; i++) {
		LED::green(true);
		LED::red(true);
		LED::yellow(true);
		Utils::delay(100);
		LED::green(false);
		LED::red(false);
		LED::yellow(false);
	}
}

void LED::tick() {}

void LED::dispose() { }

void LED::setLedAnimation(bool b) {
}

bool LED::getLedAnimation() {
	return ledAnimation;
}
