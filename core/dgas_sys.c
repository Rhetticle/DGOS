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
#include <kwp.h>

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
	task_dgas_ui_init();
	//task_dgas_obd_init();
	task_init_kwp_bus();

	for (;;) {
		if (queueAccelerometerData != NULL) {
			xQueueReceive(queueAccelerometerData, &data, 10);
		}
		vTaskDelay(200);
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
