/*
 * dgas_ui.h
 *
 *  Created on: 6 Jun. 2025
 *      Author: rhett
 */

#ifndef INC_DGAS_UI_H_
#define INC_DGAS_UI_H_

#include <dgas_types.h>
#include <display.h>
#include <lvgl.h>

#define TASK_DGAS_UI_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define TASK_DGAS_UI_PRIORITY (tskIDLE_PRIORITY + 3)
#define TASK_DGAS_LVGL_STACK_SIZE (configMINIMAL_STACK_SIZE * 6)
#define TASK_DGAS_LVGL_PRIORITY (tskIDLE_PRIORITY + 7)

#define SDRAM_START_ADDR 	0xC0000000
#define SDRAM_SIZE			0x200000


#define UI_FRAME_BUFF_SIZE (LCD_RESOLUTION_X * LCD_RESOLUTION_Y * 2)
#define UI_FRAME_BUFF_ONE_ADDR (SDRAM_START_ADDR)
#define UI_FRAME_BUFF_TWO_ADDR (SDRAM_START_ADDR + SDRAM_SIZE - UI_FRAME_BUFF_SIZE)

typedef struct {
	lv_obj_t* screen;
	lv_group_t* group;
}UI;

typedef void (*evtCallback) (lv_event_t*);

typedef struct {
	uint32_t min;
	uint32_t max;
	char* units;
	char* name;
	uint32_t colour;
}GaugeParam;

typedef enum {
	UI_CALLBACK_USE_FOR_ALL,
	UI_CALLBACK_USE_FOR_SCREEN
}UICallbackOpt;

// Function prototypes
TaskHandle_t task_dgas_ui_get_handle(void);
TaskHandle_t task_lvgl_get_handle(void);
void ui_load_screen(UI* ui);
void task_dgas_ui_init(void);

#endif /* INC_DGAS_UI_H_ */
