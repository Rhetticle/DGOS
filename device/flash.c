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
#else
	if (HAL_QSPI_Command(&qspiFlash, cmd, FLASH_COMMAND_TIMEOUT) != HAL_OK) {
		return DEV_WRITE_ERROR;
	}
#endif /* FLASH_USE_FREERTOS */
	return DEV_OK;
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
#else
	if (HAL_QSPI_Receive(&qspiFlash, dest, timeout) != HAL_OK) {
		return DEV_READ_ERROR;
	}
#endif /* FLASH_USE_FREERTOS */
	return DEV_OK;
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
#else
	if (HAL_QSPI_Transmit(&qspiFlash, data, FLASH_DATA_TIMEOUT) != HAL_OK) {
		return DEV_WRITE_ERROR;
	}
#endif /* FLASH_USE_FREERTOS */
	return DEV_OK;
}

/**
 * Issue an instruction with arguments to flash IC over QSPI
 *
 * fin: Flash instruction to send
 * dMode: Data mode (e.g. FLASH_INSTRUCTION_DATA_MODE_QUAD)
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_instruction(FlashInstruction* fin, uint32_t dMode, uint32_t aMode) {
	QSPI_CommandTypeDef cmd = {0};
	DeviceStatus status;
	uint32_t total = fin->argCount + fin->expect;

	// constant QSPI_Command settings
	cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;

	// instruction setup
	cmd.Instruction = fin->opCode;
	// address setup
	cmd.AddressMode = aMode;

	if (aMode != FLASH_INSTRUCTION_ADDRESS_MODE_NONE) {
		cmd.Address = fin->addr;
		cmd.AddressSize = QSPI_ADDRESS_24_BITS; // all addresses are 24-bits
	}
	// data setup
	cmd.DataMode = dMode;

	if (total != 0) {
		cmd.NbData = total;
	}
	cmd.DummyCycles = fin->dummy;

	if ((status = flash_command(&cmd)) != DEV_OK) {
		return status;
	}

	if (fin->argCount != 0) {
		// instruction has some arguments so send after command
		if ((status = flash_data(fin->args, fin->argCount)) != DEV_OK) {
			return status;
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
DeviceStatus flash_read_mem(uint8_t* dest, uint32_t size, uint32_t addr) {
	FlashInstruction fin = {0};
	DeviceStatus status;

	fin.addr = addr;
	fin.expect = size;
	fin.dummy = FLASH_READ_QUAD_OUTPUT_DUMMY_CLOCKS;
	fin.opCode = FLASH_READ_FAST_QUAD_OUTPUT;

	if ((status = flash_instruction(&fin, FLASH_INSTRUCTION_DATA_MODE_QUAD,
				FLASH_INSTRUCTION_ADDRESS_MODE_SINGLE)) != DEV_OK) {
		return status;
	}
	// wait for flash to respond with data
	if ((status = flash_receive(dest, FLASH_DATA_TIMEOUT)) != DEV_OK) {
		return status;
	}

	return DEV_OK;
}

/**
 * Read a page (256 bytes) of data from flash.
 *
 * dest: Destination buffer
 * addr: Start address of page
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_read_page(uint8_t* dest, uint32_t addr) {
	return flash_read_mem(dest, FLASH_PAGE_SIZE, addr);
}

/**
 * Read a sector (4KiB) of data from flash
 *
 * dest: Destination buffer
 * addr: Start address of sector
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_read_sector(uint8_t* dest, uint32_t addr) {
	return flash_read_mem(dest, FLASH_SECTOR_SIZE, addr);
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
DeviceStatus flash_write_mem(uint8_t* data, uint32_t size, uint32_t addr) {
	FlashInstruction fin = {0};
	DeviceStatus status;

	fin.addr = addr;
	fin.args = data;
	fin.argCount = size;
	fin.opCode = FLASH_QUAD_INPUT_PAGE_PROGRAM;

	if ((status = flash_write_enable()) != DEV_OK) {
		return status;
	}

	if ((status = flash_instruction(&fin, FLASH_INSTRUCTION_DATA_MODE_QUAD,
				FLASH_INSTRUCTION_ADDRESS_MODE_SINGLE)) != DEV_OK) {
		return status;
	}

	return DEV_OK;
}

/**
 * Write a page (256 bytes) of data to flash
 *
 * data: Data to write (must be 256 bytes long)
 * addr: Address of page to write to
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_write_page(uint8_t* data, uint32_t addr) {
	return flash_write_mem(data, FLASH_PAGE_SIZE, addr);
}

/**
 * Write a sector (4 KiB) of data to flash
 *
 * data: Data to write (must be 4 KiB long)
 * addr: Address of sector to write to
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_write_sector(uint8_t* data, uint32_t addr) {
	return flash_write_mem(data, FLASH_SECTOR_SIZE, addr);
}

/**
 * Read information from flash IC e.g. JEDEC ID.
 *
 * instruction: Instruction to issue
 * dest: Destination buffer
 * size: Number of bytes in response
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_read_info(uint8_t instruction, uint8_t* dest, uint32_t size, uint32_t timeout) {
	FlashInstruction fin = {0};
	DeviceStatus status;

	fin.opCode = instruction;
	fin.expect = size;

	if ((status = flash_instruction(&fin, FLASH_INSTRUCTION_DATA_MODE_SINGLE,
			FLASH_INSTRUCTION_ADDRESS_MODE_SINGLE)) != DEV_OK) {
		return status;
	}

	// wait for chip response
	if ((status = flash_receive(dest, timeout)) != DEV_OK) {
		return status;
	}
	return DEV_OK;
}

/**
 * Read 8-bit register from flash IC.
 *
 * regInstr: Instruction opcode for accessing desired register
 * timeout: Timeout to use whilst receiving
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_read_reg(uint8_t regInstr, uint8_t* dest, uint32_t timeout) {
	return flash_read_info(regInstr, dest, sizeof(uint8_t), timeout);
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
	FlashInstruction fin = {0};
	DeviceStatus status;

	fin.opCode = regInstr;
	fin.argCount = sizeof(uint8_t);
	fin.args = value;

	// need to enable writing in order to modify register value
	if ((status = flash_write_enable()) != DEV_OK) {
		return status;
	}
	// issue instruction with arguments
	if ((status = flash_instruction(&fin, FLASH_INSTRUCTION_DATA_MODE_SINGLE,
			FLASH_INSTRUCTION_ADDRESS_MODE_NONE)) != DEV_OK) {
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
DeviceStatus flash_wait_on_flag(uint8_t regInstr, uint8_t bit, DevFlagOpt opt, uint32_t timeout) {
	DeviceStatus status;
	uint8_t regVal;

	// read initial register value
	if ((status = flash_read_reg(regInstr, &regVal, timeout)) != DEV_OK) {
		return status;
	}
	uint32_t tickStart = HAL_GetTick();

	if (opt == DEV_FLAG_OPT_SET) {
		// loop until bit is set or until we timeout or read fails
		while(!(regVal & bit)) {
			if ((status = flash_read_reg(regInstr, &regVal, timeout)) != DEV_OK) {
				return status;
			}
			if ((HAL_GetTick() - tickStart) > timeout) {
				return DEV_TIMEOUT;
			}
		}
	} else if (opt == DEV_FLAG_OPT_CLEAR){
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
 * Wait on busy flag of flash IC. Busy flag is set while IC is completing
 * an internal operation such as memory write, block erase etc
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_wait_on_busy(void) {
	// busy bit could be set for upwards of 20 seconds for long operations
	// such as chip erase so we will use FLASH_MAX_TIMEOUT
	return flash_wait_on_flag(FLASH_READ_STAT_REG_ONE, BUSY,
					DEV_FLAG_OPT_CLEAR, FLASH_MAX_TIMEOUT);
}

/**
 * Enable QSPI interface of flash chip itself
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_enable_qspi(void) {
	DeviceStatus status;
	uint8_t statRegTwo, statRegThree;

	// read current value of status register two
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

	// read current value of status register three
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
 * Enable memory mapped mode for the flash IC. This will map the address space
 * of the external flash to the address space of the STM32 starting from
 * 0x90000000.
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_enable_memory_mapped(void) {
	QSPI_CommandTypeDef cmd = {0};
	QSPI_MemoryMappedTypeDef conf = {0};
	HAL_StatusTypeDef status;
	// The command we give when going into memory mapped mode
	// will be the command issued when memory is accessed, so
	// we want it to be a read command. This means while the IC
	// is in memory mapped mode it cannot be written to

	// we want reading to be as fast as possible so will use
	// FLASH_READ_QUAD_IO instruction which sends address
	// using quad IO pins as well as data
	cmd.Instruction = FLASH_READ_QUAD_IO;
	cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd.Address = 0;
	cmd.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd.AddressMode = QSPI_ADDRESS_4_LINES;
	cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
	cmd.AlternateBytesSize = sizeof(uint8_t);
	cmd.AlternateBytes = FLASH_READ_QUAD_IO_ALTERNATE_BYTE;
	cmd.DataMode = QSPI_DATA_4_LINES;
	cmd.DummyCycles = FLASH_READ_QUAD_IO_DUMMY_CLOCKS;
	cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	// disable read timeout
	conf.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
	conf.TimeOutPeriod = 0;

	if ((status = HAL_QSPI_MemoryMapped(&qspiFlash, &cmd, &conf)) != HAL_OK) {
		return DEV_WRITE_ERROR;
	}

	return DEV_OK;
}

/**
 * Disable memory mapped mode for flash IC.
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_disable_memory_mapped(void) {
	// simply re-init flash IC
	return flash_init();
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

/**
 * Issue write enable instruction to flash memory chip in preparation
 * for writing data
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_write_enable(void) {
	FlashInstruction fin = {0};
	DeviceStatus status;

	fin.opCode = FLASH_WRITE_ENABLE;

	if ((status = flash_instruction(&fin, FLASH_INSTRUCTION_DATA_MODE_NONE,
					FLASH_INSTRUCTION_ADDRESS_MODE_NONE)) != DEV_OK) {
		return status;
	}

	return flash_wait_on_flag(FLASH_READ_STAT_REG_ONE, WEL, DEV_FLAG_OPT_SET, 100);
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
	FlashInstruction fin = {0};
	DeviceStatus status;

	// We must give an address of 0x000000 along with instruction
	// to get ID, check datasheet
	fin.opCode = FLASH_MFR_ID;
	fin.expect = sizeof(uint16_t);
	fin.addr = 0x000000;

	if ((status = flash_instruction(&fin, FLASH_INSTRUCTION_DATA_MODE_SINGLE,
								FLASH_INSTRUCTION_ADDRESS_MODE_SINGLE)) != DEV_OK) {
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
 * Read 16-bit JEDEC ID of flash IC
 *
 * jedec: Pointer to variable to store ID
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_get_jedec_id(uint16_t* jedec) {
	DeviceStatus status;
	// we will issue the JEDEC ID instruction
	// note that the manufacturer ID will also be given
	// in the response along with the 16-bit JEDEC so
	// will receive 24-bits total
	uint8_t resp[FLASH_ID_JEDEC_RESPONSE_SIZE];

	if ((status = flash_read_info(FLASH_JEDEC_ID, resp,
									sizeof(resp), FLASH_DATA_TIMEOUT)) != DEV_OK) {
		return status;
	}
	// resp[0] is manufacturer ID
	// resp[1] is upper byte of JEDEC
	// resp[2] is lower byte of JEDEC

	*jedec = (resp[1] << 8) | resp[2];
	return DEV_OK;
}

/**
 * Erase entire flash memory array
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_chip_erase(void) {
	FlashInstruction fin = {0};
	DeviceStatus status;

	fin.opCode = FLASH_CHIP_ERASE;

	// enable writing
	if ((status = flash_write_enable()) != DEV_OK) {
		return status;
	}

	if ((status = flash_instruction(&fin, FLASH_INSTRUCTION_DATA_MODE_NONE,
								FLASH_INSTRUCTION_ADDRESS_MODE_NONE)) != DEV_OK) {
		return status;
	}

	// wait for BUSY bit to clear meaning chip erase is done
	// Chip erase may take a while so use max delay
	return flash_wait_on_busy();
}

/**
 * Erase a given 4KiB sector in flash chip. This is the smallest
 * erasable quantity.
 *
 * sector: start address of sector to erase
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_sector_erase(uint32_t sector) {
	FlashInstruction fin = {0};
	DeviceStatus status;

	fin.addr = sector;
	fin.opCode = FLASH_SECTOR_ERASE;

	// enable writing
	if ((status = flash_write_enable()) != DEV_OK) {
		return status;
	}

	// issue erase sector instruction, no data
	if ((status = flash_instruction(&fin, FLASH_INSTRUCTION_DATA_MODE_NONE,
					FLASH_INSTRUCTION_ADDRESS_MODE_SINGLE)) != DEV_OK) {
		return status;
	}

	return flash_wait_on_busy();
}

/**
 * Erase a 32KiB block from flash IC.
 *
 * block: Start address of block to erase
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_block_erase_32k(uint32_t block) {
	FlashInstruction fin = {0};
	DeviceStatus status;

	fin.addr = block;
	fin.opCode = FLASH_BLOCK_ERASE_32K;

	// enable writing
	if ((status = flash_write_enable()) != DEV_OK) {
		return status;
	}

	if ((status = flash_instruction(&fin, FLASH_INSTRUCTION_DATA_MODE_NONE,
					FLASH_INSTRUCTION_ADDRESS_MODE_SINGLE)) != DEV_OK) {
		return status;
	}
	return flash_wait_on_busy();
}

/**
 * Erase a 64KiB block from flash IC
 *
 * block: Start address of block to erase
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_block_erase_64k(uint32_t block) {
	FlashInstruction fin = {0};
	DeviceStatus status;

	fin.addr = block;
	fin.opCode = FLASH_BLOCK_ERASE_64K;

	// enable writing
	if ((status = flash_write_enable()) != DEV_OK) {
		return status;
	}

	if ((status = flash_instruction(&fin, FLASH_INSTRUCTION_DATA_MODE_NONE,
					FLASH_INSTRUCTION_ADDRESS_MODE_SINGLE)) != DEV_OK) {
		return status;
	}
	return flash_wait_on_busy();
}

/**
 * Run a  simple self test on the external flash hardware.
 * This does not test the entire array only a section (100 pages)
 *
 * pCount: Number of pages (256 byte chunks) to check starting from
 * 		   address 0
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_self_test(uint32_t pCount) {
	DeviceStatus status;
	uint8_t tmp[FLASH_PAGE_SIZE];

	for (uint32_t i = 0; i < pCount; i++) {
		if ((status = flash_read_page(tmp, i * FLASH_PAGE_SIZE)) != DEV_OK) {
			return status;
		}
	}
	return DEV_OK;
}

/**
 * Run a self test on entire memory array of external flash
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_self_test_entire(void) {
	return flash_self_test(FLASH_PAGE_COUNT);
}

/**
 * Write a 64 byte chunk to flash memory
 *
 * chunk: Chunk to write
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_write_chunk(FlashChunk* chunk, uint8_t* data) {
	DeviceStatus stat;

	if (chunk->cSize > FLASH_WRITE_REQ_DATA_MAX) {
		return DEV_ERROR;
	}
	if ((stat = flash_write_mem(data, chunk->cSize, chunk->cAddr) != DEV_OK)) {
		return stat;
	}
	return DEV_OK;
}

/**
 * Read a 64 byte chunk from flash memory
 *
 * dest: Destination struct to store data
 *
 * Return: Status indicating success or failure
 * */
