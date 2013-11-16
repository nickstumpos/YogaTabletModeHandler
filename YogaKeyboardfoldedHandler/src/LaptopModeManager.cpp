/*
 * LaptopModeManager.cpp
 *
 *  Created on: Nov 13, 2013
 *      Author: nick
 */

#include "LaptopModeManager.h"
#define CODE_TO_FIND   4
#define VAL_TO_FIND  190
#define TYP_TO_FIND 4
#define DIFF_FOR_FOUND 4
//logic stolen from blatenly stolen from https://code.google.com/p/logkeys/ and slightly modified START
#ifndef  EXE_GREP
# define EXE_GREP "/bin/grep"
#endif
#define COMMAND_STR_DEVICES  ( EXE_GREP " -E 'Handlers|EV=' /proc/bus/input/devices | " EXE_GREP " -B1 'EV=120013' | " EXE_GREP " -Eo 'event[0-9]+' ")

using namespace std;

string LaptopModeManager::execute(const char* cmd) {
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

string LaptopModeManager::determine_input_device() {
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
void *runCheckClosed(void * managerToCast) {
	printf("beforecast\n");
	LaptopModeManager * manager = (LaptopModeManager *)managerToCast;
	printf("aftercast\n");
	double t = difftime(time(NULL), manager->getTimeLastFound());
	while (t < 1) {
		t = difftime(time(NULL), manager->getTimeLastFound());
		sleep(1);
	}
	manager->processEnteringLaptopMode();
	pthread_exit(NULL);
}


bool LaptopModeManager::isInLaptopMode() {
	return isLaptopMode;
}
void LaptopModeManager::setIsInLaptop(bool inLaptopMode) {

	printf("insetLaptop\n");
	pthread_mutex_lock(&laptopModeMutex);
	isLaptopMode = inLaptopMode;
	pthread_mutex_unlock(&laptopModeMutex);
	printf("endSetLaptop\n");
}
LaptopModeManager::LaptopModeManager(
		vector<LaptopModeChangedHandler*> *changeHandlers) {
	threadCount=0;
	last_found=0;
	running=false;
	isLaptopMode = true;//we dont know at start so assume  and correct on first keystroke
	handlers = changeHandlers;
	pthread_mutex_init(&laptopModeMutex, NULL);
	pthread_mutex_init(&lastFoundTimeMutex, NULL);
}
time_t LaptopModeManager::getTimeLastFound() {
	return last_found;
}
void LaptopModeManager::setTimeLastFound(time_t time) {
	pthread_mutex_lock(&lastFoundTimeMutex);
	last_found=time;
	pthread_mutex_unlock(&lastFoundTimeMutex);
}
LaptopModeManager::~LaptopModeManager() {
	pthread_mutex_destroy(&laptopModeMutex);
	pthread_mutex_destroy(&lastFoundTimeMutex);
}

int LaptopModeManager::processLeavingLaptopMode() {
	printf("NonLaptop mode Starting\n");
	setIsInLaptop(false);
	threadCount++;
	for(size_t i=0;i<handlers->size();i++){
		handlers->operator [](i)->onLaptopModeStop();
	}
	pthread_t outOfLaptopModeClosure;
	if (pthread_create(&outOfLaptopModeClosure, NULL, runCheckClosed,
			this)) {
		printf("Error creating thread");
		error(EXIT_FAILURE, errno, "Error opening keyboard input");
	}
	return EXIT_SUCCESS;
}
int LaptopModeManager::processEnteringLaptopMode() {
	setIsInLaptop(true);
	for(__u8 i=0;i<handlers->size();i++){
			handlers->at(i)->onLaptopModeStart();
		}
	return EXIT_SUCCESS;
}

int LaptopModeManager::listenForNonLaptopModeKey(int yogaKeyboardFD) {
	struct input_event event;
	while (running
			&& read(yogaKeyboardFD, &event, sizeof(struct input_event)) > 0) {

		if (TYP_TO_FIND == event.type && CODE_TO_FIND == event.code
				&& VAL_TO_FIND == event.value) {
			time(&last_found);
			if (isLaptopMode) {
				processLeavingLaptopMode();
			}
		}

	}
	return EXIT_SUCCESS;
}
int LaptopModeManager::start() {
	running = true;
	int yogaKeyboardFD = open(determine_input_device().c_str(), O_RDONLY);
	if (yogaKeyboardFD == -1) {
		error(EXIT_FAILURE, errno, "Error opening keyboard input");
	}
	return listenForNonLaptopModeKey(yogaKeyboardFD);
}
int LaptopModeManager::stop() {
	running = false;
	return EXIT_SUCCESS;
}


