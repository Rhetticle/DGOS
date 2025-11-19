/*
 * dgas_selftest.h
 *
 *  Created on: 26 Jul. 2025
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_DGAS_SELFTEST_H_
#define DGOS_INCLUDE_DGAS_SELFTEST_H_

#include <dgas_types.h>
#include <accelerometer.h>
#include <math.h>

#define DEVICE_REPORT_MSG_MAX 100
#define DEVICE_STATISTIC_MSG_MAX 32

#define CONV_BYTES_TO_KIB(x) 		(x / pow(2, 10))
#define CONV_BYTES_TO_MIB(x) 		(x / pow(2, 20))
#define CONV_BYTES_TO_GIB(x) 		(x / pow(2, 30))

#define CONV_MS_TO_S(x)				(x / 1000)

#define TASK_SELF_TEST_PRIORITY		(tskIDLE_PRIORITY + 3)
#define TASK_SELF_TEST_STACK_SIZE	(configMINIMAL_STACK_SIZE * 6)

/**
 * Accelerometer self test report struct
 *
 * aData: Acceleration data read from accelerometer (if successfull)
 * whoAmI: Value read from whoAmI register of accelerometer
 * */
typedef struct {
	AccelData aData;
	uint8_t whoAmI;
} AccTestDesc;

/**
 * Memory device self test report struct
 *
 * readFailAddr: Address where a read failure occurred (if any)
 * writeFailAddr: Address where a write failure occurred (if any)
 * used: Amount of memory used in KiB
 * free: Amount of memory free in KiB
 * readTime: Time taken to read entire memory array
 * writeTime: Time taken to write to entire memory array
 * */
typedef struct {
	uint32_t readFailAddr;
	uint32_t writeFailAddr;
	uint32_t used;
	uint32_t free;
	uint32_t readTime;
	uint32_t writeTime;
} MemTestDesc;

TaskHandle_t dgas_self_test_get_handle(void);

#endif /* DGOS_INCLUDE_DGAS_SELFTEST_H_ */
