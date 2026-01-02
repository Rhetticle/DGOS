/*
 * ui_selftest.h
 *
 *  Created on: 20 Dec. 2025
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_UI_SELFTEST_H_
#define DGOS_INCLUDE_UI_SELFTEST_H_

#include <dgas_types.h>
#include <dgas_ui.h>

#define DEVICE_REPORT_MSG_MAX 100
#define DEVICE_STATISTIC_MSG_MAX 32

void ui_selftest_make_request(UICmd cmd, void* arg);
void ui_selftest_init(void);

#endif /* DGOS_INCLUDE_UI_SELFTEST_H_ */
