/*
 * dgas_debug.h
 *
 *  Created on: 19 Jul. 2025
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_DGAS_DEBUG_H_
#define DGOS_INCLUDE_DGAS_DEBUG_H_

#include <dgas_types.h>
#include <bus.h>

/************************ FreeRTOS *********************/

#define TASK_DGAS_DEBUG_PRIORITY		(tskIDLE_PRIORITY + 3)
#define TASK_DGAS_DEBUG_STACK_SIZE		(configMINIMAL_STACK_SIZE * 4)

extern QueueHandle_t queueDebug;

#endif /* DGOS_INCLUDE_DGAS_DEBUG_H_ */
