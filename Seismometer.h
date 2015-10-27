//
// Created by enrico on 12/07/15.
//

#ifndef GALILEO_TERREMOTI_SEISMOMETER_H
#define GALILEO_TERREMOTI_SEISMOMETER_H

#include "Accelerometer.h"
#include "Config.h"

// #define SAVE_THRESHOLD

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
	 * Calibrate if needed
	 * @param force If true, device will recalibrate
	 */
	void calibrate(bool force);

	/**
	 * Calibrate if needed
	 */
	void calibrateIfNeeded();

	/**
	 * Get seismometer threshold
	 * @return Threshold values
	 */
	THRESHOLDS getThresholds();

	/**
	 * Get accelerometer name
	 * @return Get accelerometer name
	 */
	std::string getAccelerometerName();

	/**
	 * Get Seismometer instance (singleton)
	 */
	static Seismometer* getInstance();

	unsigned int getStatProbeSpeed();

private:
	Seismometer();
	ThresholdAlgorithm_t thresholdAlgorithm;
	Accelerometer *accelero;
	THRESHOLDS thresholds;
	HOUR thresholdHour;
	bool inEvent;
	unsigned long lastEventWas;
	HOUR nextHour = 0;

	unsigned long statLastCounterTime = 0;
	unsigned int statLastCounter = 0;
	unsigned int statProbeSpeed = 0;

	static bool isOverThresholdBasic(RECORD *db, THRESHOLDS *td);
	static bool isOverThresholdFixed(RECORD *db, THRESHOLDS *td);

	static Seismometer* instance;

	void logThresholdValues();
	void calibrateForHour(HOUR currentHour);

#ifdef SAVE_THRESHOLD
	void saveCalibration(HOUR currentHour);
	void loadThresholdIfNeeded();
	void createDBifNeeded();
#endif
};


#endif //GALILEO_TERREMOTI_SEISMOMETER_H
