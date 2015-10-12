
#include "../../Log.h"
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
	if(isOn) {
		Log::d("Green LED on");
	} else {
		Log::d("Green LED off");
	}
}

void LED::red(bool isOn) {
	if(isOn) {
		Log::d("Red LED on");
	} else {
		Log::d("Red LED off");
	}
}

void LED::yellow(bool isOn) {
	if(isOn) {
		Log::d("Yellow LED on");
	} else {
		Log::d("Yellow LED off");
	}
}

void LED::setLedBlinking(uint8_t pin) {
	Log::d("Set led %i blinking", pin);
}

void LED::clearLedBlinking() {
	Log::d("Clear ALL led blinking");
}

void LED::clearLedBlinking(uint8_t pin) {
	Log::d("Clear led %i blinking", pin);
}

void LED::set(uint8_t pin, bool status) {
	Log::d((status ? "Set led %i on" : "Set led %i off"), pin);
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
	if(b) {
		Log::d("Set LED animation ON");
	} else {
		Log::d("Set LED animation OFF");
	}
}

bool LED::getLedAnimation() {
	return ledAnimation;
}
