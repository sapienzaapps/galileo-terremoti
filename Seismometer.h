//
// Created by enrico on 12/07/15.
//

#ifndef GALILEO_TERREMOTI_SEISMOMETER_H
#define GALILEO_TERREMOTI_SEISMOMETER_H

#include "Accelerometer.h"
#include "Config.h"

typedef uint8_t HOUR;

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
	void calibrate(bool force);
	void calibrateIfNeeded();

private:
	ThresholdAlgorithm_t thresholdAlgorithm;
	Accelerometer *accelero;
	THRESHOLDS thresholds;
	HOUR thresholdHour;
	bool inEvent;
	unsigned long lastEventWas;
	HOUR nextHour = 0;

	static bool isOverThresholdBasic(RECORD *db, THRESHOLDS *td);
	static bool isOverThresholdFixed(RECORD *db, THRESHOLDS *td);

	void logThresholdValues();
	void calibrateForHour(HOUR currentHour);
	void saveCalibration(HOUR currentHour);
	void loadThresholdIfNeeded();
	void createDBifNeeded();
};


#endif //GALILEO_TERREMOTI_SEISMOMETER_H
