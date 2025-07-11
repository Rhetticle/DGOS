/*
 * dgas_gauge.c
 *
 *  Created on: 11 Jul. 2025
 *      Author: rhett
 */

#include <dgas_gauge.h>
#include <dgas_ui.h>
#include <ui.h>
#include <lvgl.h>

// Task handle for gauge task
static TaskHandle_t taskHandleGauge;

/**
 * Get task handle of gauge task
 *
 * Return: Task handle of gauge task
 * */
TaskHandle_t task_dgas_get_handle_gauge(void) {
	return taskHandleGauge;
}

/**
 * Perform start-up animation of gauge.
 *
 * Return: None
 * */
void gauge_animate(void) {
	int16_t scale = 0;

	while(scale < 4200) {
		if (ui_take_semaphore() == pdTRUE) {
			lv_arc_set_value(objects.gauge_arc, scale);
			ui_give_semaphore();
		}
		vTaskDelay(5);
		scale += 50;
	}
	while (scale >= 0) {
		if (ui_take_semaphore() == pdTRUE) {
			lv_arc_set_value(objects.gauge_arc, scale);
			ui_give_semaphore();
		}
		vTaskDelay(5);
		scale -= 50;
	}
}

/**
 * Initialise gauge UI for use
 *
 * Return: None
 * */
void gauge_init(void) {
	gauge_animate();
}

/**
 * Task function for gauge task
 *
 * Return: None
 * */
void task_dgas_gauge(void) {
	gauge_init();

	for(;;) {
		vTaskDelay(100);
	}
}

/**
 * Initialise gauge task
 *
 * Return: None
 * */
void task_dgas_gauge_init(void) {
	xTaskCreate((void*) &task_dgas_gauge, "Gauge", TASK_DGAS_GAUGE_STACK_SIZE,
			NULL, TASK_DGAS_GAUGE_PRIORITY, &taskHandleGauge);
}

