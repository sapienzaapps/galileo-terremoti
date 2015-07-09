//
// Created by enrico on 09/07/15.
//

#ifndef GALILEO_TERREMOTI_LED_H
#define GALILEO_TERREMOTI_LED_H

#include <stdint.h>

typedef enum {
	LED_MODE_OUTPUT,
	LED_MODE_INPUT
} LedMode;

typedef enum {
	LED_ON,
	LED_OFF
} LedStatus;

class LED {
public:
	static virtual void setEnabled(bool) = 0;
	static virtual bool isEnabled() = 0;
	static virtual void prepare(uint8_t, LedMode) = 0;
	static virtual void set(uint8_t, LedStatus) = 0;
};


#endif //GALILEO_TERREMOTI_LED_H
