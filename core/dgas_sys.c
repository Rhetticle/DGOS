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
#include <dgas_debug.h>
#include <dgas_selftest.h>
#include <dgas_param.h>
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

void meas_event_handler(uint32_t* eParams, uint32_t eCount) {
	if (eCount == 0) {
		return;
	}
	GaugeParamID parameter = eParams[0];
	xEventGroupSetBits(eventGaugeParam, 1 << (uint32_t) parameter);
}

void debug_event_handler(UIEventCode eCode) {
	if (eCode == UI_EVENT_DEBUG_PAUSE) {
		dgas_debug_pause();
	} else if (eCode == UI_EVENT_DEBUG_RESUME) {
		dgas_debug_resume();
	}
}

void handle_ui_event(UIEvent* evt) {
	switch(evt->eUid) {
	case UI_UID_MEAS:
		meas_event_handler(evt->eParams, evt->eCount);
		break;
	case UI_UID_DEBUG:
		debug_event_handler(evt->eCode);
		break;
	case UI_UID_DTC:
		break;
	case UI_UID_SELFTEST:
		break;
	case UI_UID_SETTINGS:
		break;
	default:
		break;
	}
}

/**
 * Thread function for DGAS system task
 *
 * Return: None
 * */
void task_dgas_sys(void) {
	AccelData data;
	UIEvent evt;

	task_dgas_ui_init();
	vTaskDelay(1000);
	task_dgas_gauge_init();
	//task_dgas_debug_init();
	task_dgas_obd_init();

	for (;;) {
		if (queueAccelerometerData != NULL) {
			xQueueReceive(queueAccelerometerData, &data, 10);
		}
		if (queueUIEvent != NULL) {
			if (xQueueReceive(queueUIEvent, &evt, 00) == pdTRUE) {
				handle_ui_event(&evt);
			}
		}
		vTaskDelay(10);
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
