//
// Created by ebassetti on 16/09/15.
//

#ifndef NOWATCHDOG
#ifndef GALILEO_TERREMOTI_WATCHDOG_H
#define GALILEO_TERREMOTI_WATCHDOG_H

#include <string>

/**
 * Watchdog class
 */
class Watchdog {
public:
	/**
	 * Launch watchdog process
	 */
	static void launch();

	/**
	 * Heart beat to signal to watchdog process that main program is alive
	 */
	static void heartBeat();

private:
	static void storeCrashInfos(std::string reason);
	static unsigned long lastBeat;
	static pid_t getSketchPid();
};


#endif //GALILEO_TERREMOTI_WATCHDOG_H
#endif