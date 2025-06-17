/*
 * dgas_ui.h
 *
 *  Created on: 6 Jun. 2025
 *      Author: rhett
 */

#ifndef INC_DGAS_UI_H_
#define INC_DGAS_UI_H_

#include <dgas_types.h>

#define TASK_DGAS_UI_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define TASK_DGAS_UI_PRIORITY (tskIDLE_PRIORITY + 3)

/*
typedef struct {
	lv_screen_t* scrn;
	lv_group_t* group;
	lv_obj_t** objs;
	void (*evt_handler)(lv_event_t*);
}Screen;
*/

#endif /* INC_DGAS_UI_H_ */
