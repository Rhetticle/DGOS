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
#include <dgas_settings.h>
#include <dgas_param.h>
#include <accelerometer.h>
#include <dgas_adc.h>
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
 * Measure UI event handler
 *
 * eParams: Event parameters
 * eCount: Number of event parameters
 *
 * Return: None
 * */
void meas_event_handler(uint32_t* eParams, uint32_t eCount) {
	if (eCount == 0) {
		return;
	}
	GaugeParamID parameter = eParams[0];
	xEventGroupSetBits(eventGaugeParam, 1 << (uint32_t) parameter);
}

/**
 * Event handler for OBD debugger
 *
 * eCode: Event code
 *
 * Return: None
 * */
void debug_event_handler(UIEventCode eCode) {
	if (eCode == UI_EVENT_DEBUG_PAUSE) {
		dgas_debug_pause();
	} else if (eCode == UI_EVENT_DEBUG_RESUME) {
		dgas_debug_resume();
	}
}

void settings_event_handler(UIEventCode eCode, UISettingsState* state) {
	if (eCode == UI_EVENT_SETTINGS_SAVE) {
		GaugeConfig conf = {0};
		conf.bid = (BusID) state->sBus;
		conf.gid = (GaugeParamID) state->sParam;
		dgas_settings_config_save(&conf);
	}
}

/**
 * Handle UI event
 *
 * evt: UI event that occured
 *
 * Return: None
 * */
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
		dgas_self_test();
		break;
	case UI_UID_SETTINGS: {
		UISettingsState state = {0};
		memcpy(&state, evt->eParams, sizeof(UISettingsState));
		settings_event_handler(evt->eCode, &state);
		break;
	}
	default:
		break;
	}
}

/**
 * Wait for given object to be created (e.g. FreeRTOS object)
 *
 * obj: Pointer to object to wait on
 * timeout: Timeout in ms
 *
 * Return: 0 on success, 1 on timeout
 * */
uint32_t dgas_sys_wait_on_object(void** obj, uint32_t timeout) {
	uint32_t time = 0;

	while(*obj == NULL) {
		vTaskDelay(1);
		time++;
		if (time == timeout) {
			return 1;
		}
	}
	return 0;
}

/**
 * Initialise hardware and devices for DGAS
 *
 * Return: 0 on success, error number otherwise
 * */
uint32_t dgas_sys_hardware_init(void) {
	dram_init();
	display_init();
	flash_init();
	flash_enable_memory_mapped();
	task_init_buttons();
	task_init_accelerometer();
	if (dgas_sys_wait_on_object((void**)&queueAccelerometerData, DGAS_SYS_BOOT_TIMEOUT_DEV) != 0) {
		return DGAS_SYS_BOOT_ERROR_DEV;
	}
	dgas_task_adc_init();
	if (dgas_sys_wait_on_object((void**)&queueADC, DGAS_SYS_BOOT_TIMEOUT_DEV) != 0) {
		return DGAS_SYS_BOOT_ERROR_DEV;
	}
	return DGAS_SYS_BOOT_OK;
}

/**
 * Main boot sequence for DGAS
 *
 * Return: 0 if successful, error number otherwise
 * */
uint32_t dgas_sys_boot(void) {
	if (dgas_sys_hardware_init() != 0) {
		return DGAS_SYS_BOOT_ERROR_DEV;
	}
	task_dgas_ui_init();
	if (dgas_sys_wait_on_object((void**)&queueUIEvent, DGAS_SYS_BOOT_TIMEOUT_UI) != 0) {
		return DGAS_SYS_BOOT_ERROR_UI;
	}
	//task_dgas_obd_init();
	//if (dgas_sys_wait_on_object((void**)&queueOBDRequest, DGAS_SYS_BOOT_TIMEOUT_OBD) != 0) {
	//	return DGAS_SYS_BOOT_ERROR_OBD;
	//}
	task_dgas_gauge_init();
	//if (dgas_sys_wait_on_object((void**)&queueGaugeUpdate, DGAS_SYS_BOOT_TIMEOUT_UI) != 0) {
	//	return DGAS_SYS_BOOT_ERROR_UI;
	//}
	//task_dgas_debug_init();
	//if (dgas_sys_wait_on_object((void**)&queueDebug, DGAS_SYS_BOOT_TIMEOUT_UI) != 0) {
	//	return DGAS_SYS_BOOT_ERROR_UI;
	//}
	dgas_selftest_init();
	return DGAS_SYS_BOOT_OK;
}

/**
 * Thread function for DGAS system task
 *
 * Return: None
 * */
void task_dgas_sys(void) {
	AccelData data;
	UIEvent evt;

	dgas_sys_boot();

	for (;;) {
		if (queueAccelerometerData != NULL) {
			xQueueReceive(queueAccelerometerData, &data, 10);
		}
		if (queueUIEvent != NULL) {
			if (xQueueReceive(queueUIEvent, &evt, 0) == pdTRUE) {
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
