/*
 * display.h
 *
 *  Created on: 7 Jun. 2025
 *      Author: rhett
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

#include <stm32f7xx.h>

#define LCD_SPI_INSTANCE SPI1

#ifndef LCD_CS_PIN
#define LCD_CS_PIN GPIO_PIN_4
#define LCD_CS_PORT GPIOB
#endif

#ifndef LCD_NRST_PIN
#define LCD_NRST_PIN GPIO_PIN_7
#define LCD_NRST_PORT GPIOB
#endif

#define LCD_RESOLUTION_X 480
#define LCD_RESOLUTION_Y 480

#define LCD_DC_BIT_POS 8

#define LCD_CS_LOW() (HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 0))
#define LCD_CS_HIGH() (HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, 1))

#define LCD_NRST_LOW() (HAL_GPIO_WritePin(LCD_NRST_PORT, LCD_NRST_PIN, 0))
#define LCD_NRST_HIGH() (HAL_GPIO_WritePin(LCD_NRST_PORT, LCD_NRST_PIN, 1))

// Function prototypes
void display_reset(void);
void display_init(void);
void sendCommand(uint8_t cmd);
void sendData(uint8_t data);

#endif /* INC_DISPLAY_H_ */
