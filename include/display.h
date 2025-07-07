/*
 * display.h
 *
 *  Created on: 7 Jun. 2025
 *      Author: rhett
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

#include <stm32f7xx.h>
#include <dram.h>

/**************************** LCD SPI Bus ***************************/

#define LCD_SPI_INSTANCE SPI1
// LCD SPI pin definitions
#define LCD_SPI_CLK_PORT GPIOB
#define __LCD_SPI_CLK_PORT_CLK_EN() __HAL_RCC_GPIOB_CLK_ENABLE()
#define LCD_SPI_CLK_PIN GPIO_PIN_3

#define LCD_SPI_MOSI_PORT GPIOB
#define __LCD_SPI_CLK_PORT_CLK_EN() __HAL_RCC_GPIOB_CLK_ENABLE()
#define LCD_SPI_MOSI_PIN GPIO_PIN_5

#ifndef LCD_CS_PIN
#define LCD_CS_PIN GPIO_PIN_4
#define LCD_CS_PORT GPIOB
#define __LCD_CS_PORT_CLK_EN() __HAL_RCC_GPIOB_CLK_ENABLE()
#endif

#ifndef LCD_NRST_PIN
#define LCD_NRST_PIN GPIO_PIN_7
#define LCD_NRST_PORT GPIOB
#define __LCD_NRST_PORT_CLK_EN() __HAL_RCC_GPIOB_CLK_ENABLE()
#endif

#define __LCD_SPI_CLK_EN() __HAL_RCC_SPI1_CLK_ENABLE();

#define __LCD_SPI_GPIO_CLK_EN() {__LCD_SPI_CLK_PORT_CLK_EN();\
								 __LCD_SPI_CLK_PORT_CLK_EN();\
								 __LCD_CS_PORT_CLK_EN();\
								 __LCD_NRST_PORT_CLK_EN();}

#define LCD_CS_LOW() HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 0)
#define LCD_CS_HIGH() HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 1)

#define LCD_NRST_LOW() HAL_GPIO_WritePin(LCD_NRST_PORT, LCD_NRST_PIN, 0)
#define LCD_NRST_HIGH() HAL_GPIO_WritePin(LCD_NRST_PORT, LCD_NRST_PIN, 1)

/******************************* LCD LTDC ********************************/

#define LCD_LTDC_INSTANCE 		LTDC

// RGB666 being used so use pins x2 -> x7
// Pins

// Red
#define LCD_LTDC_PIN_R2			GPIO_PIN_1 		// port A
#define LCD_LTDC_PIN_R3			GPIO_PIN_0		// port B
#define LCD_LTDC_PIN_R4			GPIO_PIN_5		// port A
#define LCD_LTDC_PIN_R5			GPIO_PIN_0		// port C
#define LCD_LTDC_PIN_R6			GPIO_PIN_1		// port B
#define LCD_LTDC_PIN_R7			GPIO_PIN_6		// port G

// Green
#define LCD_LTDC_PIN_G2			GPIO_PIN_6 		// port A
#define LCD_LTDC_PIN_G3			GPIO_PIN_10		// port G
#define LCD_LTDC_PIN_G4			GPIO_PIN_10		// port B
#define LCD_LTDC_PIN_G5			GPIO_PIN_11		// port B
#define LCD_LTDC_PIN_G6			GPIO_PIN_7		// port C
#define LCD_LTDC_PIN_G7			GPIO_PIN_3		// port D


// Blue
#define LCD_LTDC_PIN_B2			GPIO_PIN_6 		// port D
#define LCD_LTDC_PIN_B3			GPIO_PIN_11		// port G
#define LCD_LTDC_PIN_B4			GPIO_PIN_12		// port G
#define LCD_LTDC_PIN_B5			GPIO_PIN_3		// port A
#define LCD_LTDC_PIN_B6			GPIO_PIN_8		// port B
#define LCD_LTDC_PIN_B7			GPIO_PIN_9		// port B

// Control
#define LCD_LTDC_PIN_HSYNC		GPIO_PIN_6		// port C
#define LCD_LTDC_PIN_VSYNC		GPIO_PIN_4		// port A
#define LCD_LTDC_PIN_CLK		GPIO_PIN_7		// port G
#define LCD_LTDC_PIN_DE			GPIO_PIN_10		// port F

// clock enable macros
#define __LCD_LTDC_CLK_EN() __HAL_RCC_LTDC_CLK_ENABLE();

#define __LCD_LTDC_GPIO_CLK_EN_ALL() {__HAL_RCC_GPIOA_CLK_ENABLE();\
									  __HAL_RCC_GPIOB_CLK_ENABLE();\
									  __HAL_RCC_GPIOC_CLK_ENABLE();\
									  __HAL_RCC_GPIOD_CLK_ENABLE();\
									  __HAL_RCC_GPIOF_CLK_ENABLE();\
									  __HAL_RCC_GPIOG_CLK_ENABLE();}

/******************************* LCD DMA2D ********************************/

#define LCD_DMA2D_INSTANCE		DMA2D
#define __LCD_DMA2D_CLK_EN()	__HAL_RCC_DMA2D_CLK_ENABLE()

/**************************** Display Constants **************************/

#define LCD_RESOLUTION_X 480
#define LCD_RESOLUTION_Y 480

#define LCD_DC_BIT_POS 8

/************************** LTDC Constants *******************************/

#define LCD_LTDC_HSYNC_INTERVAL		3
#define LCD_LTDC_VSYNC_INTERVAL		3
#define LCD_LTDC_ACCUMULATED_HBP	11
#define LCD_LTDC_ACCUMULATED_VBP	11
#define LCD_LTDC_ACTIVE_WIDTH		(LCD_RESOLUTION_X + 11)
#define LCD_LTDC_ACTIVE_HEIGHT		(LCD_RESOLUTION_Y + 11)
#define LCD_LTDC_TOTAL_WIDTH		(LCD_RESOLUTION_X + 19)
#define LCD_LTDC_TOTAL_HEIGHT		(LCD_RESOLUTION_Y + 19)

/*********************** LTDC Layer Constants ****************************/

#define LCD_LTDC_LAYER_ALPHA		255
#define LCD_LTDC_LAYER_ALPHA_0		0

/********************** Frame Buffer Constants **************************/

// We will space the two frame buffers as far from each other as possible
// as visual glitches can occur if they're too close.
#define LCD_FRAME_BUFF_SIZE			(LCD_RESOLUTION_X * LCD_RESOLUTION_Y * 2)
#define LCD_FRAME_BUFF_ONE_ADDR		DRAM_START_ADDR
#define LCD_FRAME_BUFF_TWO_ADDR		(DRAM_START_ADDR + DRAM_SIZE - LCD_FRAME_BUFF_SIZE)


// Function prototypes
void display_reset(void);
LTDC_HandleTypeDef* display_get_ltdc_handle(void);
void display_init(void);
void sendCommand(uint8_t cmd);
void sendData(uint8_t data);

#endif /* INC_DISPLAY_H_ */
