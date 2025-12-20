/*
 * dgas_selftest.c
 *
 *  Created on: 26 Jul. 2025
 *      Author: rhett
 */

#include <dgas_selftest.h>
#include <dgas_ui.h>
#include <device.h>
#include <accelerometer.h>
#include <flash.h>
#include <dram.h>
#include <stdio.h>

static TaskHandle_t taskHandleSelfTest;

/**
 * Calculate total time taken to run memory device self test
 *
 * mDesc: Memory test descriptor
 *
 * Return: Time taken in ms
 **/
static uint32_t memory_calc_test_time(MemTestDesc* mDesc) {
	return (mDesc->readTime + mDesc->writeTime);
}

/**
 * Calculate total size of memory device
 *
 * mDesc: Memory test descriptor
 *
 * Return: Size in KiB
 * */
static uint32_t memory_calc_size(MemTestDesc* mDesc) {
	return (CONV_BYTES_TO_KIB(mDesc->used + mDesc->free));
}

/**
 * Calculate the access speed of memory device
 *
 * mDesc: Memory test descriptor
 *
 * Return: Speed in KiB/s
 * */
static uint32_t memory_calc_speed(MemTestDesc* mDesc) {
	uint32_t time = memory_calc_test_time(mDesc);
	uint32_t size = memory_calc_size(mDesc);

	return (size / CONV_MS_TO_S(time));
}

/**
 * Display statistic of memory device
 *
 * mDesc: Memory test descriptor
 * ta: Textarea to display results to
 *
 * Return: None
 * */
static void display_memory_device_statistics(MemTestDesc* mDesc, lv_obj_t* ta) {
	char statistic[DEVICE_STATISTIC_MSG_MAX];

	sprintf(statistic, "Read Time: %ldms\n", mDesc->readTime);
	lv_textarea_add_text(ta, statistic);

	sprintf(statistic, "Speed: %ldMiB/s\n", memory_calc_speed(mDesc));
	lv_textarea_add_text(ta, statistic);

	sprintf(statistic, "Used: %.1fKiB\n", memory_calc_size(mDesc));
	lv_textarea_add_text(ta, statistic);

	sprintf(statistic, "Test Time: %ldms\n", memory_calc_test_time(mDesc));
	lv_textarea_add_text(ta, statistic);
}

/**
 * Display accelerometer statistics
 *
 * aDesc: Accelerometer test descriptor
 *
 * Return: None
 * */
