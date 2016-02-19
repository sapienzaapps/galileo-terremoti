//
// Created by enrico on 12/07/15.
//

#ifndef GALILEO_TERREMOTI_SEISMOMETER_H
#define GALILEO_TERREMOTI_SEISMOMETER_H

#include "Accelerometer.h"
#include "Config.h"
#include "net/Collector.h"

typedef struct {
	unsigned long ts;
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
	void setQuakeThreshold(double);
	void firstTimeThresholdCalculation();
	double getQuakeThreshold();

	void setSigmaIter(double);
	double getSigmaIter();

	double getLastPeriodAVG();
	double getLastPeriodVAR();
	void resetLastPeriod();

	/**
	 * Get Seismometer instance (singleton)
	 */
	static Seismometer* getInstance();

private:
	Seismometer();

	void addValueToAvgVar(double val);

	Accelerometer *accelero;
	bool inEvent;
	unsigned long lastEventWas;
	Collector *serverCollector;

	double lastPeriodAvg = 0;
	double lastPeriodSQM = 0;
	unsigned int elements = 0;

	unsigned long statLastCounterTime = 0;
	unsigned int statLastCounter = 0;
	unsigned int statProbeSpeed = 0;
	double quakeThreshold = 0.3;
	double sigmaIter = 1;

	static Seismometer* instance;
};


#endif //GALILEO_TERREMOTI_SEISMOMETER_H
