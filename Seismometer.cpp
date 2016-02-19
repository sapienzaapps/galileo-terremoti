//
// Created by enrico on 12/07/15.
//

#include "common.h"
#include "Seismometer.h"
#include "Log.h"
#include "net/NTP.h"
#include "net/HTTPClient.h"
#include "LED.h"
#include "CommandInterface.h"
#include "Utils.h"
#include "generic.h"

Seismometer* Seismometer::instance = NULL;

Seismometer::Seismometer() {
	lastEventWas = 0;
	inEvent = false;
	accelero = getAccelerometer();
	serverCollector = Collector::getInstance();
}

void Seismometer::init() {
	if(accelero == NULL) {
		Log::e("Accelerometer is NULL");
	}
}

void Seismometer::tick() {
	if(accelero == NULL) {
		return;
	}

	// Let's read and throw away if needed (to empty read queue if needed)
	double valx = accelero->getXAccel();
	double valy = accelero->getYAccel();
	double valz = accelero->getZAccel();

	serverCollector->send((float)valx, (float)valy, (float)valz);
	CommandInterface::sendValues((float)valx, (float)valy, (float)valz);

	statLastCounter++;
	if(Utils::millis() - statLastCounterTime > 1000) {
		statProbeSpeed = statLastCounter;
		statLastCounter = 0;
		statLastCounterTime = Utils::millis();
	}

	if(inEvent && Utils::millis()-lastEventWas >= 5000) {
		// Out of event
		LED::red(false);
		inEvent = false;
	} else if(inEvent && Utils::millis()-lastEventWas < 5000) {
		// In event, skipping detections for 5 seconds
		return;
	}

	RECORD db = {0, 0, false};

	db.ts = NTP::getUNIXTime();
	db.accel = accelero->getTotalVector();
	db.overThreshold = db.accel > quakeThreshold;

	addValueToAvgVar(db.accel);

	// if the values of the accelerometer have passed the threshold
	//  or if an "event" is currently running
	if (db.overThreshold && !inEvent) {
		Log::i("Over threshold: %f > %f", db.accel, quakeThreshold);

		LED::red(true);

		inEvent = true;
		lastEventWas = Utils::millis();

		HTTPClient::httpSendAlert(&db, quakeThreshold);
	}
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

unsigned int Seismometer::getStatProbeSpeed() {
	return statProbeSpeed;
}

double Seismometer::getQuakeThreshold() {
	return quakeThreshold;
}

void Seismometer::setSigmaIter(double i) {
	double newquakeThreshold = getLastPeriodAVG() + (getLastPeriodVAR() * i);
	if(newquakeThreshold != NAN) {
		Log::d("Attempting to set quake threshold to %f (old %f) iter %f elem %i", newquakeThreshold, quakeThreshold, i, elements);
		//this->sigmaIter = i;
		//quakeThreshold = newquakeThreshold;
	}
}

double Seismometer::getLastPeriodAVG() {
	return lastPeriodAvg;
}

double Seismometer::getLastPeriodVAR() {
	return lastPeriodSQM / elements;
}

void Seismometer::addValueToAvgVar(double val) {
	double precAvg = lastPeriodAvg;

	elements++;
	lastPeriodAvg = precAvg + ((val - precAvg)/elements);
	lastPeriodSQM = lastPeriodSQM + ((val - precAvg)*(val - lastPeriodAvg));
}

void Seismometer::resetLastPeriod() {
	lastPeriodAvg = 0;
	lastPeriodSQM = 0;
	elements = 0;
}

double Seismometer::getSigmaIter() {
	return sigmaIter;
}

void Seismometer::firstTimeThresholdCalculation() {
	for(int i=0; i < 200; i++) {
		addValueToAvgVar(accelero->getTotalVector());
		Utils::delay(SEISMOMETER_TICK_INTERVAL);
	}
	setSigmaIter(getSigmaIter());
	Log::d("First time AVG, VAR and Threshold: %lf %lf %lf", getLastPeriodAVG(), getLastPeriodVAR(), quakeThreshold);
}
