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
	// ports B, E and F are being used

	// common settings between all QSPI pins
	init.Mode = GPIO_MODE_AF_PP;
	init.Pull = GPIO_NOPULL;
	init.Speed = GPIO_SPEED_HIGH;

	init.Pin = FLASH_QSPI_NCS_PIN;
	init.Alternate = FLASH_QSPI_NCS_AF;

	HAL_GPIO_Init(FLASH_QSPI_NCS_PORT, &init);

	init.Pin = FLASH_QSPI_CLK_PIN;
	init.Alternate = FLASH_QSPI_CLK_AF;

	HAL_GPIO_Init(FLASH_QSPI_CLK_PORT, &init);

	init.Pin = FLASH_QSPI_IO_ZERO_PIN;
	init.Alternate = FLASH_QSPI_IO_ZERO_AF;

	HAL_GPIO_Init(FLASH_QSPI_IO_ZERO_PORT, &init);

	init.Pin = FLASH_QSPI_IO_ONE_PIN;
	init.Alternate = FLASH_QSPI_IO_ONE_AF;

	HAL_GPIO_Init(FLASH_QSPI_IO_ONE_PORT, &init);

	init.Pin = FLASH_QSPI_IO_TWO_PIN;
	init.Alternate = FLASH_QSPI_IO_TWO_AF;

	HAL_GPIO_Init(FLASH_QSPI_IO_TWO_PORT, &init);

	init.Pin = FLASH_QSPI_IO_THREE_PIN;
	init.Alternate = FLASH_QSPI_IO_THREE_AF;

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
	qspiFlash.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_6_CYCLE;
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
	flash_init_qspi();
	flash_init_gpio();
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
 * Write data to flash IC to accompany command or write actual
 * flash data.
 *
 * data: Data to write
 * size: Number of bytes to write
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_data(uint8_t* data, uint32_t size) {
#ifdef FLASH_USE_FREERTOS
	taskENTER_CRITICAL();
	if (HAL_QSPI_Transmit(&qspiFlash, data, FLASH_DATA_TIMEOUT) != HAL_OK) {
		taskEXIT_CRITICAL();
		return DEV_WRITE_ERROR;
	}
	taskEXIT_CRITICAL();
	return DEV_OK;
#else
	if (HAL_QSPI_Transmit(&qspiFlash, data, FLASH_DATA_TIMEOUT) != HAL_OK) {
		return DEV_WRITE_ERROR;
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
	cmd.AddressMode = QSPI_ADDRESS_NONE;
	cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd.DataMode = QSPI_DATA_1_LINE;
	cmd.DummyCycles = 0;
	cmd.NbData = 1;
	cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	// send read instruction over QSPI
	if ((status = flash_command(&cmd)) != DEV_OK) {
		return status;
	}

	// wait for response from chip
	if ((status = flash_receive(dest, FLASH_COMMAND_TIMEOUT)) != DEV_OK) {
		return status;
	}
	return DEV_OK;
}

/**
 * Write value to flash IC register
 *
 * regInstr: Instruction opcode to access register
 * value: Value to write to register
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_write_reg(uint8_t regInstr, uint8_t* value) {
	QSPI_CommandTypeDef cmd = {0};
	DeviceStatus status;

	cmd.Instruction = regInstr;
	cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd.AddressMode = QSPI_ADDRESS_NONE;
	cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd.DataMode = QSPI_DATA_1_LINE;
	cmd.DummyCycles = 0;
	cmd.NbData = 1;
	cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	// need to enable writing in order to modify register value
	if ((status = flash_write_enable()) != DEV_OK) {
		return status;
	}
	// send command over QSPI
	if ((status = flash_command(&cmd)) != DEV_OK) {
		return status;
	}

	// send argument for command
	if ((status = flash_data(value, sizeof(uint8_t))) != DEV_OK) {
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
	cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd.DataMode = QSPI_DATA_NONE;
	cmd.DummyCycles = 0;
	cmd.NbData = 0;
	cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if ((status = flash_command(&cmd)) != DEV_OK) {
		return status;
	}

	return flash_wait_on_flag(FLASH_READ_STAT_REG_ONE, WEL, true, 100);
}

/**
 * Get the device and manufacturer ID of flash IC as a single
 * 16 bit value. The lower byte will be manufacturer ID and the
 * upper byte will be device ID
 *
 * id: Variable to store resulting ID in
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_get_device_and_mfr_id(uint16_t* id) {
	DeviceStatus status;
	QSPI_CommandTypeDef cmd = {0};

	cmd.Instruction = FLASH_MFR_ID;
	cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	// need to give address 0x0000 along with instruction
	// check datasheet
	cmd.Address = 0x0000;
	cmd.AddressMode = QSPI_ADDRESS_1_LINE;
	cmd.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd.DataMode = QSPI_DATA_1_LINE;
	cmd.DummyCycles = 0;
	cmd.NbData = sizeof(uint16_t);
	cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	// send command over QSPI
	if ((status = flash_command(&cmd)) != DEV_OK) {
		return status;
	}

	// receive response from chip
	if ((status = flash_receive((uint8_t*) id, 100)) != DEV_OK) {
		return status;
	}
	return DEV_OK;
}

/**
 * Read device ID of flash IC.
 *
 * id: Pointer to variable to store ID
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_get_device_id(uint8_t* id) {
	uint16_t mfrDevId;
	DeviceStatus status;

	if ((status = flash_get_device_and_mfr_id(&mfrDevId)) != DEV_OK) {
		return status;
	}
	*id = FLASH_ID_EXTRACT_DEVICE_ID(mfrDevId);
	return DEV_OK;
}

/**
 * Read manufacturer ID of flash IC.
 *
 * mfs: Pointer to variable to store ID
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_get_mfr_id(uint8_t* mfr) {
	uint16_t mfrDevId;
	DeviceStatus status;

	if ((status = flash_get_device_and_mfr_id(&mfrDevId)) != DEV_OK) {
		return status;
	}
	*mfr = FLASH_ID_EXTRACT_MFR_ID(mfrDevId);
	return DEV_OK;
}

/**
 * Read JEDEC ID of flash IC
 *
 * jedec: Pointer to variable to store ID
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_get_jedec_id(uint8_t* jedec) {
	return DEV_OK;
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

/**
 * Enable QSPI interface of flash chip itself
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_enable_qspi(void) {
	DeviceStatus status;
	uint8_t statRegTwo, statRegThree;

	if ((status = flash_read_reg(FLASH_READ_STAT_REG_TWO, &statRegTwo, 100)) != DEV_OK) {
		return status;
	}
	// have read status register two value
	// now set QE bit to 1 to enable QSPI
	statRegTwo |= (1 << QE);

	if ((status = flash_write_reg(FLASH_WRITE_STAT_REG_TWO, &statRegTwo)) != DEV_OK) {
		return status;
	}
	// next operation is optional but will increase output drive
	// strength of IO pins of flash IC to give better rise times
	// we will set the DRV[1:0] bits of status register 3 to zero
	// for maximum drive strength

	if ((status = flash_read_reg(FLASH_READ_STAT_REG_THREE, &statRegThree, 100)) != DEV_OK) {
		return status;
	}
	// clear DRV[1:0] to zero
	statRegThree &= ~((1 << DRV0) | (1 << DRV1));

	// write new value to status register three
	if ((status = flash_write_reg(FLASH_WRITE_STAT_REG_THREE, &statRegThree)) != DEV_OK) {
		return status;
	}

	return DEV_OK;
}

/**
 * Initialise flash IC and related peripherals for use
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_init(void) {
	// initialise GPIO pins and QSPI peripheral
	flash_init_hardware();
	// enable the flash ICs QSPI interface
	return flash_enable_qspi();
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
