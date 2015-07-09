//
// Created by enrico on 09/07/15.
//

#include <Arduino.h>
#include "GalileoLED.h"

void GalileoLED::prepare(uint8_t t, LedMode mode) {
	if(isEnabled()) {
		pinMode(t, (uint8_t) (mode == LED_MODE_OUTPUT ? OUTPUT : INPUT));
	}
}

void GalileoLED::set(uint8_t t, LedStatus status) {
	if(isEnabled()) {
		digitalWrite(t, (uint8_t) (status == LED_ON ? HIGH : LOW));
	}
}

void GalileoLED::setEnabled(bool b) {
	enabled = b;
}

bool GalileoLED::isEnabled() {
	return enabled;
}
