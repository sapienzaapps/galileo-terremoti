//
// Created by enrico on 12/07/15.
//

#include "common.h"
#include "Seismometer.h"
#include "Log.h"
#include "avg.h"
#include "net/NTP.h"
#include "net/HTTPClient.h"
#include "LED.h"
#include "CommandInterface.h"
#include "Utils.h"
#include <Arduino.h>

#define CALIBRATIONITER 1000
#define ORANGEZONE 6

Seismometer::Seismometer() {
	lastEventWas = 0;
	inEvent = 0;
	thresholds = { 0, 0, 0, 0, 0, 0 };
}

void Seismometer::init() {
	Log::i("Initial calibration");

	/* Calibrating Accelerometer */
	accelero.begin(A0, A1, A2);

	// number of samples that have to be averaged
	accelero.setAveraging(10);
	accelero.calibrate();

	Log::d("Calibration ended");
}

void Seismometer::tick() {

	int valx = accelero.getXAccel();
	int valy = accelero.getYAccel();
	int valz = accelero.getZAccel();

	RECORD db = {0, 0, 0, 0, 0, false};

	db.ts = NTP::getUNIXTime();
	db.ms = NTP::getUNIXTimeMS();
	db.valx = getAvgX(valx);
	db.valy = getAvgY(valy);
	db.valz = getAvgZ(valz);
	db.overThreshold = false;


	switch(thresholdAlgorithm) {
		case Fixed:
			db.overThreshold = isOverThresholdFixed(&db, &thresholds);
			break;
		case Basic:
		default:
			db.overThreshold = isOverThresholdBasic(&db, &thresholds);
			break;
	}

	CommandInterface::sendValues(&db);

	// if the values of the accelerometer have passed the threshold
	//  or if an "event" is currently running
	if (db.overThreshold && !inEvent) {
		Log::i(db.overThreshold ? "overThreshold" : "inEvent");

		LED::red(true);

		inEvent = true;
		lastEventWas = millis();

		HTTPClient::httpSendAlert1(&db, &thresholds);

	} else {
		LED::red(false);
	}
}

// return true if at least one of the axis is over the threshold
bool Seismometer::isOverThresholdBasic(RECORD *db, THRESHOLDS *td) {
	return (db->valx > td->pthresx || db->valx < td->nthresx)
		   || (db->valy > td->pthresy || db->valy < td->nthresy)
		   || (db->valz > td->pthresz || db->valz < td->nthresz);
}

bool Seismometer::isOverThresholdFixed(RECORD *db, THRESHOLDS *td) {
	return (abs(db->valx) > td->pthresx)
		   || (abs(db->valy) > td->pthresy)
		   || (abs(db->valz - GFORCE) > td->pthresz);
}

bool Seismometer::isInEvent() {
	return inEvent;
}

void Seismometer::calibrateForHour(int currentHour) {
	int cbufx[CALIBRATIONITER];
	int cbufy[CALIBRATIONITER];
	int cbufz[CALIBRATIONITER];

	Log::d("Begin calibration for hour: %i", currentHour);

	int i = 0;
	for (i = 0; i < CALIBRATIONITER; i++) {
		cbufx[i] = accelero.getXAccel();
		cbufy[i] = accelero.getYAccel();
		cbufz[i] = accelero.getZAccel();
	}

	float avgx = Utils::absavg(cbufx, CALIBRATIONITER);
	float avgy = Utils::absavg(cbufy, CALIBRATIONITER);
	float avgz = Utils::absavg(cbufz, CALIBRATIONITER);
	double sdevx = Utils::stddev(cbufx, CALIBRATIONITER, avgx);
	double sdevy = Utils::stddev(cbufy, CALIBRATIONITER, avgy);
	double sdevz = Utils::stddev(cbufz, CALIBRATIONITER, avgz);

	thresholds.pthresx = avgx + (sdevx + ORANGEZONE);
	thresholds.pthresy = avgy + (sdevy + ORANGEZONE);
	thresholds.pthresz = avgz + (sdevz + ORANGEZONE);

	thresholds.nthresx = avgx - (sdevx + ORANGEZONE);
	thresholds.nthresy = avgy - (sdevy + ORANGEZONE);
	thresholds.nthresz = avgz - (sdevz + ORANGEZONE);
}

void Seismometer::calibrateIfNeeded() {
	this->calibrateIfNeeded(false);
}

void Seismometer::calibrateIfNeeded(bool force) {
	// do calibration every random amount of hours? or if it's the first time ever
	int currentHour = NTP::getHour();
	if ((nextHour == currentHour) || (thresholds.pthresx <= 0.00) || force) {
		if (nextHour == currentHour) {
			Log::i("nextHour = currentHour on SD #-#-#-#-#-#-#-#-#-#: %i", currentHour);
		}

		if (thresholds.pthresx <= 0) {
			Log::i("pthresx <= 0 on NOSD #-#-#-#-#-#-#-#-#-#");
		}

		Log::d("WRITE THRESHOLDS on SD #-#-#-#-#-#-#-#-#-#");
		calibrateForHour(currentHour);
		nextHour = ((currentHour + 1) % 24);
	}
}

void Seismometer::showThresholdValues() {
	Log::i("Calibration on SD ended");
	Log::i("Positive thresholds X:%lf Y:%lf Z:%lf", thresholds.pthresx, thresholds.pthresy, thresholds.pthresz);
	Log::i("Negative thresholds X:%lf Y:%lf Z:%lf", thresholds.nthresx, thresholds.nthresy, thresholds.nthresz);
}
