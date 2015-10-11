
#include <Arduino.h>
#include <pthread.h>
#include "../../LED.h"
#include "../../Log.h"
#include "../../Utils.h"

uint8_t LED::greenLedPin;
uint8_t LED::yellowLedPin;
uint8_t LED::redLedPin;

volatile bool greenLedStatus = false;
volatile bool yellowLedStatus = false;
volatile bool redLedStatus = false;

volatile bool LED::ledAnimation = false;
volatile bool greenBlinkStatus = false;
volatile bool redBlinkStatus = false;
volatile bool yellowBlinkStatus = false;

pthread_t led_thread;

void *led_doWork(void* mem) {
	int i = 0;
	bool lastBlinkStatus = false;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
	while(true) {
		if(LED::getLedAnimation()) {
			i = (++i) % 3;
			switch(i) {
				default:
				case 0:
					LED::green(true);
					LED::red(false);
					LED::yellow(false);
					break;
				case 1:
					LED::green(false);
					LED::red(true);
					LED::yellow(false);
					break;
				case 2:
					LED::green(false);
					LED::red(false);
					LED::yellow(true);
					break;
			}
			Utils::delay(100);
		} else if(greenBlinkStatus || redBlinkStatus || yellowBlinkStatus) {
			lastBlinkStatus = !lastBlinkStatus;
			if (greenBlinkStatus) {
				LED::green(lastBlinkStatus);
			}
			if (redBlinkStatus) {
				LED::red(lastBlinkStatus);
			}
			if (yellowBlinkStatus) {
				LED::yellow(lastBlinkStatus);
			}
			Utils::delay(500);
		} else {
			Utils::delay(100);
		}
	}
#pragma clang diagnostic pop
	pthread_exit(NULL);
}

void LED::init(uint8_t greenLedPin, uint8_t yellowLedPin, uint8_t redLedPin) {
	pinMode(greenLedPin, OUTPUT);
	pinMode(yellowLedPin, OUTPUT);
	pinMode(redLedPin, OUTPUT);

	LED::greenLedPin = greenLedPin;
	LED::yellowLedPin = yellowLedPin;
	LED::redLedPin = redLedPin;

	int rc = pthread_create(&led_thread, NULL, led_doWork, NULL);
	if(rc) {
		Log::e("Error during LED thread creation");
	}
}

void LED::green(bool isOn) {
	LED::set(greenLedPin, isOn);
}

void LED::red(bool isOn) {
	LED::set(redLedPin, isOn);
}

void LED::yellow(bool isOn) {
	LED::set(yellowLedPin, isOn);
}

bool LED::get(uint8_t pin) {
	if(pin == greenLedPin) {
		return greenLedStatus;
	} else if(pin == redLedPin) {
		return redLedStatus;
	} else if(pin == yellowLedPin) {
		return yellowLedStatus;
	} else {
		return false;
	}
}

void LED::set(uint8_t pin, bool isOn) {
	if(pin == greenLedPin) {
		greenLedStatus = isOn;
	} else if(pin == redLedPin) {
		redLedStatus = isOn;
	} else if(pin == yellowLedPin) {
		yellowLedStatus = isOn;
	} else {
		return;
	}
	digitalWrite(pin, isOn ? (uint8_t)HIGH : (uint8_t)LOW);
}

void LED::clearLedBlinking() {
	greenBlinkStatus = false;
	redBlinkStatus = false;
	yellowBlinkStatus = false;
}

void LED::clearLedBlinking(uint8_t pin) {
	if(pin == greenLedPin) {
		greenBlinkStatus = false;
		LED::green(false);
	}
	if(pin == redLedPin) {
		redBlinkStatus = false;
		LED::red(false);
	}
	if(pin == yellowLedPin) {
		yellowBlinkStatus = false;
		LED::yellow(false);
	}
}

void LED::setLedBlinking(uint8_t pin) {
	if(pin == greenLedPin) {
		greenBlinkStatus = true;
	}
	if(pin == redLedPin) {
		redBlinkStatus = true;
	}
	if(pin == yellowLedPin) {
		yellowBlinkStatus = true;
	}
}

void LED::startupBlink() {
	for(int i=0; i < 10; i++) {
		LED::green(true);
		LED::red(true);
		LED::yellow(true);
		delay(20);
		LED::green(false);
		LED::red(false);
		LED::yellow(false);
		delay(20);
	}
}

void LED::tick() {}

void LED::dispose() {
	pthread_exit(NULL);
}

void LED::setLedAnimation(bool b) {
	ledAnimation = b;
	if(!b) {
		LED::green(false);
		LED::red(false);
		LED::yellow(false);
	}
}

bool LED::getLedAnimation() {
	return ledAnimation;
}
