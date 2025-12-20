/*
 * ui_selftest.c
 *
 *  Created on: 20 Dec. 2025
 *      Author: rhett
 */

#include <dgas_types.h>
#include <dgas_ui.h>
#include <ui_selftest.h>

/**
 * Hide self test report objects (objects are hidden until test is done)
 *
 * Return: None
 * */
static void ui_selftest_hide_report_objs(void) {
	lv_obj_add_flag(objects.self_test_progress_bar, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(objects.self_test_accel_icon, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(objects.self_test_accel_label, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(objects.self_test_accel_textarea, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(objects.self_test_dram_icon, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(objects.self_test_dram_label, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(objects.self_test_dram_textarea, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(objects.self_test_flash_icon, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(objects.self_test_flash_label, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(objects.self_test_flash_textarea, LV_OBJ_FLAG_HIDDEN);
}

/**
 * Show self test report objects
 *
 * Return: None
 * */
static void ui_selftest_show_report_objs(void) {
	lv_obj_add_flag(objects.self_test_progress_bar, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.self_test_accel_icon, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.self_test_accel_label, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.self_test_accel_textarea, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.self_test_dram_icon, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.self_test_dram_label, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.self_test_dram_textarea, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.self_test_flash_icon, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.self_test_flash_label, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.self_test_flash_textarea, LV_OBJ_FLAG_HIDDEN);
}

/**
 * Clear textareas for self test results
 *
 * Return: None
 * */
static void ui_selftest_clear_report_textareas(void) {
	lv_textarea_set_text(objects.self_test_dram_textarea, "");
	lv_textarea_set_text(objects.self_test_flash_textarea, "");
	lv_textarea_set_text(objects.self_test_accel_textarea, "");
}

/**
 * Update progress bar of self test
 *
 * val: Value to set progress bar to
 *
 * Return: None
 * */
static void ui_selftest_update_progress_bar(int32_t val) {
	lv_bar_set_value(objects.self_test_progress_bar, val, LV_ANIM_OFF);
}

/**
 * Handle a selftest related UI request
 *
 * req: Request to handle
 *
 * Return: None
 * */
static void ui_selftest_handle_request(UIRequest* req) {
	switch(req->uCmd) {
		case UI_CMD_SELFTEST_OBJS_SHOW:
			ui_selftest_show_report_objs();
			break;
		case UI_CMD_SELFTEST_OBJS_HIDE:
			ui_selftest_hide_report_objs();
			break;
		case UI_CMD_SELFTEST_UPDATE_PROGBAR:
			//ui_selftest_update_progress_bar(req->uData.selfBar);
			break;
		case UI_CMD_SELFTEST_SHOW_REPORT:
			break;
		default:
			break;
	}
}

/**
 * Make a self test UI request
 *
 * cmd: UI command to execute
 * arg: Optional argument for request
 *
 * Return: None
 * */
void ui_selftest_make_request(UICmd cmd, void* arg) {
	UIRequest req = {0};
	req.uCmd = cmd;
	req.uSys = UI_SUBSYS_SELFTEST;

	ui_make_request(&req);
}

/**
 * Initialise the self test UI
 * */
void ui_selftest_init(void) {
	ui_request_register_callback(UI_SUBSYS_SELFTEST, &ui_selftest_handle_request);
}
