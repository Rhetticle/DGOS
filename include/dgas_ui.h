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
#define TASK_DGAS_LVGL_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define TASK_DGAS_LVGL_PRIORITY (tskIDLE_PRIORITY + 4)

#define SDRAM_START_ADDR 0xC0000000

#define UI_FRAME_BUFF_SIZE (LCD_RESOLUTION_X * LCD_RESOLUTION_Y)
#define UI_FRAME_BUFF_ONE_ADDR (SDRAM_START_ADDR - UI_FRAME_BUFF_SIZE)
#define UI_FRAME_BUFF_TWO_ADDR (SDRAM_START_ADDR)

#define DGAS_ATTR_FLASH (__attribute__((section(".extFlash"))))

typedef struct {
	lv_obj_t* screen;
	lv_group_t* group;
}Screen;

typedef struct {
	uint32_t min;
	uint32_t max;
	char* units;
	char* name;
	uint32_t colour;
}GaugeParam;

#endif /* INC_DGAS_UI_H_ */
