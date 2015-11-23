//
// Created by ebassetti on 21/08/15.
//

#ifndef GALILEO_TERREMOTI_DUMMYACCELEROMETER_H
#define GALILEO_TERREMOTI_DUMMYACCELEROMETER_H


#include "../../Accelerometer.h"

class DummyAccelerometer : public Accelerometer {
public:
	DummyAccelerometer();
	virtual double getXAccel();
	virtual double getYAccel();
	virtual double getZAccel();
	virtual std::string getAccelerometerName();
};


#endif //GALILEO_TERREMOTI_DUMMYACCELEROMETER_H
