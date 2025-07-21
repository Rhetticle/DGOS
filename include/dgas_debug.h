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
#include <dgas_obd.h>

#define DGAS_DEBUG_LOG_BYTE(byte)				do{if (streamDebug != NULL) {\
													xStreamBufferSend(streamDebug, &byte, 1, 0);\
												}}while(0)

#define DGAS_DEBUG_LOG_STREAM(stream, len)		do{if (streamDebug != NULL) {\
													xStreamBufferSend(streamDebug, stream, len, 0);\
												}}while(0)

#define DGAS_DEBUG_DIRECTION_RECEIVING			(1 << 31)
#define DGAS_DEBUG_DIRECTION_TRANSMITTING		(0 << 30)
#define DGAS_DEBUG_DIRECTION_MASK				(1 << 31)

#define DGAS_DEBUG_STATUS_MASK					(~(1 << 31))

#define DGAS_DEBUG_NOTIFY(status, dir)			xTaskNotify(task_dgas_debug_get_handle(), status | dir, eSetValueWithOverwrite)
#define DGAS_DEBUG_NOTIFY_RECEIVING(status)		DGAS_DEBUG_NOTIFY(status, DGAS_DEBUG_DIRECTION_RECEIVING)
#define DGAS_DEBUG_NOTIFY_TRANSMITTING(status)	DGAS_DEBUG_NOTIFY(status, DGAS_DEBUG_DIRECTION_TRANSMITTING)

#define DGAS_DEBUG_MSG_LEN						256
#define DGAS_DEBUG_BUFF_LEN						256
#define DGAS_DEBUG_WINDOW_MAX_TEXT				550

/************************ FreeRTOS *********************/

#define TASK_DGAS_DEBUG_PRIORITY				(tskIDLE_PRIORITY + 3)
#define TASK_DGAS_DEBUG_STACK_SIZE				(configMINIMAL_STACK_SIZE * 4)


// stream buffer constants
#define DGAS_DEBUG_STREAM_BUFFER_LEN			BUS_TRANSACTION_MAX
#define DGAS_DEBUG_STREAM_BUFFER_TRIG_LEVEL		1

extern StreamBufferHandle_t streamDebug;

/************************ Prototypes *********************/

TaskHandle_t task_dgas_debug_get_handle(void);
void dgas_debug_pause(void);
void dgas_debug_resume(void);
void dgas_debug_add_str(char* dest, char* add);
void dgas_debug_log_byte(uint8_t byte);
void dgas_debug_add_newline(char* dest);
OBDMode dgas_debug_get_obd_mode(uint8_t* data);
void dgas_debug_add_obd_mode(char* dest, OBDMode mode);
void dgas_debug_add_header(char* dest, BusStatus status, uint32_t direction);
void dgas_debug_add_data(char* dest, uint8_t* data, uint32_t dataLen);
void dgas_debug_add_error(char* dest, BusStatus status);
void dgas_debug_build_message(char* message, BusStatus status, uint32_t direction);
void dgas_debug_log_message(char* message);
void dgas_debug_flush(void);
void dgas_debug_handle_notification(uint32_t noti);
void task_dgas_debug_init(void);

#endif /* DGOS_INCLUDE_DGAS_DEBUG_H_ */
