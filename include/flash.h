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
#include <semphr.h>
#endif /* FLASH_USE_FREERTOS */

#ifdef FLASH_USE_FREERTOS
extern QueueHandle_t queueFlashReq;
#endif /* FLASH_USE_FREERTOS */

// Memory size constants
#define FLASH_ARRAY_SIZE			0x2000000 	// 32MiB (256MBit)
#define FLASH_SECTOR_SIZE			0x1000		// 4KiB
#define FLASH_PAGE_SIZE				0x100		// 256 bytes
#define FLASH_SECTOR_COUNT			(FLASH_ARRAY_SIZE / FLASH_SECTOR_SIZE)
#define FLASH_PAGE_COUNT			(FLASH_ARRAY_SIZE / FLASH_PAGE_SIZE)

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


// For the quad output read instruction (0x6B) we need 8 dummy clocks
#define FLASH_READ_QUAD_OUTPUT_DUMMY_CLOCKS			8
// For the quad IO instruction (0xEB) we need 4 dummy clocks
// we also need a dummy (alternate) byte which is sent after 24-bit address
// and before dummy clocks, check datasheet
#define FLASH_READ_QUAD_IO_DUMMY_CLOCKS				4
#define FLASH_READ_QUAD_IO_ALTERNATE_BYTE			0xFF


/***************** INSTRUCTION Related Constants *****************/

#define FLASH_INSTRUCTION_NO_ARGS	0
#define FLASH_INSTRUCTION_NO_EXPECT	0
#define FLASH_INSTRUCTION_NO_DUMMY	0

#define FLASH_INSTRUCTION_DATA_MODE_NONE			QSPI_DATA_NONE
#define FLASH_INSTRUCTION_DATA_MODE_SINGLE			QSPI_DATA_1_LINE
#define FLASH_INSTRUCTION_DATA_MODE_DUAL			QSPI_DATA_2_LINES
#define FLASH_INSTRUCTION_DATA_MODE_QUAD			QSPI_DATA_4_LINES

#define FLASH_INSTRUCTION_ADDRESS_MODE_NONE			QSPI_ADDRESS_NONE
#define FLASH_INSTRUCTION_ADDRESS_MODE_SINGLE		QSPI_ADDRESS_1_LINE
#define FLASH_INSTRUCTION_ADDRESS_MODE_DUAL			QSPI_ADDRESS_2_LINES
#define FLASH_INSTRUCTION_ADDRESS_MODE_QUAD			QSPI_ADDRESS_4_LINES

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

#ifndef DGAS_CONFIG_FLASH_WRITE_REQ_DATA_MAX
#define FLASH_WRITE_REQ_DATA_MAX 64
#else
#define FLASH_WRITE_REQ_DATA_MAX DGAS_CONFIG_FLASH_WRITE_REQ_DATA_MAX
#endif /* DGAS_CONFIG_FLASH_WRITE_REQ_DATA_MAX */

#ifndef DGAS_CONFIG_FLASH_READ_REQ_DATA_MAX
#define FLASH_READ_REQ_DATA_MAX	64
#else
#define FLASH_READ_REQ_DATA_MAX	DGAS_CONFIG_FLASH_READ_REQ_DATA_MAX
#endif /* DGAS_CONFIG_FLASH_READ_REQ_DATA_MAX */

#ifdef FLASH_USE_FREERTOS
#define FLASH_QUEUE_REQ_SIZE	5

#define FLASH_REQ_BUFFER_SIZE	256
#define FLASH_REQ_BUFFER_COUNT	10

#define FLASH_ALLOC_TIMEOUT_10		10
#define FLASH_ALLOC_TIMEOUT_50		50
#define FLASH_ALLOC_TIMEOUT_100		100
#define FLASH_ALLOC_TIMEOUT_200		200
#define FLASH_ALLOC_TIMEOUT_500		500
#define FLASH_ALLOC_TIMEOUT_1000	1000
#define FLASH_ALLOC_TIMEOUT_MAX		portMAX_DELAY

#define DGAS_TASK_FLASH_PRIORITY (tskIDLE_PRIORITY + 1)
#define DGAS_TASK_FLASH_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#endif /* FLASH_USE_FREERTOS */

