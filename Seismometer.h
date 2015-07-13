//
// Created by enrico on 12/07/15.
//

#ifndef GALILEO_TERREMOTI_SEISMOMETER_H
#define GALILEO_TERREMOTI_SEISMOMETER_H

#include "AcceleroMMA7361.h"

struct RECORD {
	unsigned long ts;
	unsigned long ms;
	long valx;
	long valy;
	long valz;
	bool overThreshold;
};

struct TDEF {
	double pthresx;
	double pthresy;
	double pthresz;
	double nthresx;
	double nthresy;
	double nthresz;
};

class Seismometer {
public:
	static void init();
	static void tick();
	static bool isInEvent();

private:
	static AcceleroMMA7361 accelero;
	static TDEF thresholds;
	static bool inEvent;
	static unsigned long lastEventWas;

	static bool isOverThresholdBasic(struct RECORD *db, struct TDEF *td);
	static bool isOverThresholdFixed(struct RECORD *db, struct TDEF *td);
};


#endif //GALILEO_TERREMOTI_SEISMOMETER_H
