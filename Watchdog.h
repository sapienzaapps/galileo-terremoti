//
// Created by ebassetti on 16/09/15.
//

#ifndef GALILEO_TERREMOTI_WATCHDOG_H
#define GALILEO_TERREMOTI_WATCHDOG_H


class Watchdog {
public:
	static void launch();
	static void heartBeat();

private:
	static unsigned long lastBeat;
	static pid_t getSketchPid();
};


#endif //GALILEO_TERREMOTI_WATCHDOG_H
