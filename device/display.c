/*
 * display.c
 *
 *  Created on: 7 Jun. 2025
 *      Author: rhett
 */

#include <display.h>
#include <spi.h>

// SPI bus used to send data/commands to display
// Note that the actual pixel data is sent using RGB16 bus
extern SPI_HandleTypeDef hspi1;

/**
 * Reset the display
 *
 * Return: None
 * */
void display_reset(void) {
	LCD_NRST_LOW();
	vTaskDelay(10);
	LCD_NRST_HIGH();
	vTaskDelay(10);
}

/**
 * Send command to LCD
 *
 * cmd: Command to send
 *
 * Return: None
 * */
void sendCommand(uint8_t cmd) {
	uint8_t send = cmd;
	LCD_CS_LOW();
	vTaskDelay(1);
	HAL_SPI_Transmit(&hspi1, &send, sizeof(send), 10);
	vTaskDelay(1);
	LCD_CS_HIGH();
}

/**
 * Send data to follow command to LCD
 *
 * data: Data to send
 *
 * Return: None
 * */
void sendData(uint8_t data) {
	uint8_t send = (1 << LCD_DC_BIT_POS) | data;
	LCD_CS_LOW();
	vTaskDelay(1);
	HAL_SPI_Transmit(&hspi1, &send, sizeof(send), 10);
	vTaskDelay(1);
	LCD_CS_HIGH();
}

/**
 * Initialise the LCD display
 *
 * Return: None
 * */
void display_init(void) {
	LCD_NRST_HIGH();
	LCD_CS_HIGH();
	display_reset();

	#if 1
	sendCommand(0xFF);
	sendData(0x77);
	sendData(0x01);
	sendData(0x00);
	sendData(0x00);
	sendData(0x10);

	sendCommand(0xC0);
	sendData(0x3B);
	sendData(0x00);

	sendCommand(0xC1);
	sendData(0x0B); //VBP
	sendData(0x02);

	sendCommand(0xC2);
	sendData(0x00);
	sendData(0x02);

	sendCommand(0xCC);
	sendData(0x10);

	sendCommand(0xCD);
	sendData(0x08);

	sendCommand ( 0xB0); //Positive Voltage Gamma Control
	sendData ( 0x02);
	sendData ( 0x13);
	sendData ( 0x1B);
	sendData ( 0x0D);
	sendData ( 0x10);
	sendData ( 0x05);
	sendData ( 0x08);
	sendData ( 0x07);
	sendData ( 0x07);
	sendData ( 0x24);
	sendData ( 0x04);
	sendData ( 0x11);
	sendData ( 0x0E);
	sendData ( 0x2C);
	sendData ( 0x33);
	sendData ( 0x1D);

	sendCommand ( 0xB1); //Negative Voltage Gamma Control
	sendData ( 0x05);
	sendData ( 0x13);
	sendData ( 0x1B);
	sendData ( 0x0D);
	sendData ( 0x11);
	sendData ( 0x05);
	sendData ( 0x08);
	sendData ( 0x07);
	sendData ( 0x07);
	sendData ( 0x24);
	sendData ( 0x04);
	sendData ( 0x11);
	sendData ( 0x0E);
	sendData ( 0x2C);
	sendData ( 0x33);
	sendData ( 0x1D);

	sendCommand(0xFF);
	sendData(0x77);
	sendData(0x01);
	sendData(0x00);
	sendData(0x00);
	sendData(0x11);

	sendCommand(0xB0);
	sendData(0x5d);//5d

	sendCommand(0xB1); //VCOM amplitude setting
	sendData(0x43); //43

	sendCommand(0xB2); //VGH Voltage setting
	sendData(0x81); //12V

	sendCommand(0xB3);
	sendData(0x80);

	sendCommand(0xB5); //VGL Voltage setting
	sendData(0x43); //-8.3V

	sendCommand(0xB7);
	sendData(0x85);

	sendCommand(0xB8);
	sendData(0x20);

	sendCommand(0xC1);
	sendData(0x78);

	sendCommand(0xC2);
	sendData(0x78);

	sendCommand(0xD0);
	sendData(0x88);

	sendCommand(0xE0);
	sendData(0x00);
	sendData(0x00);
	sendData(0x02);

	sendCommand(0xE1);
	sendData(0x03);
	sendData(0xA0);
	sendData(0x00);
	sendData(0x00);
	sendData(0x04);
	sendData(0xA0);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x20);
	sendData(0x20);

	sendCommand(0xE2);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);

	sendCommand(0xE3);
	sendData(0x00);
	sendData(0x00);
	sendData(0x11);
	sendData(0x00);

	sendCommand(0xE4);
	sendData(0x22);
	sendData(0x00);

	sendCommand(0xE5);
	sendData(0x05);
	sendData(0xEC);
	sendData(0xA0);
	sendData(0xA0);
	sendData(0x07);
	sendData(0xEE);
	sendData(0xA0);
	sendData(0xA0);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);

	sendCommand(0xE6);
	sendData(0x00);
	sendData(0x00);
	sendData(0x11);
	sendData(0x00);

	sendCommand(0xE7);
	sendData(0x22);
	sendData(0x00);

	sendCommand(0xE8);
	sendData(0x06);
	sendData(0xED);
	sendData(0xA0);
	sendData(0xA0);
	sendData(0x08);
	sendData(0xEF);
	sendData(0xA0);
	sendData(0xA0);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);

	sendCommand(0xEB);
	sendData(0x00);
	sendData(0x00);
	sendData(0x40);
	sendData(0x40);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);

	sendCommand(0xED);
	sendData(0xFF);
	sendData(0xFF);
	sendData(0xFF);
	sendData(0xBA);
	sendData(0x0A);
	sendData(0xBF);
	sendData(0x45);
	sendData(0xFF);
	sendData(0xFF);
	sendData(0x54);
	sendData(0xFB);
	sendData(0xA0);
	sendData(0xAB);
	sendData(0xFF);
	sendData(0xFF);
	sendData(0xFF);

	sendCommand(0xEF);
	sendData(0x10);
	sendData(0x0D);
	sendData(0x04);
	sendData(0x08);
	sendData(0x3F);
	sendData(0x1F);

	sendCommand(0xFF);
	sendData(0x77);
	sendData(0x01);
	sendData(0x00);
	sendData(0x00);
	sendData(0x13);

	sendCommand(0xEF);
	sendData(0x08);

	sendCommand(0xFF);
	sendData(0x77);
	sendData(0x01);
	sendData(0x00);
	sendData(0x00);
	sendData(0x00);

	#if 0
	  sendCommand (0xFF);
	WriteData (0x77);
	WriteData (0x01);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x12);
	  sendCommand (0xD1);
	WriteData (0x81);
	WriteData (0x08);
	WriteData (0x03);
	WriteData (0x20);
	WriteData (0x08);
	WriteData (0x01);
	WriteData (0xA0);
	WriteData (0x01);
	WriteData (0xE0);
	WriteData (0xA0);
	WriteData (0x01);
	WriteData (0xE0);
	WriteData (0x03);
	WriteData (0x20);
	  sendCommand (0xD2);
	WriteData (0x08);
	#endif
	/////////////////Bring up the internal test picture///////////////////////////////////
	#endif
	sendCommand(0x11);

	HAL_Delay(120);

	sendCommand(0x29);

	sendCommand(0x36);
	sendData(0x00);

	sendCommand(0x3A);
	sendData(0x60);//0x60 18bit 0x50 16bit
	sendCommand(0x13);
}


