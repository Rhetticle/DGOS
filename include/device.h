/*
 * device.h
 *
 *  Created on: 8 Jun. 2025
 *      Author: rhett
 */

#ifndef INC_DEVICE_H_
#define INC_DEVICE_H_

typedef enum {
	DEV_OK,
	DEV_READ_ERROR,
	DEV_WRITE_ERROR,
	DEV_TIMEOUT
}DeviceStatus;

typedef enum {
	DEV_FLAG_OPT_SET,
	DEV_FLAG_OPT_CLEAR
}DevFlagOpt;

#endif /* INC_DEVICE_H_ */