static void display_accelerometer_statistics(AccTestDesc* aDesc) {
	char statistic[DEVICE_STATISTIC_MSG_MAX];

	sprintf(statistic, "WhoAmI: 0x%X\n", aDesc->whoAmI);
	lv_textarea_add_text(objects.self_test_accel_textarea, statistic);

	sprintf(statistic, "X-Axis: %.1f g\n", aDesc->aData.accX);
	lv_textarea_add_text(objects.self_test_accel_textarea, statistic);
	sprintf(statistic, "Y-Axis: %.1f g\n", aDesc->aData.accY);
	lv_textarea_add_text(objects.self_test_accel_textarea, statistic);
	sprintf(statistic, "Z-Axis: %.1f g\n", aDesc->aData.accZ);
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
static void display_memory_device_report(MemTestDesc* mDesc, lv_obj_t* ta) {
	if ((mDesc->readFailAddr == 0) && (mDesc->writeFailAddr == 0)) {
		lv_textarea_add_text(ta, "#00FF00 PASS#\n");
		display_memory_device_statistics(mDesc, ta);
		return;
	}
	char errorMsg[DEVICE_REPORT_MSG_MAX];
	lv_textarea_add_text(ta, "#FF0000 ERROR#\n");

	if (mDesc->readFailAddr != 0) {
		sprintf(errorMsg, "Read failure at 0x%lX\n", mDesc->readFailAddr);
		lv_textarea_add_text(ta, errorMsg);
	}
	if (mDesc->writeFailAddr != 0) {
		sprintf(errorMsg, "Write failure at 0x%lX\n", mDesc->writeFailAddr);
		lv_textarea_add_text(ta, errorMsg);
	}
}

/**
 * Display accelerometer self test report
 *
 * aDesc: Accelerometer test descriptor
 *
 * Return: None
 * */
static void display_accelerometer_test_report(AccTestDesc* aDesc) {
	if (aDesc->whoAmI == ACC_WHO_AM_I) {
		lv_textarea_add_text(objects.self_test_accel_textarea, "#00FF00 PASS#\n");
		display_accelerometer_statistics(aDesc);
		return;
	}
	char errorMsg[DEVICE_REPORT_MSG_MAX];
	lv_textarea_add_text(objects.self_test_accel_textarea, "#FF0000 \uf00d#\n");

	if (aDesc->whoAmI != ACC_WHO_AM_I) {
		sprintf(errorMsg, "WhoAmI register value wrong, got 0x%X expected 0x%X\n",
				aDesc->whoAmI, ACC_WHO_AM_I);
		lv_textarea_add_text(objects.self_test_accel_textarea, errorMsg);
	}
}

/**
 * Get task handle of self test task
 *
 * Return: None
 * */
TaskHandle_t dgas_self_test_get_handle(void) {
	return taskHandleSelfTest;
}

/**
 * Run a complete self test on accelerometer
 *
 * aDesc: Accelerometer test descriptor struct to populate
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus dgas_self_test_accelerometer(AccTestDesc* aDesc) {
	DeviceStatus status;

	// read whoAmI register first
	if ((status = accelerometer_who_am_i(&(aDesc->whoAmI))) != DEV_OK) {
		return status;
	}

	if (aDesc->whoAmI != ACC_WHO_AM_I) {
		// whoAmI did not match factory expected value
		return DEV_ERROR;
	}

	if ((status = accelerometer_get_update(&(aDesc->aData))) != DEV_OK) {
		return status;
	}
	return DEV_OK;
}

/**
 * Run a self test on the on-board DRAM IC
 *
 * mDesc: Memory test descriptor to populate with test statistics
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus dgas_self_test_dram(MemTestDesc* mDesc) {
	// Although we can access DRAM via pointer we should not when self-testing
	// as if DRAM is faulty we may get segfault and go to hardfault handler. So
	// access DRAM using HAL_SDRAM functions
	uint32_t readTick = HAL_GetTick();

	if ((mDesc->readFailAddr = dram_test_read_access()) != 0) {
		return DEV_READ_ERROR;
	}
	// We can't test write functionality to DRAM since it stores frame buffers so we
	// can only test read access
	mDesc->readTime = HAL_GetTick() - readTick;
	mDesc->used = dram_calc_used();
	mDesc->free = dram_calc_free();
	return DEV_OK;
}

/**
 * Run a self test on the on-board flash IC
 *
 * mDesc: Memory test descriptor to populate with test statistics
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus dgas_self_test_flash(MemTestDesc* mDesc) {
	return DEV_OK;
}

/**
 * Run the complete DGAS hardware self test
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus dgas_self_test(void) {
	MemTestDesc flashTest = {0};
	MemTestDesc dramTest = {0};
	AccTestDesc accTest = {0};

	hide_report_objs();
	lv_obj_clear_flag(objects.self_test_progress_bar, LV_OBJ_FLAG_HIDDEN);
	update_progress_bar(0);
	// accelerometer self test
	dgas_self_test_accelerometer(&accTest);
	update_progress_bar(33);
	// DRAM self test
	// dgas_self_test_dram(&dramTest);
	update_progress_bar(67);
	// External flash self test
	//dgas_self_test_flash(&flashTest);
	update_progress_bar(100);

	show_report_objs();
	clear_report_textareas();
	display_memory_device_report(&dramTest, objects.self_test_dram_textarea);
	//display_memory_device_report(&flashTest, objects.self_test_flash_textarea);
	display_accelerometer_test_report(&accTest);
}

/**
 * Task function for self test
 *
 * Return: None
 * */
void task_dgas_self_test(void) {
	hide_report_objs();

	for (;;) {

	}
}

/**
 * Create self test task
 *
 * Return: None
 * */
void task_dgas_self_test_init(void) {
	xTaskCreate((void*) &task_dgas_self_test, "self test", TASK_SELF_TEST_STACK_SIZE,
			NULL, TASK_SELF_TEST_PRIORITY, &taskHandleSelfTest);
}
