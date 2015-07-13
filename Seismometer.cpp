//
// Created by enrico on 12/07/15.
//

#include "Seismometer.h"
#include "Log.h"
#include "avg.h"
#include "NTP.h"
#include "HTTPClient.h"
#include "LED.h"

AcceleroMMA7361 Seismometer::accelero;
TDEF Seismometer::thresholds = { 0, 0, 0, 0, 0, 0 };
bool Seismometer::inEvent = false;
unsigned long Seismometer::lastEventWas = 0;

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

	struct RECORD db = {0, 0, 0, 0, 0, false};

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

	sendValues(db);

	// if the values of the accelerometer have passed the threshold
	//  or if an "event" is currently running
	if (db.overThreshold && !inEvent) {
		Log::i(db.overThreshold ? "overThreshold" : "inEvent");

		LED::on(LED_RED);

		inEvent = true;
		lastEventWas = millis();

		HTTPClient::httpSendAlert1(&db, &thresholds);

	} else {
		LED::off(LED_RED);
	}
}

// return true if at least one of the axis is over the threshold
bool Seismometer::isOverThresholdBasic(struct RECORD *db, struct TDEF *td) {
	return (db->valx > td->pthresx || db->valx < td->nthresx)
		   || (db->valy > td->pthresy || db->valy < td->nthresy)
		   || (db->valz > td->pthresz || db->valz < td->nthresz);
}

bool Seismometer::isOverThresholdFixed(struct RECORD *db, struct TDEF *td) {
	return (abs(db->valx) > td->pthresx)
		   || (abs(db->valy) > td->pthresy)
		   || (abs(db->valz - GFORCE) > td->pthresz);
}

bool Seismometer::isInEvent() {
	return inEvent;
}
