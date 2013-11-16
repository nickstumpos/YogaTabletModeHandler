/*
 * LaptopModeChangedHandler.h
 *
 *  Created on: Nov 13, 2013
 *      Author: nick
 */

#ifndef LAPTOPMODECHANGEDHANDLER_H_
#define LAPTOPMODECHANGEDHANDLER_H_


class LaptopModeChangedHandler {
public:
	LaptopModeChangedHandler();
	virtual ~LaptopModeChangedHandler();
	virtual int onLaptopModeStart() = 0;
	virtual int onLaptopModeStop() = 0;
};


#endif /* LAPTOPMODECHANGEDHANDLER_H_ */
