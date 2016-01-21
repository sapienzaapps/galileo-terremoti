
#ifndef GALILEO_TERREMOTI_NET_COLLECTOR
#define GALILEO_TERREMOTI_NET_COLLECTOR

#include <semaphore.h>
#include <pthread.h>
#include <deque>
#include "IPaddr.h"

typedef struct {
	uint64_t ts;
	float x;
	float y;
	float z;
} CollectorRecord;

class Collector {
public:
	static Collector* getInstance();

	void send(float, float, float);
	void start(IPaddr);
private:
	static Collector *instance;

	Collector();

	bool threadStarted;
	pthread_t worker;
	IPaddr remoteaddr;
	sem_t *prodcons;
	std::deque<CollectorRecord> pdv;
};

#endif
