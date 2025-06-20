/*
 * accelerometer.c
 *
 *  Created on: 7 Jun. 2025
 *      Author: rhett
 */

#include <accelerometer.h>
#include <stm32f7xx.h>
#include <stdbool.h>
#include <i2c.h>

// Queue for sending accelerometer values
QueueHandle_t queueAccelerometerData;
// Queue for configuring accelerometer
QueueHandle_t queueAccelerometerConf;
// stores configuration of accelerometer
static AccelConfig conf;
// stores task handle for controller task
static TaskHandle_t taskHandleAccelerometer;
// stores I2C bus being used with accelerometer
static I2C_HandleTypeDef accBus;

/**
 * Get task handle of accelerometer task
 *
 * Return: Task handle of accelerometer task
 * */
TaskHandle_t task_get_handle_accelerometer(void) {
	return taskHandleAccelerometer;
}

/**
 * Get current configuration of accelerometer
 *
 * Return: Current configuration of accelerometer
 * */
AccelConfig* accelerometer_get_config(void) {
	return &conf;
}

/**
 * Initialise I2C peripheral for accelerometer
 *
 * Return: None
 * */
static void accelerometer_init_i2c(void) {
	accBus.Instance = ACC_I2C_INSTANCE;
	accBus.Init.Timing = 0x00303D5B;
	accBus.Init.OwnAddress1 = 0;
	accBus.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	accBus.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	accBus.Init.OwnAddress2 = 0;
	accBus.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	accBus.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	accBus.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	HAL_I2C_Init(&accBus);
}

/**
 * Write data to accelerometer register
 *
 * data: Data to write
 * size: Number of bytes to write
 * reg: Register address
 *
 * Return: status indicating success or failure
 * */
DeviceStatus accelerometer_write(uint8_t* data, uint32_t size, uint32_t reg) {
	HAL_StatusTypeDef status;

	taskENTER_CRITICAL();
	if ((status = HAL_I2C_Mem_Write(&accBus, ACC_I2C_ADDR, reg, sizeof(uint8_t),
			data, size, 10)) != HAL_OK) {
		taskEXIT_CRITICAL();
		if (status == HAL_TIMEOUT) {
			return DEV_TIMEOUT;
		} else {
			return DEV_WRITE_ERROR;
		}
	}
	taskEXIT_CRITICAL();
	return DEV_OK;
}

/**
 * Read data from accelerometer register
 *
 * data: Destination buffer
 * size: Number of bytes to read
 * reg: Register address
 *
 * Return: status indicating success or failure
 * */
DeviceStatus accelerometer_read(uint8_t* dest, uint32_t size, uint32_t reg, uint32_t timeout) {
	HAL_StatusTypeDef status;

	taskENTER_CRITICAL();
	if ((status = HAL_I2C_Mem_Read(&accBus, ACC_I2C_ADDR, reg, sizeof(uint8_t),
			dest, size, timeout)) != HAL_OK) {
		taskEXIT_CRITICAL();
		if (status == HAL_TIMEOUT) {
			return DEV_TIMEOUT;
		} else {
			return DEV_WRITE_ERROR;
		}
	}
	taskEXIT_CRITICAL();
	return DEV_OK;
}

/**
 * Determine conversion rate to use based on configuration
 *
 * Return: Conversion rate (mg/digit)
 * */
float accelerometer_determine_conv_rate(void) {
	if (conf.range == ACC_RANGE_2G) {
		if (conf.highRes) {
			return ACC_CONV_RATE_2G_HIGH_RES;
		} else {
			return ACC_CONV_RATE_2G_NORMAL;
		}
	} else if (conf.range == ACC_RANGE_4G) {
		if (conf.highRes) {
			return ACC_CONV_RATE_4G_HIGH_RES;
		} else {
			return ACC_CONV_RATE_4G_NORMAL;
		}
	} else if (conf.range == ACC_RANGE_8G) {
		if (conf.highRes) {
			return ACC_CONV_RATE_8G_HIGH_RES;
		} else {
			return ACC_CONV_RATE_8G_NORMAL;
		}
	} else if (conf.range == ACC_RANGE_16G) {
		if (conf.highRes) {
			return ACC_CONV_RATE_16G_HIGH_RES;
		} else {
			return ACC_CONV_RATE_16G_NORMAL;
		}
	}
	// shouldn't get here
	return 0;
}

