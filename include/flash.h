/*
 * flash.h
 *
 *  Created on: 23 Jun. 2025
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_FLASH_H_
#define DGOS_INCLUDE_FLASH_H_

#if 1 // set to 1 if wish to use FreeRTOS with this driver
#define FLASH_USE_FREERTOS
#endif

#include <device.h>
#include <stm32f7xx.h>
#include <stdbool.h>

#ifdef FLASH_USE_FREERTOS
#include <FreeRTOS.h>
#include <queue.h>
#endif /* FLASH_USE_FREERTOS */

#ifdef FLASH_USE_FREERTOS
extern QueueHandle_t queueFlashWrite;
extern QueueHandle_t queueFlashRead;
#endif /* FLASH_USE_FREERTOS */

typedef struct{
	uint8_t data[64];
	uint32_t id;
}FlashChunk;

// Pin and ports for QSPI
#define FLASH_QSPI_NCS_PORT      	GPIOB
#define FLASH_QSPI_NCS_PIN       	GPIO_PIN_6
#define FLASH_QSPI_NCS_AF			GPIO_AF10_QUADSPI
#define FLASH_QSPI_CLK_PORT      	GPIOB
#define FLASH_QSPI_CLK_PIN       	GPIO_PIN_2
#define FLASH_QSPI_CLK_AF			GPIO_AF9_QUADSPI
#define FLASH_QSPI_IO_ZERO_PORT  	GPIOF
#define FLASH_QSPI_IO_ZERO_PIN   	GPIO_PIN_8
#define FLASH_QSPI_IO_ZERO_AF		GPIO_AF10_QUADSPI
#define FLASH_QSPI_IO_ONE_PORT   	GPIOF
#define FLASH_QSPI_IO_ONE_PIN    	GPIO_PIN_9
#define FLASH_QSPI_IO_ONE_AF		GPIO_AF10_QUADSPI
#define FLASH_QSPI_IO_TWO_PORT   	GPIOE
#define FLASH_QSPI_IO_TWO_PIN    	GPIO_PIN_2
#define FLASH_QSPI_IO_TWO_AF		GPIO_AF9_QUADSPI
#define FLASH_QSPI_IO_THREE_PORT 	GPIOF
#define FLASH_QSPI_IO_THREE_PIN  	GPIO_PIN_6
#define FLASH_QSPI_IO_THREE_AF		GPIO_AF9_QUADSPI

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
#define FLASH_QSPI_CLOCK_PRESCALER (8 - 1)
// number of bytes needed in FIFO in before reading
#define FLASH_QSPI_FIFO_THRESHOLD 4
// flash size of memory array (this is given as the number of address bits)
// W25Q256JVEIQ is 32MiB so need 25 bits
#define FLASH_QSPI_MEMORY_SIZE (25)

// Flash QSPI instance
#ifndef DGAS_CONFIG_FLASH_QSPI_INSTANCE
#define FLASH_QSPI_INSTANCE QUADSPI
#define __FLASH_QSPI_INSTANCE_CLK_EN() __HAL_RCC_QSPI_CLK_ENABLE()
#else
#define FLASH_QSPI_INSTANCE DGAS_CONFIG_FLASH_QSPI_INSTANCE
#define __FLASH_QSPI_INSTANCE_CLK_EN() DGAS_CONFIG_FLASH_QSPI_INSTANCE_CLK_EN()
#endif /* DGAS_CONFIG_FLASH_QSPI_INSTANCE */

#define FLASH_COMMAND_TIMEOUT 	100 // 100ms timeout on commands
#define FLASH_DATA_TIMEOUT		100 // 100ms timeout on data
#define FLASH_MAX_TIMEOUT		HAL_MAX_DELAY

/******************************** FLASH MEMORY INSTRUCTION OPCODES ***********************************/


// Write and program related instructions

#define FLASH_WRITE_ENABLE 								0x06
#define FLASH_WRITE_ENABLE_VOLATILE 					0x50
#define FLASH_WRITE_DISABLE 							0x04
#define FLASH_WRITE_STAT_REG_ONE 						0x01
#define FLASH_WRITE_STAT_REG_TWO 						0x31
#define FLASH_WRITE_STAT_REG_THREE 						0x11
#define FLASH_WRITE_EXT_ADDR 							0xC5
#define FLASH_PAGE_PROGRAM 							 	0x02
#define FLASH_PAGE_PROGRAM_FOUR_BYTE_ADDR 			 	0x12
#define FLASH_QUAD_INPUT_PAGE_PROGRAM 				 	0x32
#define FLASH_QUAD_INPUT_PAGE_PROGRAM_FOUR_BYTE_ADDR 	0x34

