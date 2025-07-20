/*
 * dgas_debug.c
 *
 *  Created on: 19 Jul. 2025
 *      Author: Rhett Humphreys
 *
 *  Debug controller for DGAS. Used for live OBD-II message debugging. When an OBD-II bus is sending
 *  or receiving data it should echo the byte to the debug stream buffer (streamDebug). Once the bus
 *  has finished sending/receiving it should notify the debugger at which point the stream buffer will be
 *  flushed and a message can be sent to the debug UI.
 */

#include <dgas_debug.h>
#include <dgas_obd.h>
#include <kwp.h>
#include <dgas_ui.h>

// Task handle for debug task
static TaskHandle_t taskHandleDebug;
// stream for receiving bytes being sent over OBD-II bus. It is the bus's responsibility
// to ensure it tells the debugger what it is sending and receiving
StreamBufferHandle_t streamDebug;

/**
 * Get task handle of debug task
 *
 * Return: Task handle of debug task
 * */
TaskHandle_t task_dgas_debug_get_handle(void) {
	return taskHandleDebug;
}

/**
 * Pause DGAS debuggger
 *
 * Return: None
 * */
void dgas_debug_pause(void) {
	vTaskSuspend(taskHandleDebug);
}

/**
 * Resume DGAS debugger
 *
 * Return: None
 * */
void dgas_debug_resume(void) {
	vTaskResume(taskHandleDebug);
}

/**
 * Extract the OBD mode from data sent over bus
 *
 * data: Data received from debug stream
 *
 * Return: OBDMode of request
 * */
OBDMode dgas_debug_get_obd_mode(uint8_t* data) {
	BusID active = obd_get_active_bus();

	if (active == BUS_ID_KWP) {
		return data[KWP_OBD_MODE_INDEX];
	}
}

/**
 * Add debug message header to message. Header shows status icon, bus type
 * and request type.
 * */
void dgas_debug_add_header(char* dest, BusStatus status, uint32_t mode) {

}

/**
 * Add OBD-II debug data to message
 *
 * dest: Message buffer to store data to
 *
 * Return: None
 * */
void dgas_debug_add_data(char* dest) {

}

/**
 * Build a message to display in debug window
 *
 * status: Status received from notification
 * mode: Bus mode, either transmit or receive
 *
 * Return: None
 * */
void dgas_debug_build_message(BusStatus status, uint32_t mode) {
	char message[DGAS_DEBUG_MSG_LEN];
	dgas_debug_add_header(message, status, mode);
	dgas_debug_add_data(message);
}

/**
 * Handle a task notification
 *
 * noti: Notification value
 *
 * Return: None
 * */
void dgas_debug_handle_notification(uint32_t noti) {
	BusStatus status = noti & DGAS_DEBUG_STATUS_MASK;
	uint32_t mode = noti & DGAS_DEBUG_MODE_MASK;

	dgas_debug_build_message(status, mode);
}

/**
 * Thread function for debug task
 *
 * Return: None
 * */
void task_debug(void) {
	uint32_t notiVal;
	streamDebug = xStreamBufferCreate(DGAS_DEBUG_STREAM_BUFFER_LEN, DGAS_DEBUG_STREAM_BUFFER_TRIG_LEVEL);

	for(;;) {
		if ((notiVal = ulTaskNotifyTake(pdTRUE, 10))) {
			dgas_debug_handle_notification(notiVal);
		}
		vTaskDelay(10);
	}
}

/**
 * Initialiase DGAS debug task
 *
 * Return: None
 * */
void task_dgas_debug_init(void) {
	xTaskCreate((void*) &task_debug, "Debug", TASK_DGAS_DEBUG_STACK_SIZE, NULL,
			TASK_DGAS_DEBUG_PRIORITY, &taskHandleDebug);
}


