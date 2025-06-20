/*
 * accelerometer.h
 *
 *  Created on: 7 Jun. 2025
 *      Author: rhett
 */

#ifndef INC_ACCELEROMETER_H_
#define INC_ACCELEROMETER_H_

// Set to 1 if wish to use FreeRTOS with this driver
#if 1
#define ACC_USE_FREERTOS
#endif


#include <device.h>
#include <stdbool.h>

#ifdef ACC_USE_FREERTOS
#include <FreeRTOS.h>
#include <queue.h>
extern QueueHandle_t queueAccelerometerData;
extern QueueHandle_t queueAccelerometerConf;
#endif

#define ACC_I2C_INSTANCE I2C4

#define ACC_I2C_ADDR 0b0011000

#define ACC_BYTES_NO 6 // number of acceleration data bytes (2 for each of the 3 axes)
#define WHO_AM_I 0x33 // constant value stored in the WHO_AM_I register
#define ACC_AXIS_COUNT 3
#define ACC_VALUE_OFFSET_NORMAL 6 // values in normal mode are 10-bit so 6-bit offset
#define ACC_VALUE_OFFSET_HIGH_RES 4 // values in high resolution mode are 12-bit so 4-bit offset

// Acceleration output registers
#define OUT_X_L 0x28
#define OUT_X_H 0x29
#define OUT_Y_L 0x2A
#define OUT_Y_H 0x2B
#define OUT_Z_L 0x2C
#define OUT_Z_H 0x2D
#define ACC_DATA_START_ADDR 0x28 // address of first data register (OUT_X_L)

// Control registers
#define CTRL_REG_0 0x1E
#define CTRL_REG_1 0x20
#define CTRL_REG_2 0x21
#define CTRL_REG_3 0x22
#define CTRL_REG_4 0x23
#define CTRL_REG_5 0x24
#define CTRL_REG_6 0x25

//Status regsiter
#define STATUS_REG 0x27

// Who am I register
#define WHO_AM_I_REG 0x0F

// CTRL_REG_1 bit positions
#define ODR0 4
#define ODR1 5
#define ODR2 6
#define ODR3 7
#define ZEN 2
#define YEN 1
#define XEN 0

#define CTRL_REG_1_RATE_MSK ((1 << ODR0) | (1 << ODR1) | (1 << ODR2) | (1 << ODR3))

#define ACC_SAMPLE_1HZ   (1 << ODR0)
#define ACC_SAMPLE_10HZ  (1 << ODR1)
#define ACC_SAMPLE_25HZ  ((1 << ODR0) | (1 << ODR1))
#define ACC_SAMPLE_50HZ  (1 << ODR2)
#define ACC_SAMPLE_100HZ ((1 << ODR2) | (1 << ODR0))
#define ACC_SAMPLE_200HZ ((1 << ODR2) | (1 << ODR1))
#define ACC_SAMPLE_400HZ ((1 << ODR2) | (1 << ODR1) | (1 << ODR0))
#define ACC_SAMPLE_MAX   (1 << ODR3) // max sample rate on HR/Normal mode is 1.344kHz

// CTRL_REG_4 bit positions
#define BDU 7
#define BLE 6
#define FS1 5
#define FS0 4
#define HR  3
#define ST1 2
#define ST0 1
#define SIM 0

#define CTRL_REG_4_RANGE_MSK ((1 << FS1) | (1 << FS0))

// measurement ranges
#define ACC_RANGE_16G ((1 << FS1) | (1 << FS0))
#define ACC_RANGE_8G  (1 << FS1)
#define ACC_RANGE_4G  (1 << FS0)
#define ACC_RANGE_2G  (0)

// conversion rates for different ranges and resolution mode
// (units are mg/digits) (milli G's)
#define ACC_CONV_RATE_16G_NORMAL   0.048
#define ACC_CONV_RATE_16G_HIGH_RES 0.012
#define ACC_CONV_RATE_8G_NORMAL    0.016
#define ACC_CONV_RATE_8G_HIGH_RES  0.004
#define ACC_CONV_RATE_4G_NORMAL    0.008
#define ACC_CONV_RATE_4G_HIGH_RES  0.002
#define ACC_CONV_RATE_2G_NORMAL    0.004
#define ACC_CONV_RATE_2G_HIGH_RES  0.001

#define TASK_ACCELEROMETER_PRIORITY   (tskIDLE_PRIORITY + 5)
#define TASK_ACCELEROMETER_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)

#define NOTI_ACCEL_GET_CONFIG 1

typedef struct {
	float accX;
	float accY;
	float accZ;
}AccelData;

typedef struct {
	uint8_t range;
	uint8_t sampleRate;
	bool highRes;
}AccelConfig;

// Function prototypes
AccelConfig* accelerometer_get_config(void);
DeviceStatus accelerometer_write(uint8_t* data, uint32_t size, uint32_t reg);
DeviceStatus accelerometer_read(uint8_t* dest, uint32_t size, uint32_t reg, uint32_t timeout);
float accelerometer_determine_conv_rate(void);
DeviceStatus accelerometer_set_range(uint8_t range);
DeviceStatus accelerometer_set_sample_rate(uint8_t sRate);
DeviceStatus accelerometer_set_resolution(bool highRes);
DeviceStatus accelerometer_init(uint8_t sampleRate, uint8_t range, bool highRes);
DeviceStatus accelerometer_get_update(AccelData* data);
DeviceStatus accelerometer_self_test(void);
DeviceStatus accelerometer_configure(AccelConfig* config);

#ifdef ACC_USE_FREERTOS
TaskHandle_t task_get_handle_accelerometer(void);
void task_init_accelerometer(void);
#endif

#endif /* INC_ACCELEROMETER_H_ */
