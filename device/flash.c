/**
 * Flash memory controller and peripheral driver for DGAS external flash
 * memory. This driver is suited for the W25Q256JVEIQ NOR memory IC with
 * a QSPI interface.
 *
 * Author: Rhett Humphreys
 * */

#include <flash.h>
#include <device.h>
#include <stdbool.h>

#ifdef FLASH_USE_FREERTOS
// stores task handle for flash memory controller
static TaskHandle_t taskHandleFlash;
// Queue for sending data to be written to flash
QueueHandle_t queueFlashWrite;
// Queue for sending data read from flash
QueueHandle_t queueFlashRead;
#endif /* FLASH_USE_FREERTOS */
// QSPI handle
static QSPI_HandleTypeDef qspiFlash;

/**
 * Initialise GPIO pins for flash QSPI
 *
 * Return: None
 * */
static void flash_init_gpio(void) {
	// enable GPIO peripheral Clocks
	__FLASH_QSPI_GPIO_CLK_EN_ALL();
	GPIO_InitTypeDef init = {0};

	init.Pin = FLASH_QSPI_NCS_PIN;
	init.Mode = MODE_OUTPUT;
	init.Pull = GPIO_PULLUP;
	init.Speed = GPIO_SPEED_FAST;

	HAL_GPIO_Init(FLASH_QSPI_NCS_PORT, &init);

	init.Pin = FLASH_QSPI_CLK_PIN;
	init.Mode = GPIO_MODE_AF_PP;
	init.Pull = GPIO_NOPULL;
	init.Alternate = GPIO_AF10_QUADSPI;

	HAL_GPIO_Init(FLASH_QSPI_CLK_PORT, &init);

	// initialise QSPI IO pins
	init.Pin = FLASH_QSPI_IO_ZERO_PIN;

	HAL_GPIO_Init(FLASH_QSPI_IO_ZERO_PORT, &init);

	init.Pin = FLASH_QSPI_IO_ONE_PIN;

	HAL_GPIO_Init(FLASH_QSPI_IO_ONE_PORT, &init);

	init.Pin = FLASH_QSPI_IO_TWO_PIN;

	HAL_GPIO_Init(FLASH_QSPI_IO_TWO_PORT, &init);

	init.Pin = FLASH_QSPI_IO_THREE_PIN;

	HAL_GPIO_Init(FLASH_QSPI_IO_THREE_PORT, &init);
}

/**
 * Initialise QSPI peripheral for flash memory
 *
 * Return: None
 * */
static void flash_init_qspi(void) {
	__FLASH_QSPI_INSTANCE_CLK_EN();
	// initialise QSPI peripheral
	qspiFlash.Instance = FLASH_QSPI_INSTANCE;
	qspiFlash.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
	qspiFlash.Init.ClockMode = QSPI_CLOCK_MODE_0;
	qspiFlash.Init.ClockPrescaler = FLASH_QSPI_CLOCK_PRESCALER;
	qspiFlash.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
	qspiFlash.Init.FifoThreshold = FLASH_QSPI_FIFO_THRESHOLD;
	qspiFlash.Init.FlashID = QSPI_FLASH_ID_1;
	qspiFlash.Init.FlashSize = FLASH_QSPI_MEMORY_SIZE;
	qspiFlash.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;

	HAL_QSPI_Init(&qspiFlash);
}

/**
 * Initialise flash memory hardware
 *
 * Return: None
 * */
void flash_init_hardware(void) {
#ifdef FLASH_USE_FREERTOS
	taskENTER_CRITICAL();
#endif /* FLASH_USE_FREERTOS */
	flash_init_gpio();
	flash_init_qspi();
#ifdef FLASH_USE_FREERTOS
	taskEXIT_CRITICAL();
#endif /* FLASH_USE_FREERTOS */
}

#ifdef FLASH_USE_FREERTOS
/**
 * Get task handle of flash controller task
 *
 * Return: Task handle of flash controller task
 * */
TaskHandle_t task_flash_get_handle(void) {
	return taskHandleFlash;
}
#endif /* FLASH_USE_FREERTOS */

/**
 * Issue command to flash IC
 *
 * cmd: QSPI command to issue
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_command(QSPI_CommandTypeDef* cmd) {
#ifdef FLASH_USE_FREERTOS
	taskENTER_CRITICAL();
	if (HAL_QSPI_Command(&qspiFlash, cmd, FLASH_COMMAND_TIMEOUT) != HAL_OK) {
		taskEXIT_CRITICAL();
		return DEV_WRITE_ERROR;
	}
	taskEXIT_CRITICAL();
	return DEV_OK;
#else
	if (HAL_QSPI_Command(&qspiFlash, cmd, FLASH_COMMAND_TIMEOUT) != HAL_OK) {
		return DEV_WRITE_ERROR;
	}
	return DEV_OK;
#endif /* FLASH_USE_FREERTOS */
}

/**
 * Receive data from flash IC.
 *
 * dest: Destination buffer
 * timeout: Timeout to use whilst receiving
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_receive(uint8_t* dest, uint32_t timeout) {
#ifdef FLASH_USE_FREERTOS
	taskENTER_CRITICAL();
	if (HAL_QSPI_Receive(&qspiFlash, dest, timeout) != HAL_OK) {
		taskEXIT_CRITICAL();
		return DEV_READ_ERROR;
	}
	taskEXIT_CRITICAL();
	return DEV_OK;
#else
	if (HAL_QSPI_Receive(&qspiFlash, dest, timeout) != HAL_OK) {
		return DEV_READ_ERROR;
	}
	return DEV_OK;
#endif /* FLASH_USE_FREERTOS */
}

