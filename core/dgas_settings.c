/*
 * dgas_settings.c
 *
 *  Created on: 2 Jan. 2026
 *      Author: rhett
 */

#include <stdio.h>
#include <string.h>
#include <flash.h>
#include <device.h>
#include <dgas_settings.h>

/**
 * Save gauge config to flash memory.
 *
 * conf: Gauge config to save
 *
 * Return: Status indicating success or failure
 * */
DStatus dgas_settings_config_save(GaugeConfig* conf) {
	FlashReq req = {0};
	FlashBuf* buf = flash_alloc_buffer(FLASH_ALLOC_TIMEOUT_10);
	DeviceStatus stat;

	req.rCmd = FLASH_CMD_WRITE;
	req.rAddr = DGAS_SETTINGS_FLASH_CONFIG_ADDR;
	req.rSize = sizeof(GaugeConfig);
	req.rCaller = xTaskGetCurrentTaskHandle();

	memcpy(buf, conf, sizeof(GaugeConfig));
	req.rBuf = buf;

	xQueueSend(queueFlashReq, &req, portMAX_DELAY);
	stat = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	flash_free_buffer(buf);

	if (stat != DEV_OK) {
		return DGAS_STATUS_ERROR;
	}
	return DGAS_STATUS_OK;
}

/**
 * Read gauge config from flash memory
 *
 * dest: Destination GaugeConfig struct to store config
 *
 * Return: Status indicating success or failure
 * */
DStatus dgas_settings_config_read(GaugeConfig* dest) {
	FlashReq req = {0};
	FlashBuf* buf = flash_alloc_buffer(FLASH_ALLOC_TIMEOUT_10);
	DeviceStatus stat;

	req.rBuf = buf;
	req.rAddr = DGAS_SETTINGS_FLASH_CONFIG_ADDR;
	req.rSize = sizeof(GaugeConfig);
	req.rCmd = FLASH_CMD_READ;
	req.rCaller = xTaskGetCurrentTaskHandle();

	xQueueSend(queueFlashReq, &req, portMAX_DELAY);
	stat = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

	if (stat != DEV_OK) {
		flash_free_buffer(buf);
		return DGAS_STATUS_ERROR;
	}
	memcpy(dest, buf, sizeof(GaugeConfig));
	flash_free_buffer(buf);
	return DGAS_STATUS_OK;
}


