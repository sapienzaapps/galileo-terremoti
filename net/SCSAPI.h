//
// Created by enrico on 12/03/17.
//

#ifndef GALILEO_TERREMOTI_SCSAPI_H
#define GALILEO_TERREMOTI_SCSAPI_H

#include <string>
#include <map>
#include "Tcp.h"
#include "../Seismometer.h"
#include "mqttclient.h"

#define API_KEEPALIVE   1
#define API_QUAKE       2
#define API_TIMEREQ     3
#define API_TIMERESP    4
#define API_CFG         5
#define API_DISCONNECT  6
#define API_TEMPERATURE 7
#define API_REBOOT      8

class SCSAPI {
public:

	static bool init();

	/**
	 * Alive
	 * @return The config as string (as returned to server)
	 */
	static void alive();

	/**
	 * Send alert to server
	 * @param db Accelerometer values
	 * @param threshold Threshold value
	 */
	static void terremoto(RECORD *db);

private:

	static MQTT_Client *mqtt;
};


#endif //GALILEO_TERREMOTI_SCSAPI_H
