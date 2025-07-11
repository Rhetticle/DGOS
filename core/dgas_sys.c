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
#include <dgas_gauge.h>
#include <accelerometer.h>
#include <dram.h>
#include <flash.h>
#include <display.h>
#include <string.h>
#include <buttons.h>
#include <stdio.h>

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
	GaugeUpdate update = {0};
	task_dgas_ui_init();
	update.vBat = 7.2;
	sprintf(update.obdStat, "OK");

	for (;;) {
		if (queueAccelerometerData != NULL) {
			if (xQueueReceive(queueAccelerometerData, &data, 10) == pdTRUE) {
			}
		}
		if (queueGaugeUpdate != NULL) {
			xQueueSend(queueGaugeUpdate, &update, 0);
		}
		update.paramVal++;
		if (update.paramVal > 4200) {
			update.paramVal = 0;
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
