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
	bool alive() override;

	/**
	 * Send alert to server
	 * @param db Accelerometer values
	 * @param threshold Threshold value
	 */
	bool terremoto(RECORD *db) override;

	/**
	 * Packet processor
	 */
	void tick() override;

	/**
	 * Request time sync
	 */
	bool requestTimeUpdate() override;

	bool ping() override;
};


#endif //GALILEO_TERREMOTI_SCS_HTTP_H
