/*
 * ui_debug.h
 *
 *  Created on: 2 Jan. 2026
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_UI_DEBUG_H_
#define DGOS_INCLUDE_UI_DEBUG_H_

#include <dgas_types.h>
#include <dgas_ui.h>

#define UI_DEBUG_WINDOW_MAX_TEXT				300

void ui_debug_make_request(UICmd cmd, void* arg);
void ui_debug_init(void);

#endif /* DGOS_INCLUDE_UI_DEBUG_H_ */