// Read related instructions
#define FLASH_READ_STAT_REG_ONE 						0x05
#define FLASH_READ_STAT_REG_TWO 						0x35
#define FLASH_READ_STAT_REG_THREE 						0x15
#define FLASH_READ_EXT_ADDR 							0xC8
#define FLASH_READ 										0x03
#define FLASH_READ_FOUR_BYTE_ADDR 						0x13
#define FLASH_READ_FAST 								0x0B
#define FLASH_READ_FAST_FOUR_BYTE_ADDR 					0x0C
#define FLASH_READ_FAST_DUAL_OUTPUT 					0x3B
#define FLASH_READ_FAST_DUAL_OUTPUT_FOUR_BYTE_ADDR   	0x3C
#define FLASH_READ_FAST_QUAD_OUTPUT 				 	0x6B
#define FLASH_READ_FAST_QUAD_OUTPUT_FOUR_BYTE_ADDR   	0x6C
#define FLASH_READ_FAST_DUAL_IO 					 	0xBB
#define FLASH_READ_FAST_DUAL_IO_FOUR_BYTE_ADDR 		 	0xBC
#define FLASH_READ_QUAD_IO 							 	0xEB
#define FLASH_READ_QUAD_IO_FOUR_BYTE_ADDR 			 	0xEC

// erase related instructions
#define FLASH_SECTOR_ERASE 							 	0x20
#define FLASH_SECTOR_ERASE_FOUR_BYTE_ADDR 			 	0x21
#define FLASH_BLOCK_ERASE_32K 						 	0x52
#define FLASH_BLOCK_ERASE_64K 						 	0xD8
#define FLASH_BLOCK_ERASE_64K_FOUR_BYTE_ADDR 		 	0xDC
#define FLASH_CHIP_ERASE 								0xC7 // can also use 0x60
#define FLASH_SUSPEND 									0x75
#define FLASH_RESUME 									0x7A

// ID related instructions
#define FLASH_DEVICE_ID 								0xAB
#define FLASH_MFR_ID 									0x90
#define FLASH_MFR_ID_DUAL_OUTPUT 						0x92
#define FLASH_MFS_ID_QUAD_OUTPUT 						0x94
#define FLASH_UNIQUE_ID 								0x4B
#define FLASH_JEDEC_ID 									0x9F

// security related instructions
#define FLASH_READ_SFDP 								0x5A
#define FLASH_ERASE_SECURITY 							0x44
#define FLASH_PROGRAM_SECURITY 							0x42
#define FLASH_READ_SECURITY 							0x48
#define FLASH_INDIVIDUAL_BLOCK_LOCK 					0x36
#define FLASH_INDIVIDUAL_BLOCK_UNLOCK 					0x39
#define FLASH_READ_BLOCK_LOCK 							0x3D
#define FLASH_GLOBAL_BLOCK_LOCK 						0x7E
#define FLASH_GLOBAL_BLOCK_UNLOCK 						0x98

// reset instructions
#define FLASH_ENABLE_RESET 								0x98
#define FLASH_RESET 									0x99

// miscellaneous instructions
#define FLASH_ENTER_FOUR_BYTE_ADDR_MODE 				0xB7
#define FLASH_EXIT_FOUR_BYTE_ADDR_MODE 					0xE9
#define FLASH_SET_BURST_WITH_WRAP 					 	0x77
#define FLASH_POWER_DOWN 								0xB9


/*********************** FLASH REGISTER BIT POSITIONS *****************************/


// Status register one
#define BUSY 	1 << 0
#define WEL  	1 << 1
#define BP0  	1 << 2
#define BP1  	1 << 3
#define BP2		1 << 4
#define BP3		1 << 5
#define TB		1 << 6
#define SRP		1 << 7

