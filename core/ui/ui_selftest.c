/*
 * ui_selftest.c
 *
 *  Created on: 20 Dec. 2025
 *      Author: rhett
 */

#include <dgas_types.h>
#include <dgas_ui.h>
#include <ui_selftest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
 * Hide self test report objects (objects are hidden until test is done)
 *
 * Return: None
 * */
static void ui_selftest_hide_report_objs(void) {
	ui_selftest_clear_report_textareas();
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
 * Show self test progress bar object
 *
 * Return: None
 * */
static void ui_selftest_progbar_show(void) {
	lv_obj_clear_flag(objects.self_test_progress_bar, LV_OBJ_FLAG_HIDDEN);
}

/**
 * Hide self test progress bar object
 *
 * Return: None
 * */
static void ui_selftest_progbar_hide(void) {
	lv_obj_add_flag(objects.self_test_progress_bar, LV_OBJ_FLAG_HIDDEN);
}

/**
 * Hide the self test run button
 *
 * Return: None
 * */
static void ui_selftest_run_btn_hide(void) {
	lv_obj_add_flag(objects.self_test_run_btn, LV_OBJ_FLAG_HIDDEN);
}

/**
 * Show the self test run button
 *
 * Return: None
 * */
static void ui_selftest_run_btn_show(void) {
	lv_obj_clear_flag(objects.self_test_run_btn, LV_OBJ_FLAG_HIDDEN);
}

/**
 * Display statistic of memory device
 *
 * mDesc: Memory test descriptor
 * ta: Textarea to display results to
 *
 * Return: None
 * */
static void ui_selftest_display_memory_device_statistics(UISelfTestMemStats* mStat, lv_obj_t* ta) {
	char statistic[DEVICE_STATISTIC_MSG_MAX];

	sprintf(statistic, "Read Time: %ldms\n", mStat->mReadTime);
	lv_textarea_add_text(ta, statistic);

	sprintf(statistic, "Speed: %ldMiB/s\n", mStat->mSpeed);
	lv_textarea_add_text(ta, statistic);

	sprintf(statistic, "Used: %ldKiB\n", mStat->mUsed);
	lv_textarea_add_text(ta, statistic);

	sprintf(statistic, "Test Time: %ldms\n", mStat->mTime);
	lv_textarea_add_text(ta, statistic);
}

/**
 * Display accelerometer statistics
 *
 * aDesc: Accelerometer test descriptor
 *
 * Return: None
 * */
static void ui_selftest_display_accelerometer_statistics(UISelfTestAccStats* aStat) {
	char statistic[DEVICE_STATISTIC_MSG_MAX];

	sprintf(statistic, "WhoAmI: 0x%X\n", aStat->aWhoAmI);
	lv_textarea_add_text(objects.self_test_accel_textarea, statistic);

	sprintf(statistic, "X-Axis: %.1f g\n", aStat->aX);
	lv_textarea_add_text(objects.self_test_accel_textarea, statistic);
	sprintf(statistic, "Y-Axis: %.1f g\n", aStat->aY);
	lv_textarea_add_text(objects.self_test_accel_textarea, statistic);
	sprintf(statistic, "Z-Axis: %.1f g\n", aStat->aZ);
	lv_textarea_add_text(objects.self_test_accel_textarea, statistic);
}

/**
 * Display results of a memory device self test
 *
 * mDesc: Pointer to memory test descriptor of device
 * ta: Textarea to display results to
 *
 * Return: None
 * */
static void ui_selftest_display_memory_device_report(UISelfTestMemStats* mStat, lv_obj_t* ta) {
	lv_textarea_add_text(ta, "#00FF00 PASS#\n");
	ui_selftest_display_memory_device_statistics(mStat, ta);

}

/**
 * Display accelerometer self test report
 *
 * aDesc: Accelerometer test descriptor
 *
 * Return: None
 * */
static void ui_selftest_display_accelerometer_report(UISelfTestAccStats* aStat) {
	lv_textarea_add_text(objects.self_test_accel_textarea, "#00FF00 PASS#\n");
	ui_selftest_display_accelerometer_statistics(aStat);

}

/**
 * Perform UI updates for running self test (Hide "Run" button
 * and show progress bar)
 *
 * Return: None
 * */
static void ui_selftest_run(void) {
	lv_obj_add_flag(objects.self_test_run_btn, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.self_test_progress_bar, LV_OBJ_FLAG_HIDDEN);
}

/**
 * Update progress bar of self test UI
 *
 * val: Value to set progress bar to
 *
 * Return: None
 * */
static void ui_selftest_progress(int32_t val) {
	lv_bar_set_value(objects.self_test_progress_bar, val, LV_ANIM_OFF);
}

/**
 * Display self test reports onto self test UI
 *
 * rep: Self test report to display
 *
 * Return: None
 * */
static void ui_selftest_show(UISelfTestReport* rep) {
	ui_selftest_show_report_objs();
	ui_selftest_display_accelerometer_report(&rep->sAcc);
	//ui_selftest_display_memory_device_report(&rep->sDram, objects.self_test_dram_textarea);
	//ui_selftest_display_memory_device_report(&rep->sFlash, objects.self_test_flash_textarea);
}

/**
 *
 * */
static void ui_selftest_reset(void) {
	ui_selftest_hide_report_objs();
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
		case UI_CMD_SELFTEST_RUN:
			ui_selftest_run();
			break;
		case UI_CMD_SELFTEST_PROGRESS: {
			int32_t val;
			memcpy(&val, req->uData, sizeof(int32_t));
			ui_selftest_progress(val);
			break;
		}
		case UI_CMD_SELFTEST_SHOW: {
			UISelfTestReport rep = {0};
			memcpy(req->uData, &rep, sizeof(UISelfTestReport));
			ui_selftest_show(&rep);
			break;
		}
		case UI_CMD_SELFTEST_RESET:
			ui_selftest_reset();
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

	if (cmd == UI_CMD_SELFTEST_SHOW) {
		memcpy(req.uData, (UISelfTestReport*) arg, sizeof(UISelfTestReport));
	}
	ui_make_request(&req);
}

/**
 * Initialise the self test UI
 * */
void ui_selftest_init(void) {
	ui_request_register_callback(UI_SUBSYS_SELFTEST, &ui_selftest_handle_request);
}
