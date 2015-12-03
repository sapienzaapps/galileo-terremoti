
#ifndef __LCDMONITOR_H
#define __LCDMONITOR_H

#include <SDL2/SDL.h>
#include <pthread.h>

#define LCDMONITOR_WIDTH   640
#define LCDMONITOR_HEIGHT  480

class LCDMonitor {
public:
	static LCDMonitor* getInstance();
	static void sendNewValue(float val);
private:
	LCDMonitor();
	~LCDMonitor();

	pthread_t uiThread;

	static void *uiWorker(void* mem);

	static LCDMonitor *singleton;
	static volatile float curval;
};

#endif
