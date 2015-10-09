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
#include "generic.h"

#define CALIBRATIONITER 1000
#define ORANGEZONE 6

Seismometer* Seismometer::instance = NULL;

Seismometer::Seismometer() {
	lastEventWas = 0;
	inEvent = false;
	thresholds = { 0, 0, 0, 0, 0, 0 };
	accelero = getAccelerometer();
	thresholdHour = 30; // invalid hour, so loadThresholdIfNeeded is forced to load values from storage
	if(accelero == NULL) {
		Log::e("Accelerometer object is NULL");
	}
	thresholdAlgorithm = Basic;

#ifdef SAVE_THRESHOLD
	createDBifNeeded();
#endif
}

void Seismometer::init() {
	if(accelero == NULL) {
		Log::e("Accelerometer is NULL");
		return;
	}
}

void Seismometer::tick() {

	if(accelero == NULL) {
		return;
	}

	// Let's read and throw away (to empty read queue if needed)
	int valx = accelero->getXAccel();
	int valy = accelero->getYAccel();
	int valz = accelero->getZAccel();

	// Skipping detections for 5 seconds
	if(inEvent && Utils::millis()-lastEventWas >= 5000) {
		LED::red(false);
		inEvent = false;
	} else if(inEvent && Utils::millis()-lastEventWas < 5000) {
		return;
	}

	RECORD db = {0, 0, 0, 0, 0, false};

	db.ts = NTP::getUNIXTime();
	db.ms = NTP::getUNIXTimeMS();
	db.valx = getAvgX(valx);
	db.valy = getAvgY(valy);
	db.valz = getAvgZ(valz);
	db.overThreshold = false;

#ifdef SAVE_THRESHOLD
	// Load threshold if needed
	loadThresholdIfNeeded();
#endif

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
		lastEventWas = Utils::millis();

		HTTPClient::httpSendAlert1(&db, &thresholds);
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

void Seismometer::calibrateForHour(HOUR currentHour) {

	if(accelero == NULL) {
		return;
	}

	LED::setLedBlinking(LED_YELLOW_PIN);

	int cbufx[CALIBRATIONITER];
	int cbufy[CALIBRATIONITER];
	int cbufz[CALIBRATIONITER];

	Log::d("Begin calibration for hour: %i", currentHour);

	int i = 0;
	for (i = 0; i < CALIBRATIONITER; i++) {
		cbufx[i] = accelero->getXAccel();
		cbufy[i] = accelero->getYAccel();
		cbufz[i] = accelero->getZAccel();
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

	LED::clearLedBlinking();
}

void Seismometer::calibrateIfNeeded() {
	this->calibrate(false);
}

void Seismometer::calibrate(bool force) {
	// do calibration every random amount of hours? or if it's the first time ever

#ifdef SAVE_THRESHOLD
	loadThresholdIfNeeded();
#endif

	HOUR currentHour = (HOUR)NTP::getHour();
	if ((nextHour == currentHour) || (thresholds.pthresx <= 0.00) || force) {
		if (nextHour == currentHour) {
			Log::i("Calibration started due to time constraint: hour %i", currentHour);
		} else if (thresholds.pthresx <= 0) {
			Log::i("Calibration started due to invalid threshold values");
		} else {
			Log::i("Calibration force start");
		}

		calibrateForHour(currentHour);
		Log::i("Calibration ended");
		logThresholdValues();

#ifdef SAVE_THRESHOLD
		saveCalibration(currentHour);
#endif

		nextHour = ((currentHour + (HOUR)1) % (HOUR)24);
	}
}

void Seismometer::logThresholdValues() {
	Log::i("Positive thresholds X:%lf Y:%lf Z:%lf", thresholds.pthresx, thresholds.pthresy, thresholds.pthresz);
	Log::i("Negative thresholds X:%lf Y:%lf Z:%lf", thresholds.nthresx, thresholds.nthresy, thresholds.nthresz);
}

#ifdef SAVE_THRESHOLD
// TODO: save timestamp to recalibrate every X days or random
void Seismometer::saveCalibration(HOUR currentHour) {
	FILE *fp = fopen(CALIBRATION_FILE, "rb+");
	if(fp == NULL) {
		Log::e("Error during calibration database opening on save routine");
		return;
	}
	fseek(fp, currentHour * 6 * sizeof(double), SEEK_SET);

	fwrite(&thresholds.pthresx, sizeof(double), 1, fp);
	fwrite(&thresholds.pthresy, sizeof(double), 1, fp);
	fwrite(&thresholds.pthresz, sizeof(double), 1, fp);
	fwrite(&thresholds.nthresx, sizeof(double), 1, fp);
	fwrite(&thresholds.nthresy, sizeof(double), 1, fp);
	fwrite(&thresholds.nthresz, sizeof(double), 1, fp);

	Log::d("Threshold values saved");

	fflush(fp);
	fclose(fp);
}

void Seismometer::loadThresholdIfNeeded() {
	HOUR currentHour = (HOUR)NTP::getHour();
	if(thresholdHour != currentHour) {
		FILE *fp = fopen(CALIBRATION_FILE, "rb");
		if(fp == NULL) {
			Log::e("Error during calibration database opening on read routine");
			return;
		}
		fseek(fp, currentHour * 6 * sizeof(double), SEEK_SET);

		fread(&thresholds.pthresx, sizeof(double), 1, fp);
		fread(&thresholds.pthresy, sizeof(double), 1, fp);
		fread(&thresholds.pthresz, sizeof(double), 1, fp);
		fread(&thresholds.nthresx, sizeof(double), 1, fp);
		fread(&thresholds.nthresy, sizeof(double), 1, fp);
		fread(&thresholds.nthresz, sizeof(double), 1, fp);

		logThresholdValues();

		fclose(fp);
		thresholdHour = currentHour;
	}
}

void Seismometer::createDBifNeeded() {
	FILE *fp = fopen(CALIBRATION_FILE, "r");
	if(fp == NULL) {
		fp = fopen(CALIBRATION_FILE, "wb");
		if(fp == NULL) {
			Log::e("Cannot create calibration database file");
			return;
		}
		size_t rowSize = 6 * sizeof(double);
		uint8_t row[rowSize];
		memset(row, 0, rowSize);
		for(int i = 0; i < 24; i++) {
			fwrite(&row, rowSize, 1, fp);
		}
	}
	fclose(fp);
}
#endif

THRESHOLDS Seismometer::getThresholds(){
 return thresholds;
}

Seismometer* Seismometer::getInstance() {
	if(Seismometer::instance == NULL) {
		Seismometer::instance = new Seismometer();
	}
	return Seismometer::instance;
}

std::string Seismometer::getAccelerometerName() {
	return accelero->getAccelerometerName();
}
