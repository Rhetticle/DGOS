/*
 * ui_debug.c
 *
 *  Created on: 2 Jan. 2026
 *      Author: rhett
 */

#include <ui_debug.h>
#include <dgas_ui.h>
#include <stdio.h>
#include <string.h>

/**
 * Flush message to debug window UI
 *
 * flush: Message to flush
 *
 * Return: None
 * */
static void ui_debug_flush(UIDebugFlush* flush) {
	lv_obj_t* label = lv_textarea_get_label(objects.obd2_debug_textarea);


	if (strlen(lv_label_get_text(label)) > UI_DEBUG_WINDOW_MAX_TEXT) {
		// clear textarea to stop text from overflowing
		lv_textarea_set_text(objects.obd2_debug_textarea, "");
	}
	lv_textarea_add_text(objects.obd2_debug_textarea, flush->dStr);

}

/**
 * Handle debug UI request:
 *
 * req: Request to handle
 *
 * Return: None
 * */
static void ui_debug_handle_request(UIRequest* req) {
	switch(req->uCmd) {
		case UI_CMD_DEBUG_FLUSH: {
			if (lv_screen_active() != objects.obd2_debug) {
				// debug screen not active, no point flushing
				return;
			}
			UIDebugFlush dFlush = {0};
			memcpy(&dFlush, req->uData, sizeof(UIDebugFlush));
			ui_debug_flush(&dFlush);
		}
		default:
			break;
	}
}

/**
 * Make debug UI related request
 *
 * cmd: UI command to execute
 * arg: Optional argument
 *
 * Return: None
 * */
void ui_debug_make_request(UICmd cmd, void* arg) {
	UIRequest req = {0};
	req.uCmd = cmd;
	req.uSys = UI_SUBSYS_DEBUG;

	if (arg != NULL) {
		if (cmd == UI_CMD_DEBUG_FLUSH) {
			memcpy(req.uData, (UIDebugFlush*) arg, sizeof(UIDebugFlush));
		}
	}
}

/**
 * Initialise debug UI
 *
 * Return: None
 * */
void ui_debug_init(void) {
	ui_request_register_callback(UI_SUBSYS_DEBUG, &ui_debug_handle_request);
}
