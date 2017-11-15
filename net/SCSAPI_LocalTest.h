//
// Created by enrico on 12/03/17.
//

#ifndef GALILEO_TERREMOTI_SCSAPI_MQTT_H
#define GALILEO_TERREMOTI_SCSAPI_MQTT_H

#include "SCSAPI.h"

class SCSAPI_LocalTest : public SCSAPI {
public:

	SCSAPI_LocalTest();
	~SCSAPI_LocalTest();

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


#endif //GALILEO_TERREMOTI_SCSAPI_H
