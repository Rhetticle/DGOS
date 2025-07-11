/*
 * dgas_gauge.h
 *
 *  Created on: 11 Jul. 2025
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_DGAS_GAUGE_H_
#define DGOS_INCLUDE_DGAS_GAUGE_H_

#include <dgas_types.h>

#define TASK_DGAS_GAUGE_PRIORITY		(tskIDLE_PRIORITY + 4)
#define TASK_DGAS_GAUGE_STACK_SIZE		(configMINIMAL_STACK_SIZE * 8)

TaskHandle_t task_dgas_get_handle_gauge(void);
void gauge_animate(void);
void gauge_init(void);
void task_dgas_gauge_init(void);

#endif /* DGOS_INCLUDE_DGAS_GAUGE_H_ */
