//
// Created by enrico on 12/03/17.
//

#include "SCSAPI_LocalTest.h"
#include "../generic.h"
#include "../Log.h"
#include <time.h>

unsigned int randbetween(int min, int max) {
	return (unsigned)(min + ((rand()/RAND_MAX)*max));
}

SCSAPI_LocalTest::SCSAPI_LocalTest() {
}

bool SCSAPI_LocalTest::init() {
	Log::d("API-TEST: INIT");
	sleep(randbetween(0, 2));
	return true;
}

bool SCSAPI_LocalTest::alive() {
	Log::d("API-TEST: ALIVE");
	sleep(randbetween(0, 2));
	return true;
}

void SCSAPI_LocalTest::tick() {
}

bool SCSAPI_LocalTest::requestTimeUpdate() {
	Log::d("API-TEST: REQ TIME");
	sleep(randbetween(0, 2));
	execSystemTimeUpdate(time(NULL));
	return true;
}

bool SCSAPI_LocalTest::terremoto(RECORD *db) {
	return true;
}

bool SCSAPI_LocalTest::ping() {
	sleep(randbetween(0, 2));
	return true;
}

SCSAPI_LocalTest::~SCSAPI_LocalTest() {
}
