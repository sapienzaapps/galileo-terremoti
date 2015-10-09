//
// Created by ebassetti on 21/08/15.
//

#include "../../generic.h"
#include "AcceleroMMA7361.h"
#include "../../Log.h"
#include <Arduino.h>
#include <trace.h>
#include <interrupt.h>
#include <sys/stat.h>
#include <unistd.h>

#define PLATFORM_NAME_PATH "/sys/devices/platform/"
#define MY_TRACE_PREFIX __FILE__

// Taken from Arduino Galileo SDK
void vendor_init(int argc, char** argv) {
	char *platform_path = NULL;
	struct stat s;
	int err;

	// Install a signal handler

	// make ttyprintk at some point
	stdout = freopen("/tmp/log.txt", "w", stdout);
	if (stdout == NULL){
		fprintf(stderr, "unable to remap stdout !\n");
		exit(-1);
	}
	fflush(stdout);

	stderr = freopen("/tmp/log_er.txt", "w", stderr);
	if (stderr == NULL){
		printf("Unable to remap stderr !\n");
		exit(-1);
	}
	fflush(stderr);

	// Snapshot time counter
	if (timeInit() < 0)
		exit(-1);

	// debug for the user
	if (argc < 2){
		fprintf(stderr, "./sketch tty0\n");
		exit(-1);
	}
	printf("started with binary=%s Serial=%s\n", argv[0], argv[1]);
	fflush(stdout);

	// check if we're running on the correct platform
	// and refuse to run if no match

	platform_path = (char *)malloc(sizeof(PLATFORM_NAME_PATH) + sizeof(PLATFORM_NAME));
	sprintf(platform_path,"%s%s", PLATFORM_NAME_PATH, PLATFORM_NAME);

	printf("checking platform_path [%s]\n", platform_path);
	fflush(stdout);

	err = stat(platform_path, &s);

	if(err != 0) {
		fprintf(stderr, "stat failed checking for %s with error code %d\n", PLATFORM_NAME, err);
		free(platform_path);
		exit(-1);
	}
	if(!S_ISDIR(s.st_mode)) {
		/* exists but is no dir */
		fprintf(stderr, "Target board not a %s\n", PLATFORM_NAME);
		free(platform_path);
		exit(-1);
	}

	printf("Running on a %s platform (%s)\n", PLATFORM_NAME, platform_path);
	fflush(stdout);

	free(platform_path);

	trace_init(VARIANT_TRACE_LEVEL, 0);
	trace_target_enable(TRACE_TARGET_UART);
	trace_target_enable(TRACE_TARGET_SYSLOG);

	init(argc, argv);
	interrupt_init();
}

Accelerometer* getAccelerometer() {
	AcceleroMMA7361* accel = new AcceleroMMA7361();

	/* Calibrating Accelerometer */
	accel->begin(A0, A1, A2);

	Log::i("Initial calibration");
	// number of samples that have to be averaged
	accel->setAveraging(10);
	accel->calibrate();

	Log::d("Calibration ended");

	return accel;
}

std::string getPlatformName() {
#if GALILEO_GEN == 1
	return std::string("Intel Galileo Gen1");
#else
	return std::string("Intel Galileo Gen2");
#endif
}

void platformReboot() {
	while(1) {
		system("reboot");
		sleep(5);
	}
}
