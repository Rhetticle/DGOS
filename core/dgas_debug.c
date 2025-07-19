/*
 * dgas_debug.c
 *
 *  Created on: 19 Jul. 2025
 *      Author: Rhett Humphreys
 *
 *  Debug controller for DGAS. Used for live OBD-II message debugging
 */

#include <dgas_debug.h>

// Task handle for debug task
static TaskHandle_t taskHandleDebug;

/**
 * Get task handle of debug task
 *
 * Return: Task handle of debug task
 * */
TaskHandle_t task_dgas_debug_get_handle(void) {
	return taskHandleDebug;
}

/**
 * Thread function for debug task
 *
 * Return: None
 * */
void task_debug(void) {
	for(;;) {

	}
}

/**
 * Initialiase DGAS debug task
 *
 * Return: None
 * */
void task_dgas_debug_init(void) {
	xTaskCreate((void*) &task_debug, "Debug", TASK_DGAS_DEBUG_STACK_SIZE, NULL,
			TASK_DGAS_DEBUG_PRIORITY, &taskHandleDebug);
}


