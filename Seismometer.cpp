//
// Created by enrico on 12/07/15.
//

#include "common.h"
#include "Seismometer.h"
#include "Log.h"
#include "net/NTP.h"
#include "net/HTTPClient.h"
#include "LED.h"
#include "Utils.h"
#include "generic.h"
#include "net/TraceAccumulator.h"
#include "LCDMonitor.h"

Seismometer *Seismometer::instance = NULL;

Seismometer::Seismometer() {
	lastEventWas = 0;
	inEvent = false;
	accelero = getAccelerometer();
}

void Seismometer::init() {
	if (accelero == NULL) {
		Log::e("Accelerometer is NULL");
	}
}

void Seismometer::tick() {
	if (accelero == NULL) {
		return;
	}

	double detectionAVG = getCurrentAVG();
	double detectionStdDev = getCurrentSTDDEV();

	RECORD db = {0, 0, false};

	db.ts = NTP::getUNIXTime();
	db.accel = accelero->getTotalVector();
	db.overThreshold = db.accel > quakeThreshold;

	TraceAccumulator::traceValue(db.ts, db.accel, quakeThreshold, getCurrentAVG(), getCurrentSTDDEV(), getSigmaIter());
	addValueToAvgVar(db.accel);

	statLastCounter++;
	if (Utils::millis() - statLastCounterTime > 1000) {
		statProbeSpeed = statLastCounter;
		statLastCounter = 0;
		statLastCounterTime = Utils::millis();
	}

	if (inEvent && Utils::millis() - lastEventWas >= 5000) {
		// Out of event
		LED::red(false);
		inEvent = false;
	} else if (inEvent && Utils::millis() - lastEventWas < 5000) {
		// In event, skipping detections for 5 seconds
		return;
	}

#ifdef SDL_DEMO
	LCDMonitor::sendNewValue((float)db.accel);
#endif

	// if the values of the accelerometer have passed the threshold
	//  or if an "event" is currently running
	if (db.overThreshold && !inEvent) {
		Log::i("New Event: v:%lf - thr:%f - iter:%f - avg:%f - stddev:%f", db.accel, quakeThreshold,
			   getSigmaIter(), detectionAVG, detectionStdDev);

		LED::red(true);

		inEvent = true;
		lastEventWas = Utils::millis();

		HTTPClient::httpSendAlert(&db);
	}
}

Seismometer *Seismometer::getInstance() {
	if (Seismometer::instance == NULL) {
		Seismometer::instance = new Seismometer();
	}
	return Seismometer::instance;
}

std::string Seismometer::getAccelerometerName() {
	return accelero->getAccelerometerName();
}

unsigned int Seismometer::getStatProbeSpeed() {
	return statProbeSpeed;
}

double Seismometer::getQuakeThreshold() {
	return quakeThreshold;
}

void Seismometer::setSigmaIter(double i) {
	this->sigmaIter = i;
}

void Seismometer::addValueToAvgVar(double val) {
	elements++;
	// https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
	double delta = val - partialAvg;
	partialAvg += delta / elements;
	partialStdDev += delta * (val - partialAvg);
	if (elements > 1) {
		quakeThreshold = partialAvg + (getCurrentSTDDEV() * getSigmaIter());
	}
	//Log::d("AddValueToAvgVar: EL:%f D:%f AVG:%f VAR:%f THR:%f I:%i", val, delta, getCurrentAVG(), getCurrentSTDDEV(),
	//	   quakeThreshold, elements);
}

void Seismometer::resetLastPeriod() {
	partialAvg = 0;
	partialStdDev = 0;
	elements = 0;
}

double Seismometer::getSigmaIter() {
	return sigmaIter;
}

double Seismometer::getCurrentAVG() {
	return partialAvg;
}

double Seismometer::getCurrentSTDDEV() {
	return sqrt(partialStdDev / (elements - 1));
}