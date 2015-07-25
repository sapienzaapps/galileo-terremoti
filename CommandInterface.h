//
// Created by ebassetti on 23/07/15.
//

#ifndef GALILEO_TERREMOTI_COMMANDINTERFACE_H
#define GALILEO_TERREMOTI_COMMANDINTERFACE_H

#include "Seismometer.h"

#define CONTROLPKTSIZE 48 //36

class CommandInterface {
public:
	static void checkCommandPacket();
	static void sendValues(RECORD *db);
	static void commandInterfaceInit();
};


#endif //GALILEO_TERREMOTI_COMMANDINTERFACE_H
