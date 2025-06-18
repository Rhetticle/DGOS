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
#include <ltdc.h>
#include <buttons.h>

// Stores handle of UI controller task
static TaskHandle_t taskHandleDgasUi;
// Stores handle of LVGL task
static TaskHandle_t taskHandleLVGL;
// semaphore to synchronise GUI operations
static SemaphoreHandle_t semaphoreUI;
// LVGL display
static lv_display_t* display;

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
 * LVGL encoder read callback function. Although a physical encoder is not being
 * used, we can make LVGL think one is where navigation button presses correspond
 * to rotation of encoder and selection button pressed correspond to encoder presses
 *
 * indev: LVGL input device
 * data: LVGL input device data
 *
 * Return: None
 * */
void encoder_read(lv_indev_t* indev, lv_indev_data_t* data) {
	EventBits_t uxBits = xEventGroupWaitBits(eventButton, EVT_BUTTON_PRESSED,
											pdTRUE, pdFALSE, 0);

	if (uxBits & EVT_NAV_PRESSED) {
		// navigation button pressed so increment encoder position
		data->enc_diff++;
	}
	if (uxBits & EVT_SEL_PRESSED) {
		data->state = LV_INDEV_STATE_PRESSED;
	} else {
		data->state = LV_INDEV_STATE_RELEASED;
	}
}

/**
 * Frame buffer flush function to be used with LVGL
 *
 * Return: None
 * */
void ui_flush_frame_buffer(lv_display_t* disp, const lv_area_t area, uint8_t* map) {
	if(lv_display_is_double_buffered(disp) && lv_display_flush_is_last(disp)) {
		HAL_LTDC_SetAddress_NoReload(&hltdc, (uint32_t)map, 0);
	    HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
	}
	lv_display_flush_ready(disp);
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
	// dram_init();
	lv_init();
	// set LVGL tick callback
	lv_tick_set_cb(HAL_GetTick);
	// create display object and set start address of both frame buffers
	display = lv_display_create(LCD_RESOLUTION_X, LCD_RESOLUTION_Y);
	lv_display_set_buffers(display, (void*) UI_FRAME_BUFF_ONE_ADDR, (void*) UI_FRAME_BUFF_ONE_ADDR,
						UI_FRAME_BUFF_SIZE, LV_DISP_RENDER_MODE_DIRECT);
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
