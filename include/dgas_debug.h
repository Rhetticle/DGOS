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

#define DGAS_DEBUG_MSG_LEN						256
#define DGAS_DEBUG_BUFF_LEN						256
#define DGAS_DEBUG_WINDOW_MAX_TEXT				550

/************************ FreeRTOS *********************/

#define TASK_DGAS_DEBUG_PRIORITY				(tskIDLE_PRIORITY + 3)
#define TASK_DGAS_DEBUG_STACK_SIZE				(configMINIMAL_STACK_SIZE * 8)

#define DGAS_DEBUG_QUEUE_LEN		5

extern QueueHandle_t queueDebug;

/******************* Message Logging *******************/

typedef struct {
	uint8_t data[BUS_TRANSACTION_MAX];
	uint32_t dataLen;
	BusStatus status;
	BusID bid;
	BusDirection direction;
}DebugMsg;
// macros for logging debug messages, should be called by bus control tasks
#define DGAS_DEBUG_LOG_MSG(data, len, status, bid, dir)			dgas_debug_log_msg(data, len, status, bid, dir)

#define DGAS_DEBUG_LOG_MSG_DATA(data, len, bid, dir)			DGAS_DEBUG_LOG_MSG(data, len, BUS_OK, bid, dir)
#define DGAS_DEBUG_LOG_MSG_DATA_TRANSMIT(data, len, bid)		DGAS_DEBUG_LOG_MSG_DATA(data, len, bid, BUS_DIR_TRANSMIT)
#define DGAS_DEBUG_LOG_MSG_DATA_RECEIVE(data, len, bid)			DGAS_DEBUG_LOG_MSG_DATA(data, len, bid, BUS_DIR_RECEIVE)

#define DGAS_DEBUG_LOG_MSG_ERROR(bid, status, dir)				DGAS_DEBUG_LOG_MSG(NULL, 0, status, bid, dir)
#define DGAS_DEBUG_LOG_MSG_ERROR_TRANSMIT(bid, status)			DGAS_DEBUG_LOG_MSG_ERROR(bid, status, BUS_DIR_TRANSMIT)
#define DGAS_DEBUG_LOG_MSG_ERROR_RECEIVE(bid, status)			DGAS_DEBUG_LOG_MSG_ERROR(bid, status, BUS_DIR_RECEIVE)


/************************ Prototypes *********************/

TaskHandle_t task_dgas_debug_get_handle(void);
void dgas_debug_pause(void);
void dgas_debug_resume(void);
void dgas_debug_add_str(char* dest, char* add);
void dgas_debug_log_byte(uint8_t byte);
void dgas_debug_add_newline(char* dest);
void dgas_debug_log_msg(uint8_t* data, uint32_t dataLen, BusStatus status, BusID bid, BusDirection direction);
OBDMode dgas_debug_get_obd_mode(uint8_t* data);
void dgas_debug_add_obd_mode(char* dest, OBDMode mode);
void dgas_debug_add_header(char* dest, BusStatus status, BusDirection direction);
void dgas_debug_add_data(char* dest, uint8_t* data, uint32_t dataLen);
void dgas_debug_add_error(char* dest, BusStatus status);
void dgas_debug_build_message(char* message, DebugMsg* msg);
void dgas_debug_log_message(char* message);
void dgas_debug_flush(void);
void task_dgas_debug_init(void);

#endif /* DGOS_INCLUDE_DGAS_DEBUG_H_ */
