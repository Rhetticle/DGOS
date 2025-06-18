/*
 * dgas_ui.c
 *
 *  Created on: 6 Jun. 2025
 *      Author: rhett
 */

#include <dgas_types.h>
#include <dgas_ui.h>
#include <display.h>
#include <lvgl.h>

// Stores handle of UI controller task
static TaskHandle_t taskHandleDgasUi;
// Stores handle of LVGL task
static TaskHandle_t taskHandleLVGL;
// semaphore to synchronise GUI operations
SemaphoreHandle_t semaphoreUI;

/**
 * Get task handle of DGAS UI task
 *
 * Return: Task handle of DGAS UI task
 * */
TaskHandle_t task_dgas_ui_get_handle(void) {
	return taskHandleDgasUi;
}

/**
 * Get task handle of LVGL task
 *
 * Return: Task handle of LVGL task
 * */
TaskHandle_t task_lvgl_get_handle(void) {
	return taskHandleLVGL;
}

/**
 * Take UI semaphore
 *
 * Return: pdTRUE if taken, pdFALSE otherwise
 * */
BaseType_t ui_take_semaphore(void) {
	return xSemaphoreTake(semaphoreUI, 10);
}

/**
 * Give UI semaphore
 *
 * Return: None
 * */
void ui_give_semaphore(void) {
	xSemaphoreGive(semaphoreUI);
}

/**
 * Thread function for DGAS UI task
 *
 * Return: None
 * */
void task_dgas_ui(void) {
	taskENTER_CRITICAL();
	semaphoreUI = xSemaphoreCreateBinary();
	display_init();
	taskEXIT_CRITICAL();

	for(;;) {

	}
}

/**
 * LVGL tick task function.
 *
 * Return: None
 * */
void task_lvgl_tick(void) {

	for(;;) {
		if (ui_take_semaphore() == pdTRUE) {
			// got semaphore, update UI
			lv_task_handler();
			ui_give_semaphore();
		}
		vTaskDelay(10);
	}
}

/**
 * Initialise UI controller task
 *
 * Return: None
 * */
void task_dgas_ui_init(void) {
	// create UI controller task
	xTaskCreate((void*) &task_dgas_ui, "TaskDgasUi", TASK_DGAS_UI_STACK_SIZE,
			NULL, TASK_DGAS_UI_PRIORITY, &taskHandleDgasUi);
	// create LVGL tick task
	xTaskCreate((void*) &task_lvgl_tick, "TaskLVGL", TASK_DGAS_LVGL_STACK_SIZE,
			NULL, TASK_DGAS_LVGL_PRIORITY, &taskHandleLVGL);
}
