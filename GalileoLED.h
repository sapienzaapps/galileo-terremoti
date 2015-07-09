//
// Created by enrico on 09/07/15.
//

#ifndef GALILEO_TERREMOTI_GALILEOLED_H
#define GALILEO_TERREMOTI_GALILEOLED_H

#include "LED.h"

class GalileoLED : public LED {

public:
	static void prepare(uint8_t t, LedMode mode);
	static void set(uint8_t t, LedStatus status);
	static void setEnabled(bool b);
	static bool isEnabled();
private:
	static bool enabled;
};


#endif //GALILEO_TERREMOTI_GALILEOLED_H