/**
 * Read register from flash IC.
 *
 * regInstr: Instruction opcode for accessing desired register
 * timeout: Timeout to use whilst receiving
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_read_reg(uint8_t regInstr, uint8_t* dest, uint32_t timeout) {
	QSPI_CommandTypeDef cmd = {0};
	DeviceStatus status;

	cmd.Instruction = regInstr;
	cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd.Address = QSPI_ADDRESS_NONE;
	cmd.AddressMode = QSPI_ADDRESS_4_LINES;
	cmd.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd.DataMode = QSPI_DATA_NONE;
	cmd.DummyCycles = 0U;
	cmd.NbData = 0;
	cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if ((status = flash_command(&cmd)) != DEV_OK) {
		return status;
	}

	if ((status = flash_receive(dest, FLASH_COMMAND_TIMEOUT)) != DEV_OK) {
		return status;
	}
	return DEV_OK;
}

/**
 * Wait on flag to be set within status register of flash IC
 *
 * regInstr: Instruction to read status register of interest
 * bit: Bit to wait on being set or clear
 * set: True if wait for bit to be set (1), false for clear (0)
 * timeout: Timeout to use whilst waiting
 *
 * Return: Status indicating succcess or failure
 * */
DeviceStatus flash_wait_on_flag(uint8_t regInstr, uint8_t bit, bool set, uint32_t timeout) {
	DeviceStatus status;
	uint8_t regVal;

	// read initial register value
	if ((status = flash_read_reg(regInstr, &regVal, timeout)) != DEV_OK) {
		return status;
	}
	uint32_t tickStart = HAL_GetTick();

	if (set) {
		// loop until bit is set or until we timeout or read fails
		while(!(regVal & bit)) {
			if ((status = flash_read_reg(regInstr, &regVal, timeout)) != DEV_OK) {
				return status;
			}
			if ((HAL_GetTick() - tickStart) > timeout) {
				return DEV_TIMEOUT;
			}
		}
	} else {
		// loop until bit is cleared or until we timeout or read fails
		while(regVal & bit) {
			if ((status = flash_read_reg(regInstr, &regVal, timeout)) != DEV_OK) {
				return status;
			}
			if ((HAL_GetTick() - tickStart) > timeout) {
				return DEV_TIMEOUT;
			}
		}
	}

	return DEV_OK;
}

/**
 * Read data from flash memory.
 *
 * dest: Destination buffer
 * size: Number of bytes to read
 * addr: Address to begin reading from
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_read(uint8_t* dest, uint32_t size, uint32_t addr) {
	QSPI_CommandTypeDef cmd = {0};
	DeviceStatus status;

	cmd.Instruction = FLASH_READ_FAST_QUAD_OUTPUT;
	cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd.Address = addr;
	cmd.AddressMode = QSPI_ADDRESS_4_LINES;
	cmd.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd.DataMode = QSPI_DATA_4_LINES;
	cmd.DummyCycles = 6U;
	cmd.NbData = size;
	cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if ((status = flash_command(&cmd)) != DEV_OK) {
		return status;
	}

	if (HAL_QSPI_Receive(&qspiFlash, dest, FLASH_COMMAND_TIMEOUT) != HAL_OK) {
		return DEV_READ_ERROR;
	}

	return DEV_OK;
}

/**
 * Issue write enable instruction to flash memory chip in preparation
 * for writing data
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_write_enable(void) {
	QSPI_CommandTypeDef cmd = {0};
	DeviceStatus status;

	cmd.Instruction = FLASH_WRITE_ENABLE;
	cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd.Address = QSPI_ADDRESS_NONE;
	cmd.AddressMode = QSPI_ADDRESS_4_LINES;
	cmd.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd.DataMode = QSPI_DATA_4_LINES;
	cmd.DummyCycles = 0;
	cmd.NbData = QSPI_DATA_NONE;
	cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if ((status = flash_command(&cmd)) != DEV_OK) {
		return status;
	}

	return flash_wait_on_flag(FLASH_READ_STAT_REG_ONE, WEL, true, 100);
}

/**
 * Write data to flash memory
 *
 * data: Data to write
 * size: Number of bytes to write
 * addr: Address to begin writing from
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_write(uint8_t* data, uint32_t size, uint32_t addr) {
	return DEV_OK;
}

/**
 * Erase entire flash memory array
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_erase_chip(void) {
	return DEV_OK;
}

/**
 * Write a 64 byte chunk to flash memory
 *
 * chunk: Chunk to write
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_write_chunk(FlashChunk* chunk) {
	return DEV_OK;
}

/**
 * Read a 64 byte chunk from flash memory
 *
 * dest: Destination struct to store data
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_read_chunk(FlashChunk* dest) {
	return DEV_OK;
}

#ifdef FLASH_USE_FREERTOS
/**
 * Thread function for flash controller task
 *
 * Return: None
 * */
static void task_flash(void) {
	flash_init_hardware();

	queueFlashWrite = xQueueCreate(FLASH_WRITE_QUEUE_LENGTH,
									sizeof(FlashChunk));
	queueFlashRead = xQueueCreate(FLASH_READ_QUEUE_LENGTH,
									sizeof(FlashChunk));
	for(;;) {

	}
}

/**
 * Initialise flash controller task
 *
 * Return: None
 * */
void task_init_flash(void) {
	xTaskCreate((void*) &task_flash, "TaskFlash", DGAS_TASK_FLASH_STACK_SIZE,
				NULL, DGAS_TASK_FLASH_PRIORITY, &taskHandleFlash);
}
#endif /* FLASH_USE_FREERTOS */
