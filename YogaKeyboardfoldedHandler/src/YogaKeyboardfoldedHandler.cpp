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
#include "LaptopModeChangedHandler.h"
#include "LaptopModeManager.h"
#include "TouchpadHandler.h"
using namespace std;



int main() {
//	if (geteuid()) {
//		error(EXIT_FAILURE, errno, "Need to run as root");
//	}
	//TODO
	//Daemonize
	//listenforstop
	//clean mem on stop
	vector<LaptopModeChangedHandler*> handlers ;
	TouchpadHandler *touchhandler = new TouchpadHandler();
	handlers.push_back(touchhandler);
	LaptopModeManager *manager = new LaptopModeManager(&handlers);
	manager->start();
	delete(manager);
	return EXIT_SUCCESS;
}

