
#include "Collector.h"
#include "NTP.h"
#include "../Log.h"
#include "../Utils.h"
#include "Tcp.h"
#include <string.h>

Collector::Collector() {
	this->remoteaddr.setInt(0);
	sem_init(&(this->prodcons), 0, 0);
}

Collector *Collector::getInstance() {
	if(Collector::instance == NULL) {
		Collector::instance = new Collector();
	}
	return Collector::instance;
}

void Collector::send(float x, float y, float z) {
	if(this->remoteaddr != 0) {
		// Add to queue
		CollectorRecord cr;
		cr.ts = NTP::getUNIXTimeMS();
		cr.x = x;
		cr.y = y;
		cr.z = z;

		pdv.push_back(cr);
		sem_post(&prodcons);
	}
}

void Collector::start(IPaddr paddr) {
	this->remoteaddr = paddr;
	int rc = pthread_create(&(this->worker), NULL, threadWorker, NULL);
	if(rc) {
		Log::e("Error during Collector thread creation");
	}
}

void *Collector::threadWorker(void *mem) {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
	do {
		Tcp *c = new Tcp();
		if (c->connectTo(this->remoteaddr, 13060)) {
			while(c->connected()) {
				sem_wait(&prodcons);

				CollectorRecord cr = pdv.front();
				pdv.pop_front();

				// Assuming sizeof(long) = 8 and sizeof(float) = 4
				uint8_t buf[8 + 4 + 4 + 4];
				memcpy(buf, &(cr.ts), 8);
				memcpy(buf+8, &(cr.x), 8);
				memcpy(buf+12, &(cr.y), 8);
				memcpy(buf+16, &(cr.z), 8);
				c->send(buf, 8 + 4 + 4 + 4);
			}
		}
		c->stop();
		Utils::delay(60 * 1000);
	} while(true);
#pragma clang diagnostic pop

	pthread_exit(NULL);
}
