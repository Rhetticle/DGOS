/*
 * ui_dtc.c
 *
 *  Created on: 8 Jan. 2026
 *      Author: rhett
 */

#include <ui_dtc.h>
#include <dgas_ui.h>

/**
 * Display DTC report onto DTC UI.
 *
 * rep: Report to display:
 *
 * Return: None
 * */
static ui_dtc_show_report(UIDTCReport* rep) {

}

/**
 * Handle DTC related UI request
 *
 * req: UI request to handle
 *
 * Return: None
 * */
static void ui_dtc_handle_request(UIRequest* req) {
	switch (req->uCmd) {
		case UI_CMD_DTC_SHOW: {
			UIDTCReport rep = {0};
			memcpy(&rep, req->uData, sizeof(UIDTCReport));
			ui_dtc_show_report(&rep);
			break;
		}
		default:
			break;

	}
}

/**
 * Make DTC UI related request.
 *
 * cmd: UI command to execute
 * arg: Optional argument
 *
 * Return: None
 * */
void ui_dtc_make_request(UICmd cmd, void* arg) {
	UIRequest req = {0};

	req.uCmd = cmd;
	req.uSys = UI_SUBSYS_DTC;

	if (cmd == UI_CMD_DTC_SHOW) {
		memcpy(req.uData, (UIDTCReport*) arg, sizeof(UIDTCReport));
	}
	ui_make_request(&req);
}

/**
 * Initialise DTC UI.
 *
 * Return: None
 * */
void ui_dtc_init(void) {
	ui_request_register_callback(UI_SUBSYS_DTC, &ui_dtc_handle_request);
}


