/*
 * dram.h
 *
 *  Created on: Jun 27, 2025
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_DRAM_H_
#define DGOS_INCLUDE_DRAM_H_

#include <device.h>
#include <stm32f7xx.h>

#define DRAM_FMC_INSTANCE FMC_SDRAM_DEVICE

/**************************** PIN DEFINITIONS *****************************/

// address pins
#define DRAM_PIN_A0 		GPIO_PIN_0 		// port F
#define DRAM_PIN_A1			GPIO_PIN_1 		// port F
#define DRAM_PIN_A2			GPIO_PIN_2 		// port F
#define DRAM_PIN_A3			GPIO_PIN_3 		// port F
#define DRAM_PIN_A4 		GPIO_PIN_4 		// port F
#define DRAM_PIN_A5			GPIO_PIN_5 		// port F
#define DRAM_PIN_A6			GPIO_PIN_12 	// port F
#define DRAM_PIN_A7			GPIO_PIN_13 	// port F
#define DRAM_PIN_A8			GPIO_PIN_14 	// port F
#define DRAM_PIN_A9			GPIO_PIN_15 	// port F
#define DRAM_PIN_A10		GPIO_PIN_0  	// port G
#define DRAM_PIN_A11		GPIO_PIN_1  	// port G

// data pins
#define DRAM_PIN_D0			GPIO_PIN_14		// port D
#define DRAM_PIN_D1			GPIO_PIN_15 	// port D
#define DRAM_PIN_D2			GPIO_PIN_0 		// port D
#define DRAM_PIN_D3			GPIO_PIN_1 		// port D
#define DRAM_PIN_D4			GPIO_PIN_7		// port E
#define DRAM_PIN_D5			GPIO_PIN_8		// port E
#define DRAM_PIN_D6			GPIO_PIN_9		// port E
#define DRAM_PIN_D7			GPIO_PIN_10		// port E
#define DRAM_PIN_D8			GPIO_PIN_11		// port E
#define DRAM_PIN_D9			GPIO_PIN_12		// port E
#define DRAM_PIN_D10		GPIO_PIN_13		// port E
#define DRAM_PIN_D11		GPIO_PIN_14		// port E
#define DRAM_PIN_D12		GPIO_PIN_15		// port E
#define DRAM_PIN_D13		GPIO_PIN_8		// port D
#define DRAM_PIN_D14		GPIO_PIN_9		// port D
#define DRAM_PIN_D15		GPIO_PIN_10		// port D

// address command/control pins
#define DRAM_PIN_SDNE0		GPIO_PIN_2		// port C
#define DRAM_PIN_SDCKE0		GPIO_PIN_3		// port C
#define DRAM_PIN_SDNWE		GPIO_PIN_7		// port A
#define DRAM_PIN_SDNRAS		GPIO_PIN_11		// port F
#define DRAM_PIN_BA0		GPIO_PIN_4		// port G
#define DRAM_PIN_SDCLK		GPIO_PIN_8		// port G
#define DRAM_PIN_SDNCAS		GPIO_PIN_15		// port G


// clock enable macros
#define __DRAM_GPIO_CLK_EN_ALL() {__HAL_RCC_GPIOA_CLK_ENABLE();\
								  __HAL_RCC_GPIOC_CLK_ENABLE();\
								  __HAL_RCC_GPIOD_CLK_ENABLE();\
								  __HAL_RCC_GPIOE_CLK_ENABLE();\
								  __HAL_RCC_GPIOF_CLK_ENABLE();\
								  __HAL_RCC_GPIOG_CLK_ENABLE();}

#define __DRAM_FMC_CLK_EN() __HAL_RCC_FMC_CLK_ENABLE()

// Delay and timing values (check reference manual)
#define DRAM_FMC_LOAD_TO_ACTIVE_DELAY 	2
#define DRAM_FMC_SELF_REFRESH_DELAY   	6
#define DRAM_FMC_SELF_REFRESH_TIME		6
#define DRAM_FMC_ROW_CYCLE_DELAY		9
#define DRAM_FMC_WRITE_RECOVERY_TIME	5
#define DRAM_FMC_RP_DELAY				2
#define DRAM_FMC_RCD_DELAY				2

/* COUNT = [(SDRAM self refresh time / number of row) x  SDRAM CLK] – 20
	     = [(64ms/4096) * 100MHz] - 20 = 1562.5 - 20 ~ 1542 */
#define DRAM_CELL_REFRESH_RATE 			1105

// DRAM size
#define DRAM_SIZE 0x200000
// DRAM start address relative to STM32 address space
#define DRAM_START_ADDR 0xC0000000

/************************ FUNCTION PROTOTYPES ***********************/
void dram_init_hardware(void);
void dram_init(void);
void dram_fill_section(uint32_t startAddr, uint32_t endAddr, uint16_t value);
void dram_clear_section(uint32_t startAddr, uint32_t endAddr);
void dram_clear(void);
void dram_read_section(uint32_t startAddr, uint8_t* dest, uint32_t size);

#endif /* DGOS_INCLUDE_DRAM_H_ */