/**
 * Set the measurement range of accelerometer
 *
 * range: Range to set to
 *
 * Return: status indicating success or failure
 * */
DeviceStatus accelerometer_set_range(uint8_t range) {
	uint8_t tmp;
	DeviceStatus status;
	// read the current value of control register 4
	if ((status = accelerometer_read(&tmp, sizeof(uint8_t), CTRL_REG_4, 100)) != DEV_OK) {
		return status;
	}
	// clear range and set new range
	tmp &= ~CTRL_REG_4_RANGE_MSK;
	tmp |= range;

	// write new configuration to control register 4
	if ((status = accelerometer_write(&tmp, sizeof(uint8_t), CTRL_REG_4)) != DEV_OK) {
		return status;
	}
	return DEV_OK;
}

/**
 * Set the sample rate of accelerometer
 *
 * sRate: Sample rate to use
 *
 * Return: status indicating success or failure
 * */
DeviceStatus accelerometer_set_sample_rate(uint8_t sRate) {
	uint8_t tmp;
	DeviceStatus status;

	// read the current value of control register 1
	if ((status = accelerometer_read(&tmp, sizeof(uint8_t), CTRL_REG_1, 100)) != DEV_OK) {
		return status;
	}
	// clear sample rate and set new sample rate
	tmp &= ~CTRL_REG_1_RATE_MSK;
	tmp |= sRate;

	// write new configuration to control register 1
	if ((status = accelerometer_write(&tmp, sizeof(uint8_t), CTRL_REG_1)) != DEV_OK) {
		return status;
	}
	return DEV_OK;
}

/**
 * Set measurement resolution mode of accelerometer. Either normal or
 * high resolution.
 *
 * highRes: True for high resolution mode, false for normal
 *
 * Return: status indicating success or failure
 * */
DeviceStatus accelerometer_set_resolution(bool highRes) {
	uint8_t tmp;
	DeviceStatus status;

	if ((status = accelerometer_read(&tmp, sizeof(uint8_t), CTRL_REG_4, 100)) != DEV_OK) {
		return status;
	}

	tmp &= ~(1 << HR);
	tmp |= highRes;

	if ((status = accelerometer_write(&tmp, sizeof(uint8_t), CTRL_REG_4)) != DEV_OK) {
		return status;
	}
	return DEV_OK;
}

/**
 * Initialise accelerometer
 *
 * sampleRate: Sample rate to use e.g. ACC_SAMPLE_100HZ etc
 * range: Sample range to use e.g. ACC_RANGE_8G etc.
 * highRes: True if wish to use high resolution mode, false otherwise
 *
 * Return: status indicating success or failure
 * */
DeviceStatus accelerometer_init(uint8_t sampleRate, uint8_t range, bool highRes) {
	DeviceStatus status;
	uint8_t ctr1Data = sampleRate | (1 << XEN) | (1 << YEN) | (1 << ZEN);
	uint8_t ctr4Data = range;

	if (highRes) {
		// enable high resolution
		ctr4Data |= (1 << HR);
	}

	if ((status = accelerometer_write(&ctr1Data, sizeof(ctr1Data), CTRL_REG_1)) != DEV_OK) {
		return status;
	}

	if ((status = accelerometer_write(&ctr4Data, sizeof(uint8_t), CTRL_REG_4)) != DEV_OK) {
		return status;
	}
	// set config struct
	conf.sampleRate = sampleRate;
	conf.range = range;
	conf.highRes = highRes;

	return DEV_OK;
}

/**
 * Convert raw acceleration data to actual acceleration
 *
 * raw: Array of raw accelerometer values
 * acc: Destination buffer to store results
 *
 * Return: None
 * */
static void accelerometer_conv_raw_to_acc(uint8_t* raw, float* acc) {
	float convRate = accelerometer_determine_conv_rate();
	uint8_t offset;
	uint16_t xRaw, yRaw, zRaw;

	if (conf.highRes) {
		offset = ACC_VALUE_OFFSET_HIGH_RES;
	} else {
		offset = ACC_VALUE_OFFSET_NORMAL;
	}

	xRaw = (((uint16_t) raw[1] << 8) | raw[0]);
	yRaw = (((uint16_t) raw[3] << 8) | raw[2]);
	zRaw = (((uint16_t) raw[5] << 8) | raw[4]);

	xRaw >>= offset;
	yRaw >>= offset;
	zRaw >>= offset;

	acc[0] = (float) xRaw * convRate;
	acc[1] = (float) yRaw * convRate;
	acc[2] = (float) zRaw * convRate;
}

