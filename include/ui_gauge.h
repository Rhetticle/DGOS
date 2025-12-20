/*
 * ui_gauge.h
 *
 *  Created on: 22 Nov. 2025
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_UI_GAUGE_H_
#define DGOS_INCLUDE_UI_GAUGE_H_

#include <dgas_types.h>
#include <dgas_ui.h>

#define UI_GAUGE_ARC_TICK_COUNT	7

#define UI_GAUGE_ANIM_TIME					1250
#define UI_GAUGE_ANIM_ARC_START_VALUE		0
#define UI_GAUGE_ANIM_ARC_END_VALUE			4200
#define UI_GAUGE_ANIM_ARC_STEP_SIZE			150

#define UI_GAUGE_OBD_STATUS_BUFF_LEN		32
#define UI_GAUGE_PARAM_VAL_BUFF_LEN			32
#define UI_GAUGE_VBAT_BUFF_LEN				32
#define UI_GAUGE_PARAM_MAX_BUFF_LEN			32
#define UI_GAUGE_TICK_BUFF_LEN				32

void ui_gauge_make_request(UICmd cmd, void* arg);
void ui_gauge_init(void);

#endif /* DGOS_INCLUDE_UI_GAUGE_H_ */
