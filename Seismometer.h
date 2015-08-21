//
// Created by enrico on 12/07/15.
//

#ifndef GALILEO_TERREMOTI_SEISMOMETER_H
#define GALILEO_TERREMOTI_SEISMOMETER_H

#include "Accelerometer.h"
#include "Config.h"

typedef enum {
	Basic, Fixed
} ThresholdAlgorithm_t;

typedef struct {
	unsigned long ts;
	unsigned long ms;
	long valx;
	long valy;
	long valz;
	bool overThreshold;
} RECORD;

typedef struct {
	double pthresx;
	double pthresy;
	double pthresz;
	double nthresx;
	double nthresy;
	double nthresz;
} THRESHOLDS;

class Seismometer {
public:
	Seismometer();
	void init();
	void tick();
	bool isInEvent();
	void showThresholdValues();
	void calibrateIfNeeded(bool force);
	void calibrateIfNeeded();

private:
	ThresholdAlgorithm_t thresholdAlgorithm;
	Accelerometer *accelero;
	THRESHOLDS thresholds;
	bool inEvent;
	unsigned long lastEventWas;
	long nextHour = 0;

	static bool isOverThresholdBasic(RECORD *db, THRESHOLDS *td);
	static bool isOverThresholdFixed(RECORD *db, THRESHOLDS *td);

	void calibrateForHour(int currentHour);
};


#endif //GALILEO_TERREMOTI_SEISMOMETER_H