/**
 * FlashInstruction.
 *
 * Used to send QSPI instructions to flash IC.
 *
 * opCode: Instruction opCode
 * addr: Register address
 * args: Optional arguments for instruction
 * argCount: Number of optional arguments
 * expect: Number of bytes we expect to receive from IC after instruction
 * 		   is issued (if any)
 * dummy: Number of dummy bytes to send at end of instruction
 * */
typedef struct {
	uint8_t opCode;
	uint32_t addr;
	uint8_t* args;
	uint32_t argCount;
	uint32_t expect;
	uint32_t dummy;
}FlashInstruction;

#ifdef FLASH_USE_FREERTOS

typedef uint8_t FlashBuf[FLASH_REQ_BUFFER_SIZE];

/**
 * FlashCMD.
 *
 * Different types of flash commands to be used when making
 * a flash request.
 * */
typedef enum {
	FLASH_CMD_WRITE,
	FLASH_CMD_READ,
	FLASH_CMD_ERASE
}FlashCMD;

/**
 * FlashReq.
 *
 * Flash request struct. Interface used by other tasks to read
 * and write to flash IC.
 *
 * rCmd: Flash command to execute
 * rAddr: Flash address to read/write to (if applicable)
 * rBuf: Buffer containing data to write or to store read data (if applicable)
 * rSize: Number of bytes to read or write (if applicable)
 * */
typedef struct {
	FlashCMD rCmd;
	uint32_t rAddr;
	FlashBuf* rBuf;
	uint32_t rSize;
	TaskHandle_t rCaller;
}FlashReq;
#endif /* FLASH_USE_FREERTOS */

/****************** Function Prototypes ************************/


void flash_init_hardware(void);
DeviceStatus flash_command(QSPI_CommandTypeDef* cmd);
DeviceStatus flash_receive(uint8_t* dest, uint32_t timeout);
DeviceStatus flash_data(uint8_t* data, uint32_t size);
DeviceStatus flash_instruction(FlashInstruction* fin, uint32_t dMode, uint32_t aMode);
DeviceStatus flash_read_info(uint8_t instruction, uint8_t* dest, uint32_t size, uint32_t timeout);
DeviceStatus flash_read_reg(uint8_t regInstr, uint8_t* dest, uint32_t timeout);
DeviceStatus flash_write_reg(uint8_t regInstr, uint8_t* value);
DeviceStatus flash_wait_on_flag(uint8_t regInstr, uint8_t bit, DevFlagOpt set, uint32_t timeout);
DeviceStatus flash_wait_on_busy(void);
DeviceStatus flash_enable_qspi(void);
DeviceStatus flash_enable_memory_mapped(void);
DeviceStatus flash_disable_memory_mapped(void);
DeviceStatus flash_init(void);
DeviceStatus flash_read_mem(uint8_t* dest, uint32_t size, uint32_t addr);
DeviceStatus flash_read_page(uint8_t* dest, uint32_t addr);
DeviceStatus flash_read_sector(uint8_t* dest, uint32_t addr);
DeviceStatus flash_write_mem(uint8_t* data, uint32_t size, uint32_t addr);
DeviceStatus flash_write_page(uint8_t* data, uint32_t addr);
DeviceStatus flash_write_sector(uint8_t* data, uint32_t addr);
DeviceStatus flash_write_enable(void);
DeviceStatus flash_get_device_id(uint8_t* id);
DeviceStatus flash_get_mfr_id(uint8_t* mfr);
DeviceStatus flash_get_jedec_id(uint16_t* jedec);
DeviceStatus flash_chip_erase(void);
DeviceStatus flash_sector_erase(uint32_t sector);
DeviceStatus flash_block_erase_32k(uint32_t block);
DeviceStatus flash_block_erase_64k(uint32_t block);
DeviceStatus flash_self_test(uint32_t pCount);
DeviceStatus flash_self_test_entire(void);

#ifdef FLASH_USE_FREERTOS
FlashBuf* flash_alloc_buffer(uint32_t timeout);
void flash_free_buffer(FlashBuf* ptr);
void task_init_flash(void);
TaskHandle_t task_flash_get_handle(void);
#endif /* FLASH_USE_FREERTOS */

#endif /* DGOS_INCLUDE_FLASH_H_ */
