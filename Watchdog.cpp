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

unsigned long Watchdog::lastBeat = 0;

void Watchdog::launch() {
	pid_t pid, sid;
	int fd;

	// already a daemon
	if ( getppid() == 1 ) return;

	// Fork off the parent process
	pid = fork();
	if (pid < 0) {
		system("reboot");
		exit(EXIT_FAILURE);
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
#ifdef __LINUX__
		if(fileinfo.st_mtim.tv_sec < time(NULL) - 15) {
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
			if(!heartbeat) {
				Log::d("Heartbeat missed");
			}
			if(sketchpid == 0) {
				Log::d("Unable to get sketch PID");
			}
			Log::i("Sketch is not running");
			Log::close();
			system(REBOOT_CMD);
			exit(EXIT_SUCCESS);
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
	if(Utils::millis() - lastBeat > 5000) {
		FILE* fp = fopen(WATCHDOG_FILE, "w");
		char buf[10];
		memset(buf, 0xFF, 10);
		fwrite(buf, 10, 1, fp);
		fflush(fp);
		fclose(fp);
		lastBeat = Utils::millis();
	}
}
