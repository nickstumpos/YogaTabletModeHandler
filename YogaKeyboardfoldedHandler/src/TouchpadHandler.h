/*
 * TouchpadHandler.h
 *
 *  Created on: Nov 15, 2013
 *      Author: nick
 */

#ifndef TOUCHPADHANDLER_H_
#define TOUCHPADHANDLER_H_

#include "LaptopModeChangedHandler.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/Xatom.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
using namespace std;

class TouchpadHandler: public LaptopModeChangedHandler {
private:
	Display *display;
	XDevice *dev;
	XDevice* openDevice();
	void changeEnabledDeviceState(char * val);
public:
	TouchpadHandler();
	virtual ~TouchpadHandler();
	int onLaptopModeStart();
	int onLaptopModeStop();
};


#endif /* TOUCHPADHANDLER_H_ */
