//
// Created by enrico on 12/03/17.
//

#include "SCSAPI_LocalTest.h"
#include "../generic.h"
#include "../Log.h"

SCSAPI_LocalTest::SCSAPI_LocalTest() {
}

bool SCSAPI_LocalTest::init() {
	Log::d("API-TEST: INIT");
	return true;
}

bool SCSAPI_LocalTest::alive() {
	Log::d("API-TEST: ALIVE");
	return true;
}

void SCSAPI_LocalTest::tick() {
}

bool SCSAPI_LocalTest::requestTimeUpdate() {
	Log::d("API-TEST: REQ TIME");
	execSystemTimeUpdate(1510724683);
	return true;
}

bool SCSAPI_LocalTest::terremoto(RECORD *db) {
	return true;
}

bool SCSAPI_LocalTest::ping() {
	return true;
}

SCSAPI_LocalTest::~SCSAPI_LocalTest() {
}
