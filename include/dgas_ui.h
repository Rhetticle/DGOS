/*
 * dgas_ui.h
 *
 *  Created on: 6 Jun. 2025
 *      Author: rhett
 */

#ifndef INC_DGAS_UI_H_
#define INC_DGAS_UI_H_

#include <dgas_types.h>
#include <lvgl.h>

// UI task priorities and stack sizes. If the UI task does any drawing
// it should have a reasonably large stack as the call chain of most LVGL
// functions is quite long. The LVGL tick task should have the highest priority
#define TASK_DGAS_UI_STACK_SIZE 			(configMINIMAL_STACK_SIZE * 8)
#define TASK_DGAS_UI_PRIORITY 				(tskIDLE_PRIORITY + 3)
#define TASK_DGAS_LVGL_UPDATE_STACK_SIZE 	(configMINIMAL_STACK_SIZE * 12)
#define TASK_DGAS_LVGL_UPDATE_PRIORITY 		(tskIDLE_PRIORITY + 7)
#define TASK_DGAS_LVGL_TICK_STACK_SIZE		(configMINIMAL_STACK_SIZE * 2)
#define TASK_DGAS_LVGL_TICK_PRIORITY		(tskIDLE_PRIORITY + 8)

typedef struct {
	lv_obj_t* screen;
	lv_group_t* group;
}UI;

typedef void (*evtCallback) (lv_event_t*);

typedef enum {
	UI_CALLBACK_USE_FOR_ALL,
	UI_CALLBACK_USE_FOR_SCREEN
}UICallbackOpt;

// Function prototypes
BaseType_t ui_take_semaphore(void);
void ui_give_semaphore(void);
TaskHandle_t task_dgas_ui_get_handle(void);
TaskHandle_t task_lvgl_update_get_handle(void);
TaskHandle_t task_lvgl_tick_get_handle(void);
void ui_load_screen(UI* ui);
void task_dgas_lvgl_update_init(void);
void task_dgas_lvgl_tick_init(void);
void task_dgas_ui_init(void);

#endif /* INC_DGAS_UI_H_ */
