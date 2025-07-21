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
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Task handle for debug task
static TaskHandle_t taskHandleDebug;
// stream for receiving bytes being sent over OBD-II bus. It is the bus's responsibility
// to ensure it tells the debugger what it is sending and receiving
StreamBufferHandle_t streamDebug;
// flag to keep track of whether debug window is paused or not
static bool debugPaused;
// buffer for storing debug messages before flushing to UI
static char debugLog[DGAS_DEBUG_BUFF_LEN];

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
	debugPaused = true;
}

/**
 * Resume DGAS debugger
 *
 * Return: None
 * */
void dgas_debug_resume(void) {
	debugPaused = false;
}

/**
 * Add string to destination string
 *
 * dest: Destination string
 * add: String to add
 *
 * Return: None
 * */
void dgas_debug_add_str(char* dest, char* add) {
	uint32_t len = strlen(dest);
	sprintf(dest + len, add);
}

/**
 * Add newline character to debug message
 *
 * dest: Destination buffer
 *
 * Return: None
 * */
void dgas_debug_add_newline(char* dest) {
	dgas_debug_add_str(dest, "\n");
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
	return OBD_MODE_LIVE;
}

/**
 * Add OBD mode to debug message header
 *
 * dest: Destination buffer
 * mode: OBD mode
 *
 * Return: None
 * */
void dgas_debug_add_obd_mode(char* dest, OBDMode mode) {
	if (mode == OBD_MODE_LIVE) {
		dgas_debug_add_str(dest, "#FF7200 {PID}#");
	} else if (mode == OBD_MODE_DTC) {
		dgas_debug_add_str(dest, "#FFFF00 {DTC}#");
	}
}

/**
 * Add debug message header to message. Header shows status icon, bus type
 * and request type.
 *
 * dest: Destination buffer for message
 * status: Bus status of request/response
 * oMode: OBDMode of request e.g. PID, DTC etc
 * direction: DGAS_DEBUG_MODE_TRANSMITTING, DGAS_DEBUG_MODE_RECEIVING etc
 *
 * Return: None
 * */
void dgas_debug_add_header(char* dest, BusStatus status, OBDMode oMode, uint32_t direction) {
	// add icon (tick or cross)
	if (status == BUS_OK) {
		dgas_debug_add_str(dest, "#00FF00 \uf00c#");
	}  else {
		dgas_debug_add_str(dest, "#FF0000 \uf00d#");
	}

	if (direction == DGAS_DEBUG_DIRECTION_TRANSMITTING) {
		dgas_debug_add_str(dest, "#00FF00 [DGAS]#");
	} else {
		dgas_debug_add_str(dest, "#AA11F0 [ECU]#");
	}
	// add OBD mode
	dgas_debug_add_obd_mode(dest, oMode);
}

/**
 * Add OBD-II debug data to message
 *
 * dest: Message buffer to store data to
 * data: Data received over stream buffer
 *
 * Return: None
 * */
void dgas_debug_add_data(char* dest, uint8_t* data, uint32_t dataLen) {
	dgas_debug_add_str(dest, "{");

	for (uint32_t i = 0; i < dataLen; i++) {
		if (data[i] <= 0xF) {
			sprintf(dest + strlen(dest), "0x0%X", data[i]);
		} else {
			sprintf(dest + strlen(dest), "0x%X", data[i]);
		}
		if (i != dataLen - 1) {
			dgas_debug_add_str(dest, ", ");
		} else {
			dgas_debug_add_str(dest, "}\n");
		}
	}
}

/**
 * Add error string to debug message briefly explaining error
 *
 * dest: Destination buffer
 * status: Status indicating success or failure
 *
 * Return: None
 * */
void dgas_debug_add_error(char* dest, BusStatus status) {
	if (status == BUS_BUFFER_ERROR) {
		dgas_debug_add_str(dest, "#FF0000 ERROR: BUFFER#");
	} else if (status == BUS_CHECKSUM_ERROR) {
		dgas_debug_add_str(dest, "#FF0000 ERROR: CHECKSUM#");
	} else if (status == BUS_ECHO_ERROR) {
		dgas_debug_add_str(dest, "#FF0000 ERROR: ECHO#");
	} else if (status == BUS_TX_ERROR) {
		dgas_debug_add_str(dest, "#FF0000 ERROR: TX ERROR#");
	} else if (status == BUS_RX_ERROR) {
		dgas_debug_add_str(dest, "#FF0000 ERROR: RX ERROR#");
	}
}

/**
 * Build a message to display in debug window
 *
 * status: Status received from notification
 * mode: Bus mode, either transmit or receive
 *
 * Return: None
 * */
void dgas_debug_build_message(char* message, BusStatus status, uint32_t direction) {
	uint8_t data[DGAS_DEBUG_STREAM_BUFFER_LEN];
	uint32_t dataLen;
	OBDMode mode = dgas_debug_get_obd_mode(data);

	// receive data from stream
	dataLen = xStreamBufferReceive(streamDebug, data, DGAS_DEBUG_STREAM_BUFFER_LEN, 0);

	dgas_debug_add_header(message, status, mode, direction);
	if (status == BUS_OK) {
		dgas_debug_add_data(message, data, dataLen);
	} else {
		dgas_debug_add_error(message, status);
	}
}

/**
 * Log message to debugger, add it to the debug buffer
 *
 * message: Message to add to debug log
 *
 * Return: None
 * */
void dgas_debug_log_message(char* message) {
	if (strlen(message) > strlen(debugLog) - DGAS_DEBUG_BUFF_LEN) {
		// no room for new message so clear buffer and wrap around
		memset(debugLog, 0, DGAS_DEBUG_BUFF_LEN);
	}
	// add message to debug log
	strcat(debugLog, message);
}

/**
 * Flush a debug message to the debug UI textarea
 *
 * message: Message to flush to UI
 *
 * Return: None
 * */
void dgas_debug_flush(void) {
	// only flush if debug screen is active and not paused
	lv_obj_t* label = lv_textarea_get_label(objects.obd2_debug_textarea);

	if (strlen(lv_label_get_text(label)) > DGAS_DEBUG_WINDOW_MAX_TEXT) {
		// clear textarea to stop text from overflowing
		if (ui_take_semaphore() == pdTRUE) {
			lv_textarea_set_text(objects.obd2_debug_textarea, "");
			ui_give_semaphore();
		}
	}

	if (ui_take_semaphore() == pdTRUE) {
		lv_textarea_add_text(objects.obd2_debug_textarea, debugLog);
		ui_give_semaphore();
	}
	memset(debugLog, 0, sizeof(debugLog));

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
	uint32_t direction = noti & DGAS_DEBUG_DIRECTION_MASK;
	char message[DGAS_DEBUG_MSG_LEN];

	dgas_debug_build_message(message, status, direction);
	dgas_debug_log_message(message);
}

/**
 * Thread function for debug task
 *
 * Return: None
 * */
void task_debug(void) {
	uint32_t notiVal = 0xFF;
	debugPaused = false;
	streamDebug = xStreamBufferCreate(DGAS_DEBUG_STREAM_BUFFER_LEN, DGAS_DEBUG_STREAM_BUFFER_TRIG_LEVEL);

	for(;;) {
		if ((notiVal = ulTaskNotifyTake(pdTRUE, 10))) {
			dgas_debug_handle_notification(notiVal);
		}
		if ((lv_screen_active() == objects.obd2_debug) && !debugPaused) {
			// only flush to UI if debug screen is active and debugger is not paused
			dgas_debug_flush();
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


