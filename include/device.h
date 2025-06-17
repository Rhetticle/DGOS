/*
 * device.h
 *
 *  Created on: 8 Jun. 2025
 *      Author: rhett
 */

#ifndef INC_DEVICE_H_
#define INC_DEVICE_H_

#include <stm32f7xx.h>
#include <i2c.h>
#include <spi.h>

typedef enum {
	DEV_OK,
	DEV_READ_ERROR,
	DEV_WRITE_ERROR,
	DEV_TIMEOUT
}DeviceStatus;

#endif /* INC_DEVICE_H_ */
