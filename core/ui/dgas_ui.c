/*
 * dgas_ui.c
 *
 *  Created on: 6 Jun. 2025
 *      Author: rhett
 */

#include <dgas_types.h>
#include <dgas_ui.h>
#include <display.h>

// Stores handle of task
static TaskHandle_t taskHandleDgasUi;

/**
 * Get task handle of DGAS UI task
 *
 * Return: Task handle of DGAS UI task
 * */
TaskHandle_t task_dgas_ui_get_handle(void) {
	return taskHandleDgasUi;
}

/**
 * Thread function for DGAS UI task
 *
 * Return: None
 * */
void task_dgas_ui(void) {
	display_init();

	for(;;) {

	}
}

/**
 * Initialise UI controller task
 *
 * Return: None
 * */
void task_dgas_ui_init(void) {
	xTaskCreate((void*) &task_dgas_ui, "TaskDgasUi", TASK_DGAS_UI_STACK_SIZE,
			NULL, TASK_DGAS_UI_PRIORITY, &taskHandleDgasUi);
}
