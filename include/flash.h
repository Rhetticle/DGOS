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

// Pin and ports for QSPI
#define FLASH_QSPI_NCS_PORT      GPIOB
#define FLASH_QSPI_NCS_PIN       GPIO_PIN_6
#define FLASH_QSPI_CLK_PORT      GPIOB
#define FLASH_QSPI_CLK_PIN       GPIO_PIN_2
#define FLASH_QSPI_IO_ZERO_PORT  GPIOF
#define FLASH_QSPI_IO_ZERO_PIN   GPIO_PIN_8
#define FLASH_QSPI_IO_ONE_PORT   GPIOF
#define FLASH_QSPI_IO_ONE_PIN    GPIO_PIN_9
#define FLASH_QSPI_IO_TWO_PORT   GPIOE
#define FLASH_QSPI_IO_TWO_PIN    GPIO_PIN_2
#define FLASH_QSPI_IO_THREE_PORT GPIOF
#define FLASH_QSPI_IO_THREE_PIN  GPIO_PIN_6

// Clock enable macros for QSPI pins/ports
#define __FLASH_QSPI_NCS_CLK_EN()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define __FLASH_QSPI_CLK_CLK_EN() 	   __HAL_RCC_GPIOB_CLK_ENABLE()
#define __FLASH_QSPI_IO_ZERO_CLK_EN()  __HAL_RCC_GPIOF_CLK_ENABLE()
#define __FLASH_QSPI_IO_ONE_CLK_EN()   __HAL_RCC_GPIOF_CLK_ENABLE()
#define __FLASH_QSPI_IO_TWO_CLK_EN()   __HAL_RCC_GPIOE_CLK_ENABLE()
#define __FLASH_QSPI_IO_THREE_CLK_EN() __HAL_RCC_GPIOF_CLK_ENABLE()
#define __FLASH_QSPI_GPIO_CLK_EN_ALL() {__FLASH_QSPI_NCS_CLK_EN();\
								   	    __FLASH_QSPI_CLK_CLK_EN();\
										__FLASH_QSPI_IO_ZERO_CLK_EN();\
										__FLASH_QSPI_IO_ONE_CLK_EN();\
										__FLASH_QSPI_IO_TWO_CLK_EN();\
										__FLASH_QSPI_IO_THREE_CLK_EN();}

// QSPI source clock is 216MHz, flash IC can handle up to 133MHz so use prescaler of 1 (2 - 1)
#define FLASH_QSPI_CLOCK_PRESCALER (2 - 1)
// number of bytes needed in FIFO in before reading
#define FLASH_QSPI_FIFO_THRESHOLD 4
// flash size of memory array (this is given as the number of address bits minus 1)
// W25Q256JVEIQ is 32MiB so need 25 bits
#define FLASH_QSPI_MEMORY_SIZE (25 - 1)

// Flash QSPI instance
#ifndef DGAS_CONFIG_FLASH_QSPI_INSTANCE
#define FLASH_QSPI_INSTANCE QUADSPI
#define __FLASH_QSPI_INSTANCE_CLK_EN() __HAL_RCC_QSPI_CLK_ENABLE()
#else
#define FLASH_QSPI_INSTANCE DGAS_CONFIG_FLASH_QSPI_INSTANCE
#define __FLASH_QSPI_INSTANCE_CLK_EN() DGAS_CONFIG_FLASH_QSPI_INSTANCE_CLK_EN()
#endif


#ifndef DGAS_CONFIG_FLASH_CHUNK_SIZE
#define FLASH_CHUNK_SIZE 64
#else
#define FLASH_CHUNK_SIZE DGAS_CONFIG_CHUNK_SIZE
#endif /* DGAS_CONFIG_FLASH_CHUNK_SIZE */



typedef struct{
	uint8_t data[64];
	uint32_t id;
}FlashChunk;

#define FLASH_WRITE_QUEUE_LENGTH 10
#define FLASH_READ_QUEUE_LENGTH  10

#define DGAS_TASK_FLASH_PRIORITY (tskIDLE_PRIORITY + 1)
#define DGAS_TASK_FLASH_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)


#endif /* DGOS_INCLUDE_FLASH_H_ */
