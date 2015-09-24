
#include <Arduino.h>
#include <pthread.h>
#include "../../LED.h"
#include "../../Log.h"
#include "../../Utils.h"

uint8_t LED::greenLedPin;
uint8_t LED::yellowLedPin;
uint8_t LED::redLedPin;

volatile bool LED::ledAnimation = false;

pthread_t led_thread;

void *led_doWork(void* mem) {
	int i = 0;
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
		}
		Utils::delay(100);
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

void LED::tick() {}

void LED::dispose() {
	pthread_exit(NULL);
}

void LED::setLedAnimation(bool b) {
	ledAnimation = b;
}

bool LED::getLedAnimation() {
	return ledAnimation;
}
