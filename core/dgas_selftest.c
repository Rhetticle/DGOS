/*
 * dgas_selftest.c
 *
 *  Created on: 26 Jul. 2025
 *      Author: rhett
 */

#include <dgas_selftest.h>
#include <ui_selftest.h>
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
 * Populate UISelfTestAccStats struct from accelerometer test descriptor
 *
 * dest: Destination struct
 * aDesc: Accelerometer test descriptor
 *
 * Return: None
 * */
void selftest_populate_acc_results(UISelfTestAccStats* dest, AccTestDesc* aDesc) {
	dest->aWhoAmI = aDesc->whoAmI;
	dest->aX = aDesc->aData.accX;
	dest->aY = aDesc->aData.accY;
	dest->aZ = aDesc->aData.accZ;
}

/**
 * Populate UISelfTestMemStats struct from memory device test descriptor
 *
 * dest: Destination struct
 * mDesc: Memory device test descriptor
 *
 * Return: None
 * */
void selftest_populate_mem_results(UISelfTestMemStats* dest, MemTestDesc* mDesc) {
	dest->mReadTime = mDesc->readTime;
	dest->mSpeed = memory_calc_speed(mDesc);
	dest->mTime = memory_calc_test_time(mDesc);
	dest->mUsed = memory_calc_size(mDesc);
}

/**
 * Show self test results on UI
 *
 * aDesc: Accelerometer test descriptor
 * dDesc: DRAM test descriptor
 * fDesc: Flash test descriptor
 *
 * Return: None
 * */
void selftest_show_results(AccTestDesc* aDesc, MemTestDesc* dDesc, MemTestDesc* fDesc) {
	UISelfTestReport rep = {0};
	selftest_populate_acc_results(&rep.sAcc, aDesc);
	//selftest_populate_mem_results(&rep.sDram, dDesc);
	//selftest_populate_mem_results(&rep.sFlash, fDesc);
	ui_selftest_make_request(UI_CMD_SELFTEST_SHOW, &rep);
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
	int32_t progress = 0;

	ui_selftest_make_request(UI_CMD_SELFTEST_RUN, NULL);
	// accelerometer self test
	dgas_self_test_accelerometer(&accTest);
	progress = 33;
	ui_selftest_make_request(UI_CMD_SELFTEST_PROGRESS, &progress);
	// DRAM self test
	// dgas_self_test_dram(&dramTest);
	progress = 67;
	ui_selftest_make_request(UI_CMD_SELFTEST_PROGRESS, &progress);
	// External flash self test
	//dgas_self_test_flash(&flashTest);
	progress = 100;
	ui_selftest_make_request(UI_CMD_SELFTEST_PROGRESS, &progress);

	selftest_show_results(&accTest, &dramTest, &flashTest);
	return DEV_OK;
}

void dgas_selftest_init(void) {
	ui_selftest_init();
}

/**
 * Task function for self test
 *
 * Return: None
 * */
void task_dgas_self_test(void) {

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
