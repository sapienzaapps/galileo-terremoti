
#ifndef __LCDMONITOR_H
#define __LCDMONITOR_H

#include <SDL2/SDL.h>
#include <pthread.h>

#define LCDMONITOR_WIDTH   320
#define LCDMONITOR_HEIGHT  240

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
	static float olddwval;
};

#endif
