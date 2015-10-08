//
// Created by ebassetti on 16/09/15.
//

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

unsigned long Watchdog::lastBeat = 0;

void Watchdog::launch() {
	pid_t pid, sid;
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

	unlink(WATCHDOG_LOG_PATH);
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
		pid_t sketchpid = Watchdog::getSketchPid();
		struct stat fileinfo;
		stat(WATCHDOG_FILE, &fileinfo);

		bool heartbeat = true;
#ifdef __linux__
		if(fileinfo.st_mtim.tv_sec < time(NULL) - WATCHDOG_TIMER/1000) {
			heartbeat = false;
		}
#else
#if defined(OPENBSD) || defined(FREEBSD) ||defined(__APPLE__) || defined(__darwin__)
		if(fileinfo.st_mtimespec.tv_sec < time(NULL) - 15) {
			heartbeat = false;
		}
#else
#error No definition for file modify time (watchdog)
#endif
#endif
		if (sketchpid == 0 || !heartbeat) {
			std::string reason = "";
			if(!heartbeat) {
				Log::d("Heartbeat missed");
				reason.append("Heartbeat missed;");
			}
			if(sketchpid == 0) {
				Log::d("Unable to get sketch PID");
				reason.append("Unable to get sketch PID");
			}
			Log::i("Sketch is not running");
			Log::close();
			storeCrashInfos(reason);
			platformReboot();
		}
	}
#pragma clang diagnostic pop
}

pid_t Watchdog::getSketchPid() {
	pid_t ret = 0;
	struct dirent *dir;

	DIR *d = opendir("/proc/");
	if(d) {
		while((dir = readdir(d)) != NULL && ret == 0) {
			if(dir->d_type == DT_DIR) {
				pid_t temppid = atoi(dir->d_name);
				if(temppid == 0) continue;
				char path[1024];
				snprintf(path, 1024, "/proc/%i/cmdline", temppid);
				if(Utils::fileExists(path)) {
					std::string cmdline = Utils::readFirstLine(path);
					if(strncmp("/sketch/sketch.elf", cmdline.c_str(), strlen("/sketch/sketch.elf")) == 0 && getpid() != temppid) {
						ret = temppid;
					}
				}
			}

		}
		closedir(d);
	}

	return ret;
}

void Watchdog::heartBeat() {
	if(Utils::millis() - lastBeat > WATCHDOG_TIMER/3) {
		FILE* fp = fopen(WATCHDOG_FILE, "w");
		char buf[10];
		memset(buf, 0xFF, 9);
		buf[9] = 0;
		fwrite(buf, 1, 10, fp);
		fflush(fp);
		fclose(fp);
		lastBeat = Utils::millis();
	}
}

void Watchdog::storeCrashInfos(std::string reason) {

	reason = "reason:" + reason + "\n";

	std::string macstr = "mac:" + Utils::getInterfaceMAC() + "\n";
	std::string unixtime = "time:" + Utils::toString(time(NULL)) + "\n";
	std::string freemem = "freeram:" + Utils::toString(Utils::getFreeRam()) + "\n";
	std::string uptime = "uptime:" + Utils::toString(Utils::uptime()) + "\n";
	std::string platform = "platform:" + std::string(PLATFORM_TAG) + "\n";
	std::string softwareversion = "softwareversion:" + std::string(SOFTWARE_VERSION) + "\n";

	std::string path = WATCHDOG_CRASHDIR;
	path.append(Utils::toString(time(NULL)));
	path.append(".dat");

	mkdir(WATCHDOG_CRASHDIR, 0644);

	FILE* fp = fopen(path.c_str(), "wb");
	fwrite(macstr.c_str(), macstr.length(), 1, fp);
	fwrite(unixtime.c_str(), unixtime.length(), 1, fp);
	fwrite(freemem.c_str(), freemem.length(), 1, fp);
	fwrite(uptime.c_str(), uptime.length(), 1, fp);
	fwrite(platform.c_str(), platform.length(), 1, fp);
	fwrite(softwareversion.c_str(), softwareversion.length(), 1, fp);
	fwrite(reason.c_str(), reason.length(), 1, fp);

	if(Utils::fileExists(STACKTRACEINFO)) {
		fwrite("Crash infos:\n", 13, 1, fp);
		char buf[1024];
		memset(buf, 0, 1024);

		FILE* crashfd = fopen(STACKTRACEINFO, "r");
		fread(buf, 1024, 1, crashfd);
		fwrite(buf, strlen(buf), 1, fp);
		fclose(crashfd);
		unlink(STACKTRACEINFO);
	}

	if(Utils::fileExists("/media/realroot/core")) {
		size_t fileSize = Utils::fileSize("/media/realroot/core");
		fwrite("Core dump:\n", 11, 1, fp);
		uint8_t *buf = (uint8_t *)malloc(fileSize);
		memset(buf, 0, fileSize);

		FILE* corefd = fopen("/media/realroot/core", "rb");
		size_t c = fread(buf, 1, fileSize, corefd);
		fwrite(buf, 1, c, fp);
		fclose(corefd);

		free(buf);

		unlink("/media/realroot/core");
	}

	fclose(fp);
}
