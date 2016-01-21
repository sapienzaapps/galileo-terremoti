//
// Created by enrico on 12/07/15.
//

#ifndef GALILEO_TERREMOTI_SEISMOMETER_H
#define GALILEO_TERREMOTI_SEISMOMETER_H

#include "Accelerometer.h"
#include "Config.h"
#include "net/Collector.h"

typedef uint8_t HOUR;

typedef struct {
	unsigned long ts;
	uint64_t ms;
	double accel;
	bool overThreshold;
} RECORD;

/**
 * Seismometer class
 */
class Seismometer {
public:
	/**
	 * Init seismometer class
	 */
	void init();

	/**
	 * Tick (if no threaded)
	 */
	void tick();

	/**
	 * Get accelerometer name
	 * @return Get accelerometer name
	 */
	std::string getAccelerometerName();

	unsigned int getStatProbeSpeed();
	void setQuakeThreshold(float);
	float getQuakeThreshold();

	/**
	 * Get Seismometer instance (singleton)
	 */
	static Seismometer* getInstance();

private:
	Seismometer();
	Accelerometer *accelero;
	bool inEvent;
	unsigned long lastEventWas;
	Collector *serverCollector;

	unsigned long statLastCounterTime = 0;
	unsigned int statLastCounter = 0;
	unsigned int statProbeSpeed = 0;
	float quakeThreshold = 0.3;

	static Seismometer* instance;
};


#endif //GALILEO_TERREMOTI_SEISMOMETER_H
