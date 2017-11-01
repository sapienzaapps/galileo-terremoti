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
	 * @return The config as string (as returned to server)
	 */
	virtual void alive() = 0;

	/**
	 * Send alert to server
	 * @param db Accelerometer values
	 * @param threshold Threshold value
	 */
	virtual void terremoto(RECORD *db) = 0;

	/**
	 * Packet processor
	 */
	virtual void tick() = 0;

	/**
	 * Get time as UNIX timestamp (eg. seconds since Jan 1 1970 UTC)
	 * @return UNIX time
	 */
	virtual unsigned long getUNIXTime() = 0;

	/**
	 * Request time sync
	 */
	virtual void requestTimeUpdate() = 0;

	/**
	 * Ping
	 * @return
	 */
	virtual bool ping() = 0;

};


#endif //GALILEO_TERREMOTI_SCSAPI_H