/**
 * Get acceleration reading update from accelerometer
 *
 * data: AccelData struct to store readings
 *
 * Return: status indicating success or failure
 * */
DeviceStatus accelerometer_get_update(AccelData* data) {
	DeviceStatus status;
	uint8_t accRaw[ACC_BYTES_NO * ACC_AXIS_COUNT];
	float accData[ACC_AXIS_COUNT];

	for (int i = 0; i < ACC_BYTES_NO; i++) {
		// loop through and read all six acceleration data registers
		if ((status = accelerometer_read(&accRaw[i], sizeof(uint8_t),
					ACC_DATA_START_ADDR + i, 100)) != DEV_OK) {
			return status;
		}
	}

	// convert raw to acceleration (in units of g's)
	accelerometer_conv_raw_to_acc(accRaw, accData);
	data->accX = accData[0];
	data->accY = accData[1];
	data->accZ = accData[2];
	return DEV_OK;
}

/**
 * Run a self test on the accelerometer. Checks the whoAmI register value as
 * simple way of testing accelerometer.
 *
 * Return: DEV_OK if self test was successfull, error code otherwise
 * */
DeviceStatus accelerometer_self_test(void) {
	uint8_t whoAmI;
	DeviceStatus status;

	if ((status = accelerometer_read(&whoAmI, sizeof(uint8_t),
			WHO_AM_I_REG, 100)) != DEV_OK) {
		return status;
	}
	if (whoAmI != WHO_AM_I) {
		return DEV_READ_ERROR;
	}
	return DEV_OK;
}

/**
 * Configure sample rate, resolution and range of accelerometer.
 *
 * config: Config struct to write to accelerometer
 *
 * Return: status indicating success or failure
 * */
DeviceStatus accelerometer_configure(AccelConfig* config) {
	DeviceStatus status;

	if (config->sampleRate != conf.sampleRate) {
		if ((status = accelerometer_set_sample_rate(config->sampleRate)) != DEV_OK) {
			return status;
		}
	}
	if (config->range != conf.range) {
		if ((status = accelerometer_set_range(config->range)) != DEV_OK) {
			return status;
		}
	}
	if (config->highRes != conf.highRes) {
		if ((status = accelerometer_set_resolution(config->highRes)) != DEV_OK) {
			return status;
		}
	}
	return DEV_OK;
}

/**
 * Accelerometer control task thread function
 *
 * Return: None
 * */
void task_accelerometer(void) {
	AccelData accData = {0};
	AccelConfig config = {0};


	queueAccelerometerData = xQueueCreate(1, sizeof(AccelData));
	queueAccelerometerConf = xQueueCreate(1, sizeof(AccelConfig));
	accelerometer_init(ACC_SAMPLE_400HZ, ACC_RANGE_2G, true);

	for(;;) {
		if (accelerometer_get_update(&accData) != DEV_OK) {
			// do something
		} else {
			// send update to queue
			xQueueSend(queueAccelerometerData, &accData, 10);
		}
		if (xQueueReceive(queueAccelerometerConf, &config, 10) == pdTRUE) {
			// got configuration so configure accelerometer
			if (accelerometer_configure(&config) != DEV_OK) {
				// do something
			}
		}
		if (ulTaskNotifyTake(pdTRUE, 10) == NOTI_ACCEL_GET_CONFIG) {
			// task has requested accelerometer configuration so send to queue
			xQueueSend(queueAccelerometerConf, &conf, 10);
		}
		vTaskDelay(50);
	}
}

/**
 * Initialise accelerometer controller task
 *
 * Return: None
 * */
void task_init_accelerometer(void) {
	xTaskCreate((void*) &task_accelerometer , "TaskAccelerometer",
			TASK_ACCELEROMETER_STACK_SIZE, NULL, TASK_ACCELEROMETER_STACK_SIZE,
		&taskHandleAccelerometer);
}


