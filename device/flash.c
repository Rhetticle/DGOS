/**
 * Flash memory controller and peripheral driver for DGAS external flash
 * memory. This driver is suited for the W25Q256JVEIQ NOR memory IC with
 * a QSPI interface.
 *
 * Author: Rhett Humphreys
 * */

#include <flash.h>
#include <device.h>

// stores task handle for flash memory controller
static TaskHandle_t taskHandleFlash;
// Queue for sending data to be written to flash
QueueHandle_t queueFlashWrite;
// Queue for sending data read from flash
QueueHandle_t queueFlashRead;
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
	// initialise GPIO pins for QSPI
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
void flash_init(void) {
	flash_init_gpio();
	flash_init_qspi();
}

/**
 * Get task handle of flash controller task
 *
 * Return: Task handle of flash controller task
 * */
TaskHandle_t task_flash_get_handle(void) {
	return taskHandleFlash;
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
 * Thread function for flash controller task
 *
 * Return: None
 * */
void task_flash(void) {
	flash_init();

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

