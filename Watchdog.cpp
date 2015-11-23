//
// Created by ebassetti on 16/09/15.
//

#ifndef NOWATCHDOG

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <vendor_specific.h>
#include "Watchdog.h"
#include "Utils.h"
#include "Log.h"
#include "generic.h"
#include "Config.h"

unsigned long Watchdog::lastBeat = 0;

void Watchdog::launch() {
	pid_t pid, sid, parentpid = getpid();
	int fd;

	// already a daemon
	if ( getppid() == 1 ) return;

	// Fork off the parent process
	pid = fork();
	if (pid < 0) {
		platformReboot();
	}

	if (pid > 0) {
		// Parent process can proceed
		signal(SIGCHLD, SIG_IGN);
		heartBeat();
		return;
	}

	// At this point we are executing as the intermediate parent process

	// Create a new SID for the intermediate parent process
	sid = setsid();
	if (sid < 0) {
		exit(EXIT_FAILURE);
	}

	signal(SIGCHLD, SIG_IGN);

	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}

	if (pid > 0) {
		// Intermediate parent can be killed
		exit(EXIT_SUCCESS);
	}

	Log::setLogFile(WATCHDOG_LOG_PATH);
	Log::setLogLevel(LEVEL_DEBUG);
	Log::d("Watchdog PID is %i", getpid());

	// Change the current working directory.
	if ((chdir("/")) < 0) {
		exit(EXIT_FAILURE);
	}

	fd = open("/dev/null",O_RDWR, 0);

	if (fd != -1) {
		dup2 (fd, STDIN_FILENO);
		dup2 (fd, STDOUT_FILENO);
		dup2 (fd, STDERR_FILENO);

		if (fd > 2) {
			close (fd);
		}
	}

	// resettign File Creation Mask
	umask(027);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
	while(true) {
		sleep(15);
		bool sketchPidRunning = Watchdog::checkSketchPid(parentpid);
		struct stat fileinfo;

		bool heartbeat = true;
		int rp = stat(WATCHDOG_FILE, &fileinfo);
		if(rp == 0) {

#ifdef __linux__
			if(fileinfo.st_mtim.tv_sec < time(NULL) - WATCHDOG_TIMER/1000) {
				heartbeat = false;
			}
#else
#if defined(OPENBSD) || defined(FREEBSD) || defined(__APPLE__) || defined(__darwin__)
			if (fileinfo.st_mtimespec.tv_sec < time(NULL) - 15) {
				heartbeat = false;
			}
#else
#error No definition for file modify time (watchdog)
#endif
#endif
		} else {
			heartbeat = true;
		}
		if (sketchPidRunning == 0 || !heartbeat) {
			std::string reason = "";
			if(!heartbeat) {
				reason.append("Heartbeat missed;");
			}
			if(sketchPidRunning) {
				reason.append("Sketch is not running");
			}
			Log::i("Reboot reason: %s", reason.c_str());
			Log::close();
#ifdef DEBUG
			storeCrashInfos(reason);
#endif
			platformReboot();
		}
	}
#pragma clang diagnostic pop
}

bool Watchdog::checkSketchPid(pid_t pid) {
	char path[1024];
	snprintf(path, 1024, "/proc/%i/cmdline", pid);
	return Utils::fileExists(path);
}

void Watchdog::heartBeat() {
	if(Utils::millis() - lastBeat > WATCHDOG_TIMER/3) {
		FILE* fp = fopen(WATCHDOG_FILE, "w");
		if(fp == NULL) {
			unlink(WATCHDOG_FILE);
			platformReboot();
			return;
		}
		char buf[10];
		memset(buf, 0xFF, 9);
		buf[9] = 0;
		fwrite(buf, 1, 10, fp);
		fflush(fp);
		fclose(fp);
		lastBeat = Utils::millis();
	}
}

#ifdef DEBUG
void Watchdog::storeCrashInfos(std::string reason) {

	reason = "reason:" + reason + "\n";

	std::string macstr = "mac:" + Config::getMacAddress() + "\n";
	std::string unixtime = "time:" + Utils::toString(time(NULL)) + "\n";
	std::string freemem = "freeram:" + Utils::toString(Utils::getFreeRam()) + "\n";
	std::string uptime = "uptime:" + Utils::toString(Utils::uptime()) + "\n";
	std::string platform = "platform:" + std::string(PLATFORM_TAG) + "\n";
	std::string buildversion = "buildversion:" + std::string(BUILD_VERSION) + "\n";
	std::string softwareversion = "softwareversion:" + std::string(SOFTWARE_VERSION) + "\n";

	std::string path = WATCHDOG_CRASHDIR;
	path.append(Utils::toString(time(NULL)));
	path.append(".dat");

	mkdir(WATCHDOG_CRASHDIR, 0644);

	FILE* fp = fopen(path.c_str(), "wb");
	if(fp == NULL) {
		// Ooops!
		return;
	}
	fwrite(macstr.c_str(), macstr.length(), 1, fp);
	fwrite(unixtime.c_str(), unixtime.length(), 1, fp);
	fwrite(freemem.c_str(), freemem.length(), 1, fp);
	fwrite(uptime.c_str(), uptime.length(), 1, fp);
	fwrite(platform.c_str(), platform.length(), 1, fp);
	fwrite(buildversion.c_str(), buildversion.length(), 1, fp);
	fwrite(softwareversion.c_str(), softwareversion.length(), 1, fp);
	fwrite(reason.c_str(), reason.length(), 1, fp);

	if(Utils::fileExists(STACKTRACEINFO)) {
		fwrite("Crash infos:\n", 13, 1, fp);
		char buf[1024];
		memset(buf, 0, 1024);

		FILE* crashfd = fopen(STACKTRACEINFO, "r");
		if(crashfd != NULL) {
			fread(buf, 1024, 1, crashfd);
			fwrite(buf, strlen(buf), 1, fp);
			fclose(crashfd);
		}
		unlink(STACKTRACEINFO);
	}

	if(Utils::fileExists("/media/realroot/core")) {
		ssize_t fileSize = Utils::fileSize("/media/realroot/core");
		fwrite("Core dump:\n", 11, 1, fp);
		uint8_t *buf = (uint8_t *)malloc(fileSize);
		memset(buf, 0, fileSize);

		FILE* corefd = fopen("/media/realroot/core", "rb");
		if(corefd != NULL) {
			size_t c = fread(buf, 1, fileSize, corefd);
			fwrite(buf, 1, c, fp);
			fclose(corefd);
		}
		free(buf);

		unlink("/media/realroot/core");
	}

	fclose(fp);
}
#endif
#endif