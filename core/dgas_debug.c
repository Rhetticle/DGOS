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
// Queue for debug messages
QueueHandle_t queueDebug;
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
 * Log a message to the debugger. This should be called by bus tasks either directly or using
 * macros in dgas_debug.h
 *
 * data: Data sent or received over bus (NULL if error)
 * dataLen: Number of bytes successfully sent or received
 * status: Status of bus transaction
 * bid: BusID of bus which logged message
 * direction: Direction of transaction (transmit or receive)
 *
 * Return: None
 * */
void dgas_debug_log_msg(uint8_t* data, uint32_t dataLen, BusStatus status, BusID bid, BusDirection direction) {
	DebugMsg msg = {0};

	if (data != NULL) {
		memcpy(msg.data, data, dataLen);
	}
	msg.dataLen = dataLen;
	msg.bid = bid;
	msg.status = status;
	msg.direction = direction;

	if (queueDebug != NULL) {
		// since this will be called by bus controller we don't want to wait at all
		// so set ticks to wait to zero
		xQueueSend(queueDebug, &msg, 0);
	}
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
void dgas_debug_add_header(char* dest, BusStatus status, BusDirection direction) {
	// add icon (tick or cross)
	if (status == BUS_OK) {
		dgas_debug_add_str(dest, "#00FF00 \uf00c#");
	}  else {
		dgas_debug_add_str(dest, "#FF0000 \uf00d#");
	}

	if (direction == BUS_DIR_TRANSMIT) {
		dgas_debug_add_str(dest, "#00FF00 [DGAS]#");
	} else {
		dgas_debug_add_str(dest, "#AA11F0 [ECU]#");
	}
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
		dgas_debug_add_str(dest, "#FF0000 ERROR: BUFFER#\n");
	} else if (status == BUS_CHECKSUM_ERROR) {
		dgas_debug_add_str(dest, "#FF0000 ERROR: CHECKSUM#\n");
	} else if (status == BUS_ECHO_ERROR) {
		dgas_debug_add_str(dest, "#FF0000 ERROR: ECHO#\n");
	} else if (status == BUS_TX_ERROR) {
		dgas_debug_add_str(dest, "#FF0000 ERROR: TX ERROR#\n");
	} else if (status == BUS_RX_ERROR) {
		dgas_debug_add_str(dest, "#FF0000 ERROR: RX ERROR#\n");
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
void dgas_debug_build_message(char* message, DebugMsg* msg) {

	dgas_debug_add_header(message, msg->status, msg->direction);
	if (msg->status == BUS_OK) {
		// receive data from stream
		dgas_debug_add_data(message, msg->data, msg->dataLen);
	} else {
		dgas_debug_add_error(message, msg->status);
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
 * Handle a new debug message to log to debugger
 *
 * msg: Pointer to debug message to log
 *
 * Return: None
 * */
void dgas_debug_handle_message(DebugMsg* msg) {
	char msgStr[DGAS_DEBUG_MSG_LEN];
	memset(msgStr, 0, DGAS_DEBUG_MSG_LEN);

	dgas_debug_build_message(msgStr, msg);
	dgas_debug_log_message(msgStr);
}

/**
 * Thread function for debug task
 *
 * Return: None
 * */
void task_debug(void) {
	DebugMsg msg = {0};
	debugPaused = false;
	queueDebug = xQueueCreate(DGAS_DEBUG_QUEUE_LEN, sizeof(DebugMsg));

	for(;;) {
		if (xQueueReceive(queueDebug, &msg, 10) == pdTRUE) {
			dgas_debug_handle_message(&msg);

			if ((lv_screen_active() == objects.obd2_debug) && !debugPaused) {
				// only flush to UI if debug screen is active and debugger is not paused
				dgas_debug_flush();
			}
		}
		vTaskDelay(20);
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


