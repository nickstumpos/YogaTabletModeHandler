//============================================================================
// Name        : YogaKeyboardfoldedHandler.cpp
// Author      : NIck
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <cstdio>
#include <cerrno>
#include <cwchar>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <csignal>
#include <error.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <linux/input.h>
#include <iostream>
#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/Xatom.h>
using namespace std;

//logic stolen from blatenly stolen from https://code.google.com/p/logkeys/ and slightly modified START
#ifndef  EXE_GREP
# define EXE_GREP "/bin/grep"
#endif
#define CODE_TO_FIND   4
#define VAL_TO_FIND  190
#define TYP_TO_FIND 4
#define DIFF_FOR_FOUND 4
#define COMMAND_STR_DEVICES  ( EXE_GREP " -E 'Handlers|EV=' /proc/bus/input/devices | " EXE_GREP " -B1 'EV=120013' | " EXE_GREP " -Eo 'event[0-9]+' ")

string execute(const char* cmd) {
	FILE* pipe = popen(cmd, "r");
	if (!pipe)
		error(EXIT_FAILURE, errno, "Pipe error");
	char buffer[128];
	string result = "";
	while (!feof(pipe))
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	pclose(pipe);
	return result;
}

string determine_input_device() {
	// better be safe than sory: while running other programs, switch user to nobody
	setegid(65534);
	seteuid(65534);

	// extract input number from /proc/bus/input/devices (I don't know how to do it better. If you have an idea, please let me know.)
	stringstream output(execute(COMMAND_STR_DEVICES));

	vector<string> results;
	string line;

	while (getline(output, line)) {
		string::size_type i = line.find("event");
		if (i != string::npos)
			i += 5; // "event".size() == 5
		if (i < line.size()) {
			int index = atoi(&line.c_str()[i]);

			if (index != -1) {
				stringstream input_dev_path;
				input_dev_path << "/dev/input/event";
				input_dev_path << index;
				results.push_back(input_dev_path.str());
			}
		}
	}

	if (results.size() == 0) {
		error(EXIT_FAILURE, 0, "Couldn't find Yoga Keyboard");
	}

	// now we reclaim those root privileges
	seteuid(0);
	setegid(0);
	return results[0];
}

//logic stolen from blatenly stolen from https://code.google.com/p/logkeys/ and slightly modified END
bool isInLaptopMode = true; //we dont know at start so assume  and correct on first keystroke
static time_t last_found = 0;
pthread_t outOfLaptopMaodeClosure;
int threadCount;
long getDevice() {
	long ret = None;
	Display *display = XOpenDisplay(NULL);
	int num_devices;

	XDeviceInfo *start;

	start = XListInputDevices(display, &num_devices);
	for (int i = 0; i < num_devices; i++) {
		XDeviceInfo *info = start + i;
		if (info->type != None) {
			char *type = XGetAtomName(display, info->type);
			//there has to be a better way to do this
			if (!strncmp("TOUCHPAD", type, 8)) {

				if (!strncmp("SynPS/2 Synaptics TouchPad", info->name, 26)) {
					ret = info->id;
					break;
				}
			}

			XFree(type);
		}

	}
	XCloseDisplay(display);
	return ret;
}
void changeEnabledDeviceState(long id, char * val) {
	Display *display = XOpenDisplay(NULL);
	XDevice *dev = XOpenDevice(display, id);
	Atom deviceEnableProperty = XInternAtom(display, "Device Enabled", 0);
	unsigned char *c =
			reinterpret_cast<unsigned char *>(calloc(1, sizeof(long)));
	c[0] = atoi(val);
	XChangeDeviceProperty(display, dev, deviceEnableProperty, XA_INTEGER, 8,
	PropModeReplace, c, 1);
	XSync(display, 0);
	free(c);
	XCloseDevice(display, dev);
	XCloseDisplay(display);

}
void stopDevice(long id) {
	printf("stopping device %ld\n", id);
	changeEnabledDeviceState(id, (char *)"0");
}
void startDevice(long id) {
	printf("starting device %ld\n", id);
	changeEnabledDeviceState(id, (char *)"1");
}
void *runCheckClosed(void * devId) {

	double t = difftime(time(NULL), last_found);
	while (t < 1) {
		t = difftime(time(NULL), last_found);
		sleep(1);
	}
	printf("laptopMode Starting\n");
	//todo thread safty
	isInLaptopMode = true;
	startDevice((long) devId);
	pthread_exit(NULL);
}
int main() {
	if (geteuid()) {
		error(EXIT_FAILURE, errno, "Need to run as root");
	}
	int yogaKeyboardFD = open(determine_input_device().c_str(), O_RDONLY);
	if (yogaKeyboardFD == -1) {
		error(EXIT_FAILURE, errno, "Error opening keyboard input");
	}
	long deviceID = getDevice();
	struct input_event event;
	while (read(yogaKeyboardFD, &event, sizeof(struct input_event)) > 0) {

		if (TYP_TO_FIND == event.type && CODE_TO_FIND == event.code
				&& VAL_TO_FIND == event.value) {
			time(&last_found);
			if (isInLaptopMode) {
				printf("NonLaptop mode Starting\n");
				stopDevice(deviceID);
				isInLaptopMode = false;
				threadCount++;
				if (pthread_create(&outOfLaptopMaodeClosure, NULL,
						runCheckClosed, (void*) deviceID)) {
					printf("Error creating thread");
					exit(-1);
				}
			}
		}

	}
	return 0;
}

