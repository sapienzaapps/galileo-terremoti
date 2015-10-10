//
// Created by ebassetti on 21/08/15.
//

#ifndef GALILEO_TERREMOTI_ACCELEROMETER_H
#define GALILEO_TERREMOTI_ACCELEROMETER_H

#include <string>

/**
 * Accelerometer base class
 */
class Accelerometer {
public:
	/**
	 * Get accelerometer X value
	 * @return X value
	 */
	virtual long getXAccel() = 0;

	/**
	 * Get accelerometer Y value
	 * @return Y value
	 */
	virtual long getYAccel() = 0;

	/**
	 * Get accelerometer Z value
	 * @return Z value
	 */
	virtual long getZAccel() = 0;

	/**
	 * Get accelerometer name
	 * @return Accelerometer name
	 */
	virtual std::string getAccelerometerName() = 0;
};

#endif //GALILEO_TERREMOTI_ACCELEROMETER_H
