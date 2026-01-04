/*
 * dgas_types.h
 *
 *  Created on: Jun 17, 2025
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_DGAS_TYPES_H_
#define DGOS_INCLUDE_DGAS_TYPES_H_

#include <dgas_conf.h>
#include <stm32f7xx.h>

#ifdef DGAS_CONFIG_USE_FREERTOS
#include <FreeRTOS.h>
#include <queue.h>
#include <message_buffer.h>
#include <stream_buffer.h>
#include <semphr.h>
#include <event_groups.h>
#else
#error "FreeRTOS not enabled, check dgas_conf.h"
#endif

#define DGAS_ATTR_FLASH			__attribute__((section(".extFlash")))

typedef enum {
	DGAS_STATUS_OK,
	DGAS_STATUS_ERROR
}DStatus;

#endif /* DGOS_INCLUDE_DGAS_TYPES_H_ */
