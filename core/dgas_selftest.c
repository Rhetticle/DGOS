/*
 * dgas_selftest.c
 *
 *  Created on: 26 Jul. 2025
 *      Author: rhett
 */

#include <dgas_selftest.h>
#include <device.h>
#include <accelerometer.h>
#include <flash.h>
#include <dram.h>

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
	mDesc->used = dgas_self_test_mem_calc_used();
}

/**
 * Run a self test on the on-board flash IC
 *
 * mDesc: Memory test descriptor to populate with test statistics
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus dgas_self_test_flash(MemTestDesc* mDesc) {

}

/**
 * Run the complete DGAS hardware self test
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus dgas_self_test(void) {

}
