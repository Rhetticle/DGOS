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

#define DGAS_DEBUG_LOG_BYTE(byte)				(xStreamBufferSend(streamDebug, &byte, 1, 0))
#define DGAS_DEBUG_LOG_STREAM(stream, len)		(xStreamBufferSend(streamDebug, stream, len, 0))

#define DGAS_DEBUG_DIRECTION_RECEIVING				(1 << 31)
#define DGAS_DEBUG_DIRECTION_TRANSMITTING			(0 << 31)
#define DGAS_DEBUG_DIRECTION_MASK					(1 << 31)

#define DGAS_DEBUG_STATUS_MASK					(~(1 << 31))

#define DGAS_DEBUG_NOTIFY(status, mode)			(xTaskNotify(task_dgas_debug_get_handle(), status | mode, eNoAction))
#define DGAS_DEBUG_NOTIFY_RECEIVING(status)		DGAS_DEBUG_NOTIFY(status, DGAS_DEBUG_MODE_RECEIVING)
#define DGAS_DEBUG_NOTIFY_TRANSMITTING(status)	DGAS_DEBUG_NOTIFY(status, DGAS_DEBUG_MODE_TRANSMITTING)

#define DGAS_DEBUG_MSG_LEN						128

/************************ FreeRTOS *********************/

#define TASK_DGAS_DEBUG_PRIORITY				(tskIDLE_PRIORITY + 3)
#define TASK_DGAS_DEBUG_STACK_SIZE				(configMINIMAL_STACK_SIZE * 4)


// stream buffer constants
#define DGAS_DEBUG_STREAM_BUFFER_LEN			64
#define DGAS_DEBUG_STREAM_BUFFER_TRIG_LEVEL		1

extern StreamBufferHandle_t streamDebug;

#endif /* DGOS_INCLUDE_DGAS_DEBUG_H_ */
