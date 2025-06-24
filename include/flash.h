/*
 * flash.h
 *
 *  Created on: 23 Jun. 2025
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_FLASH_H_
#define DGOS_INCLUDE_FLASH_H_

#include <dgas_types.h>

extern QueueHandle_t queueFlashWrite;
extern QueueHandle_t queueFlashRead;

typedef struct{
	uint8_t data[64];
	uint32_t id;
}FlashChunk;

#define FLASH_WRITE_QUEUE_LENGTH 10
#define FLASH_READ_QUEUE_LENGTH  10

#ifndef DGAS_CONFIG_FLASH_QSPI_INSTANCE
#define FLASH_QSPI_INSTANCE QUADSPI
#else
#define FLASH_QSPI_INSTANCE DGAS_CONFIG_FLASH_QSPI_INSTANCE
#endif

#define DGAS_TASK_FLASH_PRIORITY (tskIDLE_PRIORITY + 1)
#define DGAS_TASK_FLASH_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)


#endif /* DGOS_INCLUDE_FLASH_H_ */