DeviceStatus flash_read_chunk(FlashChunk* read, uint8_t* dest) {
	DeviceStatus stat;

	if (read->cSize > FLASH_READ_REQ_DATA_MAX) {
		return DEV_ERROR;
	}
	if ((stat = flash_read_mem(dest, read->cSize, read->cAddr)) != DEV_OK) {
		return stat;
	}
	return DEV_OK;
}

#ifdef FLASH_USE_FREERTOS
/**
 * Thread function for flash controller task
 *
 * Return: None
 * */
static void task_flash(void) {
	FlashWriteReq wReq = {0};
	FlashReadReq rReq = {0};
	// buffer for data for read requests
	uint8_t rData[FLASH_READ_REQ_DATA_MAX];

	flash_init_hardware();
	queueFlashWrite = xQueueCreate(FLASH_WRITE_QUEUE_LENGTH,
									sizeof(FlashWriteReq));
	queueFlashRead = xQueueCreate(FLASH_READ_QUEUE_LENGTH,
									sizeof(FlashReadReq));
	for(;;) {
		if (xQueueReceive(queueFlashWrite, &wReq, 0) == pdTRUE) {
			if (flash_write_chunk(&wReq.wChunk, wReq.wData) != DEV_OK) {
				// let dgas_sys know
			}
		}
		if (xQueueReceive(queueFlashRead, &rReq, 0) == pdTRUE) {
			if (flash_read_chunk(&rReq.rChunk, rData) != DEV_OK) {
				// let dgas_sys know
			} else {
				// read successfull so send to destination queue
				xQueueSend(rReq.rDest, rData, 0);
			}
		}
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
