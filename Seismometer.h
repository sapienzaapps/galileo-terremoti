//
// Created by enrico on 12/07/15.
//

#ifndef GALILEO_TERREMOTI_SEISMOMETER_H
#define GALILEO_TERREMOTI_SEISMOMETER_H

#include "AcceleroMMA7361.h"
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

private:
	ThresholdAlgorithm_t thresholdAlgorithm;
	AcceleroMMA7361 accelero;
	THRESHOLDS thresholds;
	bool inEvent;
	unsigned long lastEventWas;
	long nextHour = 0;

	static bool isOverThresholdBasic(RECORD *db, THRESHOLDS *td);
	static bool isOverThresholdFixed(RECORD *db, THRESHOLDS *td);

	void calibrateForHour(int currentHour);
	void calibrateIfNeeded(bool force);
	void calibrateIfNeeded();
};


#endif //GALILEO_TERREMOTI_SEISMOMETER_H
