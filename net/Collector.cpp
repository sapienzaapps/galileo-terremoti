
#include "Collector.h"
#include "NTP.h"
#include "../Log.h"
#include "../Utils.h"
#include "Tcp.h"
#include <string.h>
#include <fcntl.h>

Collector *Collector::instance;

typedef struct {
	IPaddr *remoteaddr;
	sem_t *prodcons;
	std::deque<CollectorRecord> *pdv;
} ThreadArgument;

void *threadWorker(void *mem);

Collector::Collector() {
	this->remoteaddr.setInt(0);
	this->prodcons = sem_open("collector", O_CREAT, 0600, 0);
	this->threadStarted = false;
}

Collector *Collector::getInstance() {
	if(Collector::instance == NULL) {
		Collector::instance = new Collector();
	}
	return Collector::instance;
}

void Collector::send(float x, float y, float z) {
	if(this->remoteaddr.asInt() != 0) {
		// Add to queue
		CollectorRecord cr;
		cr.ts = NTP::getUNIXTimeMS();
		cr.x = x;
		cr.y = y;
		cr.z = z;

		pdv.push_back(cr);
		sem_post(prodcons);
	}
}

void Collector::start(IPaddr paddr) {
	if(this->threadStarted) {
		return;
	}
	this->remoteaddr = paddr;

	ThreadArgument *arg = new ThreadArgument();
	arg->pdv = &(this->pdv);
	arg->prodcons = this->prodcons;
	arg->remoteaddr = &(this->remoteaddr);

	int rc = pthread_create(&(this->worker), NULL, threadWorker, arg);
	if(rc) {
		Log::e("Error during Collector thread creation");
	} else {
		Log::i("Collector started to %s", paddr.asString().c_str());
		this->threadStarted = true;
	}
}

void *threadWorker(void *mem) {
	ThreadArgument *arg = (ThreadArgument*)mem;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
	do {
		Tcp *c = new Tcp();
		if (c->connectTo(*(arg->remoteaddr), 13060)) {
			Log::i("Collector connected successfully");
			while(c->connected()) {
				sem_wait(arg->prodcons);

				CollectorRecord cr = arg->pdv->front();
				arg->pdv->pop_front();

				// Assuming sizeof(long) = 8 and sizeof(float) = 4
				uint8_t buf[8 + 4 + 4 + 4];
				uint64_t ts = cr.ts;
				memcpy(buf, &ts, 8);
				memcpy(buf+8, &(cr.x), 4);
				memcpy(buf+12, &(cr.y), 4);
				memcpy(buf+16, &(cr.z), 4);
				c->send(buf, 8 + 4 + 4 + 4);
			}
		} else {
			Log::e("Error during connection to collector server");
		}
		Log::i("Stopping collector, retry in 60 seconds");
		c->stop();
		Utils::delay(60 * 1000);
	} while(true);
#pragma clang diagnostic pop
	delete arg;
	pthread_exit(NULL);
}