// Status register two
// NOTE: Lock bits LB1 -> LB3 are one time programmable (OTP)
// Programming these bits to 1 will make their corresponding pages
// read only !!! THIS IS NOT REVERSABLE !!!
#define SRL		1 << 0
#define QE		1 << 1
/******** BIT POS 2 RESERVED ****/
#define LB1		1 << 3
#define LB2		1 << 4
#define LB3		1 << 5
#define CMP		1 << 6
#define SUS		1 << 7

// Status register three
#define ADS		1 << 0
#define ADP		1 << 1
#define WPS		1 << 2
/******** BIT POS 3 RESERVED ****/
/******** BIT POS 4 RESERVED ****/
#define DRV0	1 << 5
#define DRV1	1 << 6
/******** BIT POS 7 RESERVED ****/


/******************** READ Related Constants ********************/


#define FLASH_READ_DUMMY_CLOCKS		6 // need a few dummy clocks before data is sent


/***************** INSTRUCTION Related Constants *****************/

#define FLASH_INSTRUCTION_NO_ARGS	0
#define FLASH_INSTRUCTION_NO_EXPECT	0
#define FLASH_INSTRUCTION_NO_DUMMY	0

/********************** ID Related Constants ********************/


// check datasheet
#define FLASH_ID_DEVICE_ID_VALUE	0x18
#define FLASH_ID_MFR_ID_VALUE		0xEF
// masks to extract device and manufacturer IDs from 16-bit ID
#define FLASH_DEVICE_ID_MASK		0xFF00
#define FLASH_MFR_ID_MASK			0x00FF

#define FLASH_ID_EXTRACT_DEVICE_ID(id)		((id & FLASH_DEVICE_ID_MASK) >> 8)
#define FLASH_ID_EXTRACT_MFR_ID(id)			((id & FLASH_MFR_ID_MASK))

#define FLASH_ID_JEDEC_RESPONSE_SIZE 3

#ifndef DGAS_CONFIG_FLASH_CHUNK_SIZE
#define FLASH_CHUNK_SIZE 64
#else
#define FLASH_CHUNK_SIZE DGAS_CONFIG_CHUNK_SIZE
#endif /* DGAS_CONFIG_FLASH_CHUNK_SIZE */

#ifdef FLASH_USE_FREERTOS
#define FLASH_WRITE_QUEUE_LENGTH 10
#define FLASH_READ_QUEUE_LENGTH  10

#define DGAS_TASK_FLASH_PRIORITY (tskIDLE_PRIORITY + 1)
#define DGAS_TASK_FLASH_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#endif /* FLASH_USE_FREERTOS */



/****************** Function Prototypes ************************/



void flash_init_hardware(void);
DeviceStatus flash_command(QSPI_CommandTypeDef* cmd);
DeviceStatus flash_receive(uint8_t* dest, uint32_t timeout);
DeviceStatus flash_data(uint8_t* data, uint32_t size);
DeviceStatus flash_instruction(uint8_t instruction, uint8_t* args, uint32_t argCount,
								uint32_t expect, uint32_t dummy);
DeviceStatus flash_read_info(uint8_t instruction, uint8_t* dest, uint32_t size, uint32_t timeout);
DeviceStatus flash_read_reg(uint8_t regInstr, uint8_t* dest, uint32_t timeout);
DeviceStatus flash_write_reg(uint8_t regInstr, uint8_t* value);
DeviceStatus flash_wait_on_flag(uint8_t regInstr, uint8_t bit, DevFlagOpt set, uint32_t timeout);
DeviceStatus flash_read_mem(uint8_t* dest, uint32_t size, uint32_t addr);
DeviceStatus flash_write_enable(void);
DeviceStatus flash_get_device_id(uint8_t* id);
DeviceStatus flash_get_mfr_id(uint8_t* mfr);
DeviceStatus flash_get_jedec_id(uint16_t* jedec);
DeviceStatus flash_write(uint8_t* data, uint32_t size, uint32_t addr);
DeviceStatus flash_erase_chip(void);
DeviceStatus flash_write_chunk(FlashChunk* chunk);
DeviceStatus flash_read_chunk(FlashChunk* dest);
DeviceStatus flash_enable_qspi(void);
DeviceStatus flash_init(void);

#ifdef FLASH_USE_FREERTOS
void task_init_flash(void);
TaskHandle_t task_flash_get_handle(void);
#endif /* FLASH_USE_FREERTOS */

#endif /* DGOS_INCLUDE_FLASH_H_ */
