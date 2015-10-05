//
// Created by ebassetti on 16/09/15.
//

#ifndef GALILEO_TERREMOTI_WATCHDOG_H
#define GALILEO_TERREMOTI_WATCHDOG_H

#include <string>

class Watchdog {
public:
	static void launch();
	static void heartBeat();

private:
	static void storeCrashInfos(std::string reason);
	static unsigned long lastBeat;
	static pid_t getSketchPid();
};


#endif //GALILEO_TERREMOTI_WATCHDOG_H
