//
// Created by enrico on 11/1/17.
//

#ifndef GALILEO_TERREMOTI_SCSAPI_HTTP_H
#define GALILEO_TERREMOTI_SCSAPI_HTTP_H


#include "SCSAPI.h"

class SCSAPI_HTTP : public SCSAPI {
public:
	SCSAPI_HTTP();
	~SCSAPI_HTTP();

	bool init() override;

	/**
	 * Alive
	 * @return The config as string (as returned to server)
	 */
	void alive() override;

	/**
	 * Send alert to server
	 * @param db Accelerometer values
	 * @param threshold Threshold value
	 */
	void terremoto(RECORD *db) override;

	/**
	 * Packet processor
	 */
	void tick() override;

	/**
	 * Get time as UNIX timestamp (eg. seconds since Jan 1 1970 UTC)
	 * @return UNIX time
	 */
	unsigned long getUNIXTime() override;

	/**
	 * Request time sync
	 */
	void requestTimeUpdate() override;

	bool ping() override;

private:
	unsigned long lastNTPTime;
	unsigned long lastNTPMillis;

};


#endif //GALILEO_TERREMOTI_SCS_HTTP_H
