/*
 * LaptopModeManager.h
 *
 *  Created on: Nov 13, 2013
 *      Author: nick
 */

#ifndef LAPTOPMODEMANAGER_H_
#define LAPTOPMODEMANAGER_H_
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
#include "LaptopModeChangedHandler.h"

using namespace std;
class LaptopModeManager {
private:
	vector<LaptopModeChangedHandler*> *handlers;
	bool isLaptopMode, running;
	time_t last_found;
	long threadCount;
	pthread_mutex_t laptopModeMutex, lastFoundTimeMutex;
	string execute(const char* cmd);
	string determine_input_device();
	int listenForNonLaptopModeKey(int yogaKeyboardFD);
public:
	LaptopModeManager(vector<LaptopModeChangedHandler*> *changeHandlers);
	int start(void);
	int stop(void);
	bool isInLaptopMode();
	void setIsInLaptop(bool inLaptopMode);
	time_t getTimeLastFound();
	void setTimeLastFound(time_t lastFound);
	int processLeavingLaptopMode();
	int processEnteringLaptopMode();
	virtual ~LaptopModeManager();

};


#endif /* LAPTOPMODEMANAGER_H_ */
