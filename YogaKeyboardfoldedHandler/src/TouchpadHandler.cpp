/*
 * TouchpadHandler.cpp
 *
 *  Created on: Nov 15, 2013
 *      Author: nick
 */

#include "TouchpadHandler.h"

using namespace std;

TouchpadHandler::TouchpadHandler() {
	display = XOpenDisplay(NULL);
	dev = openDevice();

}

TouchpadHandler::~TouchpadHandler() {
	XCloseDevice(display, dev);
	XCloseDisplay(display);
}
XDevice* TouchpadHandler::openDevice() {
	long ret = None;
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
	return XOpenDevice(display, ret);
}
void TouchpadHandler::changeEnabledDeviceState(char * val) {

	printf("inenableDevice\n");
	Atom deviceEnableProperty = XInternAtom(display, "Device Enabled", 0);
	unsigned char *c =
			reinterpret_cast<unsigned char *>(calloc(1, sizeof(long)));
	c[0] = atoi(val);
	XChangeDeviceProperty(display, dev, deviceEnableProperty, XA_INTEGER, 8,
	PropModeReplace, c, 1);
	XSync(display, 0);
	free(c);

}
int TouchpadHandler::onLaptopModeStart(){

		changeEnabledDeviceState((char *)"1");
		return EXIT_SUCCESS;
}
int TouchpadHandler::onLaptopModeStop(){

		changeEnabledDeviceState((char *)"0");
		return EXIT_SUCCESS;
}
