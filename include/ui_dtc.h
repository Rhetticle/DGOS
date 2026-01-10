/*
 * ui_dtc.h
 *
 *  Created on: 8 Jan. 2026
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_UI_DTC_H_
#define DGOS_INCLUDE_UI_DTC_H_

#include <dgas_types.h>
#include <dgas_ui.h>

#define UI_DTC_REPORTED_STR_LEN_MAX	64

void ui_dtc_make_request(UICmd cmd, void* arg);
void ui_dtc_init(void);

#endif /* DGOS_INCLUDE_UI_DTC_H_ */
