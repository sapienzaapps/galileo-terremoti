//
// Created by enrico on 12/03/17.
//

#ifndef GALILEO_TERREMOTI_SCSAPI_H
#define GALILEO_TERREMOTI_SCSAPI_H

#include <string>
#include "../common.h"

class SCSAPI {
public:

	virtual bool init() = 0;

	/**
	 * Alive
	 */
	virtual bool alive() = 0;

	/**
	 * Send alert to server
	 * @param db Accelerometer values
	 * @param threshold Threshold value
	 */
	virtual bool terremoto(RECORD *db) = 0;

	/**
	 * Packet processor
	 */
	virtual void tick() = 0;

	/**
	 * Request time sync
	 */
	virtual bool requestTimeUpdate() = 0;

	/**
	 * Ping
	 * @return
	 */
	virtual bool ping() = 0;

};


#endif //GALILEO_TERREMOTI_SCSAPI_H
