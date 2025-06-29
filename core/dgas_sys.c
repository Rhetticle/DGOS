/*
 * dgas_sys.c
 *
 *  Created on: 2 Jun. 2025
 *      Author: rhett
 */

#include <dgas_types.h>
#include <dgas_sys.h>
#include <dgas_obd.h>
#include <dgas_ui.h>
#include <accelerometer.h>
#include <dram.h>
#include <flash.h>
#include <display.h>

// Task handle of DGAS system task
static TaskHandle_t taskHandleSys;

/**
 * Get task handle of DGAS system task
 *
 * Return: None
 * */
TaskHandle_t task_dgas_sys_get_handle(void) {
	return taskHandleSys;
}

/**
 * Thread function for DGAS system task
 *
 * Return: None
 * */
void task_dgas_sys(void) {
	AccelData data;
	DeviceStatus status;
	uint32_t count = 0;
	uint8_t s3;
	uint16_t jedec;
	taskENTER_CRITICAL();
	display_init();
	dram_init();
	vTaskDelay(10);
	dram_fill_section(0, 480*480*2, 0xFF);
	flash_init();
	taskEXIT_CRITICAL();
	status = flash_read_reg(FLASH_READ_STAT_REG_THREE, &s3, 100);
	status = flash_get_jedec_id(&jedec);

	for (;;) {

		if (xQueueReceive(queueAccelerometerData, &data, 10) == pdTRUE) {
			count++;
		}

		vTaskDelay(20);
	}
}

/**
 * Initialise DGAS system task
 *
 * Return: None
 * */
void task_dgas_sys_init(void) {
	xTaskCreate((void*) &task_dgas_sys, "TaskDGASSys", TASK_DGAS_SYS_STACK_SIZE,
				NULL, TASK_DGAS_SYS_PRIORITY, &taskHandleSys);
}
