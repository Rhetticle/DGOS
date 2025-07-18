/*
 * dram.c
 *
 *  Created on: Jun 27, 2025
 *      Author: rhett
 */

#include <dram.h>
#include <device.h>
#include <FreeRTOS.h>
#include <task.h>

// DRAM controller handle
static SDRAM_HandleTypeDef dramHandle;
// Pointer to use to access dram
static __IO uint8_t* dramPtr = (uint8_t*) DRAM_START_ADDR;

/**
 * Initialise GPIO pins needed for DRAM
 *
 * Return: None
 * */
static void dram_gpio_init(void) {
	__DRAM_GPIO_CLK_EN_ALL();
	GPIO_InitTypeDef init = {0};

	// ports A, C, D, E, F and G are used
	// check header to see which pins share same port

	// port A pins
	init.Alternate = GPIO_AF12_FMC;
	init.Speed = GPIO_SPEED_HIGH;
	init.Mode = GPIO_MODE_AF_PP;
	init.Pull = GPIO_NOPULL;
	init.Pin = DRAM_PIN_SDNWE;

	HAL_GPIO_Init(GPIOA, &init);

	// port C pins
	init.Pin = DRAM_PIN_SDNE0 | DRAM_PIN_SDCKE0;

	HAL_GPIO_Init(GPIOC, &init);

	// port D pins
	init.Pin = DRAM_PIN_D0 | DRAM_PIN_D1 |
			   DRAM_PIN_D2 | DRAM_PIN_D3 |
			   DRAM_PIN_D13 | DRAM_PIN_D14 |
			   DRAM_PIN_D15;

	HAL_GPIO_Init(GPIOD, &init);

	// port E pins
	init.Pin = DRAM_PIN_D4 | DRAM_PIN_D5 |
			   DRAM_PIN_D6 | DRAM_PIN_D7 |
			   DRAM_PIN_D8 | DRAM_PIN_D9 |
			   DRAM_PIN_D10 | DRAM_PIN_D11 |
			   DRAM_PIN_D12;

	HAL_GPIO_Init(GPIOE, &init);

	// port F pins
	init.Pin = DRAM_PIN_A0 | DRAM_PIN_A1 |
			   DRAM_PIN_A2 | DRAM_PIN_A3 |
			   DRAM_PIN_A4 | DRAM_PIN_A5 |
			   DRAM_PIN_A6 | DRAM_PIN_A7 |
			   DRAM_PIN_A8 | DRAM_PIN_A9;

	HAL_GPIO_Init(GPIOF, &init);

	// port G pins
	init.Pin = DRAM_PIN_A10 | DRAM_PIN_A11 |
			   DRAM_PIN_BA0 | DRAM_PIN_SDCLK |
			   DRAM_PIN_SDNCAS;

	HAL_GPIO_Init(GPIOG, &init);
}

/**
 * Initialise flexible memory controller (FMC) peripheral
 * for usage with external DRAM
 *
 * Return: None
 * */
static void dram_fmc_init(void) {
	__DRAM_FMC_CLK_EN();
	FMC_SDRAM_TimingTypeDef timing = {0};

	// initialise dramHandle struct
	dramHandle.Instance = DRAM_FMC_INSTANCE;
	dramHandle.Init.SDBank = FMC_SDRAM_BANK1;
	dramHandle.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
	dramHandle.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_11;
	dramHandle.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
	dramHandle.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_2;
	dramHandle.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_2;
	dramHandle.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
	dramHandle.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_2;
	dramHandle.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
	dramHandle.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;

	// initialise timing struct with timing values
	timing.ExitSelfRefreshDelay = DRAM_FMC_SELF_REFRESH_DELAY;
	timing.LoadToActiveDelay = DRAM_FMC_LOAD_TO_ACTIVE_DELAY;
	timing.RCDDelay = DRAM_FMC_RCD_DELAY;
	timing.RPDelay = DRAM_FMC_RP_DELAY;
	timing.RowCycleDelay = DRAM_FMC_ROW_CYCLE_DELAY;
	timing.SelfRefreshTime = DRAM_FMC_SELF_REFRESH_TIME;
	timing.WriteRecoveryTime = DRAM_FMC_WRITE_RECOVERY_TIME;

	HAL_SDRAM_Init(&dramHandle, &timing);
}

/**
 * Initialise hardware required for external DRAM
 *
 * Return: None
 * */
void dram_init_hardware(void) {
	dram_gpio_init();
	dram_fmc_init();
}

/**
 * Initialise external DRAM for use
 *
 * Return: None
 * */
void dram_init(void) {
	dram_init_hardware();
	// FMC is initialised now we must initialise the DRAM IC itself
	FMC_SDRAM_CommandTypeDef cmd = {0};

	cmd.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
	cmd.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1; // external DRAM is in BANK1
	cmd.AutoRefreshNumber = 1;
	cmd.ModeRegisterDefinition = 0;

	HAL_SDRAM_SendCommand(&dramHandle, &cmd, 10);
	vTaskDelay(1);

	// enable precharge all (PALL)
	cmd.CommandMode = FMC_SDRAM_CMD_PALL;
	HAL_SDRAM_SendCommand(&dramHandle, &cmd, 10);

	// enable auto refresh
	cmd.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
	HAL_SDRAM_SendCommand(&dramHandle, &cmd, 10);

	// set external memory mode register
	cmd.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
	cmd.ModeRegisterDefinition =  (uint32_t)0 | 0<<3 | 2<<4 | 0<<7 | 1<<9;
	HAL_SDRAM_SendCommand(&dramHandle, &cmd, 10);

	HAL_SDRAM_ProgramRefreshRate(&dramHandle, DRAM_CELL_REFRESH_RATE);
	// clear DRAM contents to 0x00 to clear any contents from previous power-up
	dram_clear();
}

/**
 * Fill DRAM section with given 8-bit value
 *
 * startAddr: Start address (DRAM relative)
 * endAddr: End address
 * value: Value to fill with
 *
 * Return: None
 * */
void dram_fill_section(uint32_t startAddr, uint32_t endAddr, uint16_t value) {
	if (startAddr > endAddr) {
		return;
	}
	for (uint32_t i = startAddr; i < endAddr; i++) {
		*(dramPtr + i) = value;
	}
}

/**
 * Clear section of external DRAM to zero
 *
 * startAddr: Address to begin clearing from
 * endAddr: Address to end clearing
 *
 * Return: None
 * */
void dram_clear_section(uint32_t startAddr, uint32_t endAddr) {
	dram_fill_section(startAddr, endAddr, 0x00);
}

/**
 * Clear entire DRAM array to zero
 *
 * Return: None
 * */
void dram_clear(void) {
	// clear entire DRAM
	dram_clear_section(0, DRAM_SIZE - 1);
}

/**
 * Read a section of data from external DRAM.
 *
 * startAddr: Address to begin reading from
 * dest: Destination buffer
 * size: Number of bytes to read
 *
 * Return: None
 * */
void dram_read_section(uint32_t startAddr, uint8_t* dest, uint32_t size) {
	for (uint32_t i = startAddr; i < startAddr + size; i++) {
		dest[i - startAddr] = *(dramPtr + i);
	}
}
