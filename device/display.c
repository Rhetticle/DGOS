/*
 * display.c
 *
 *  Created on: 7 Jun. 2025
 *      Author: rhett
 */

#include <FreeRTOS.h>
#include <task.h>
#include <display.h>
#include <stm32f7xx.h>
#include <dram.h>
#include <spi.h>

// SPI bus used to send data/commands to display
// Note that the actual pixel data is sent using RGB16 bus
static SPI_HandleTypeDef lcdBus;
// LTDC handle for LTDC peripheral being used with display
static LTDC_HandleTypeDef lcdLTDC;
// Handle for DMA2D peripheral being used with display
static DMA2D_HandleTypeDef lcdDMA;

/**
 * Initialise GPIO pins for SPI use for display
 *
 * Return: None
 * */
static void display_spi_gpio_init(void) {
	GPIO_InitTypeDef init = {0};

	__LCD_SPI_GPIO_CLK_EN();

	init.Alternate = GPIO_AF5_SPI1;
	init.Speed = GPIO_SPEED_FAST;
	init.Mode = GPIO_MODE_AF_PP;
	init.Pull = GPIO_NOPULL;

	init.Pin = LCD_SPI_CLK_PIN;

	HAL_GPIO_Init(LCD_SPI_CLK_PORT, &init);

	init.Pin = LCD_SPI_MOSI_PIN;

	HAL_GPIO_Init(LCD_SPI_MOSI_PORT, &init);

	init.Pin = LCD_CS_PIN;
	init.Mode = MODE_OUTPUT;

	HAL_GPIO_Init(LCD_CS_PORT, &init);

	init.Pin = LCD_NRST_PIN;

	HAL_GPIO_Init(LCD_NRST_PORT, &init);
}

/**
 * Initialise SPI bus for LCD commands
 *
 * Return: None
 * */
static void display_spi_init(void) {
	// enable all clocks required for SPI
	__LCD_SPI_CLK_EN();
	lcdBus.Instance = LCD_SPI_INSTANCE;
	lcdBus.Init.Mode = SPI_MODE_MASTER;
	lcdBus.Init.Direction = SPI_DIRECTION_1LINE;
	lcdBus.Init.DataSize = SPI_DATASIZE_9BIT;
	lcdBus.Init.CLKPolarity = SPI_POLARITY_LOW;
	lcdBus.Init.CLKPhase = SPI_PHASE_1EDGE;
	lcdBus.Init.NSS = SPI_NSS_SOFT;
	lcdBus.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
	lcdBus.Init.FirstBit = SPI_FIRSTBIT_MSB;
	lcdBus.Init.TIMode = SPI_TIMODE_DISABLE;
	lcdBus.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	lcdBus.Init.CRCPolynomial = 7;
	lcdBus.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	lcdBus.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
	HAL_SPI_Init(&lcdBus);
}

/**
 * Initialise GPIO pins for LTDC peripheral
 *
 * Return: None
 * */
static void display_ltdc_gpio_init(void) {
	__LCD_LTDC_GPIO_CLK_EN_ALL();
	// ports A, B, C, D, F and G are being used
	// check header to see which pins share same port

	GPIO_InitTypeDef init = {0};

	// check datasheet to see which AF to use
	init.Alternate = GPIO_AF14_LTDC;
	init.Mode = GPIO_MODE_AF_PP;
	init.Pull = GPIO_NOPULL;
	init.Speed = GPIO_SPEED_HIGH;

	// port A
	init.Pin = LCD_LTDC_PIN_R2 | LCD_LTDC_PIN_R4 |
			   LCD_LTDC_PIN_G2 | LCD_LTDC_PIN_B5 |
			   LCD_LTDC_PIN_VSYNC;

	HAL_GPIO_Init(GPIOA, &init);

	// port B
	init.Pin = LCD_LTDC_PIN_R3 | LCD_LTDC_PIN_R6;
	init.Alternate = GPIO_AF9_LTDC;
	HAL_GPIO_Init(GPIOB, &init);

	init.Pin = LCD_LTDC_PIN_G4 | LCD_LTDC_PIN_G5 |
			   LCD_LTDC_PIN_B6 | LCD_LTDC_PIN_B7;
	init.Alternate = GPIO_AF14_LTDC;

	HAL_GPIO_Init(GPIOB, &init);

	// port C
	init.Pin = LCD_LTDC_PIN_R5 | LCD_LTDC_PIN_G6 |
			   LCD_LTDC_PIN_HSYNC;
	init.Alternate = GPIO_AF14_LTDC;

	HAL_GPIO_Init(GPIOC, &init);

	// port D
	init.Pin = LCD_LTDC_PIN_G7 | LCD_LTDC_PIN_B2;
	init.Alternate = GPIO_AF14_LTDC;

	HAL_GPIO_Init(GPIOD, &init);

	// port F
	init.Pin = LCD_LTDC_PIN_DE;
	init.Alternate = GPIO_AF14_LTDC;

	HAL_GPIO_Init(GPIOF, &init);

	// port G
	init.Pin = LCD_LTDC_PIN_R7 | LCD_LTDC_PIN_G3 |
			   LCD_LTDC_PIN_CLK;
	init.Alternate = GPIO_AF14_LTDC;

	HAL_GPIO_Init(GPIOG, &init);

	init.Pin = LCD_LTDC_PIN_B3 | LCD_LTDC_PIN_B4;
	init.Alternate = GPIO_AF9_LTDC;

	HAL_GPIO_Init(GPIOG, &init);
}

/**
 * Initialise LTDC peripheral for use with display
 *
 * Return: None
 * */
static void display_ltdc_init(void) {
	__LCD_LTDC_CLK_EN();
	// struct to configure LTDC layers (Only one is being used)
	LTDC_LayerCfgTypeDef config = {0};

	lcdLTDC.Instance = LCD_LTDC_INSTANCE;

	lcdLTDC.Init.AccumulatedActiveH = LCD_LTDC_ACTIVE_HEIGHT;
	lcdLTDC.Init.AccumulatedActiveW = LCD_LTDC_ACTIVE_WIDTH;
	lcdLTDC.Init.AccumulatedHBP = LCD_LTDC_ACCUMULATED_HBP;
	lcdLTDC.Init.AccumulatedVBP = LCD_LTDC_ACCUMULATED_VBP;
	// set background colour to black
	lcdLTDC.Init.Backcolor.Blue = 0;
	lcdLTDC.Init.Backcolor.Red = 0;
	lcdLTDC.Init.Backcolor.Green = 0;
	lcdLTDC.Init.Backcolor.Reserved = 0;

	lcdLTDC.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	lcdLTDC.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	lcdLTDC.Init.HorizontalSync = LCD_LTDC_HSYNC_INTERVAL;
	lcdLTDC.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
	lcdLTDC.Init.TotalHeigh = LCD_LTDC_TOTAL_HEIGHT;
	lcdLTDC.Init.TotalWidth = LCD_LTDC_TOTAL_WIDTH;
	lcdLTDC.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	lcdLTDC.Init.VerticalSync = LCD_LTDC_VSYNC_INTERVAL;

	HAL_LTDC_Init(&lcdLTDC);

	config.Alpha = LCD_LTDC_LAYER_ALPHA;
	config.Alpha0 = LCD_LTDC_LAYER_ALPHA_0;
	// set background colour to black
	config.Backcolor.Blue = 0;
	config.Backcolor.Red = 0;
	config.Backcolor.Green = 0;
	config.Backcolor.Reserved = 0;

	config.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	config.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
	config.FBStartAdress = DRAM_START_ADDR;
	config.ImageHeight = LCD_RESOLUTION_Y;
	config.ImageWidth = LCD_RESOLUTION_X;
	config.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
	config.WindowX0 = 0;
	config.WindowX1 = LCD_RESOLUTION_X;
	config.WindowY0 = 0;
	config.WindowY1 = LCD_RESOLUTION_Y;

	HAL_LTDC_ConfigLayer(&lcdLTDC, &config, 0);
}

/**
 * Initialise DMA2D peripheral to be used with flushing
 * frame buffers to DRAM.
 *
 * Return: None
 * */
static void display_dma2d_init(void) {
	__LCD_DMA2D_CLK_EN();

	lcdDMA.Instance = LCD_DMA2D_INSTANCE;
	lcdDMA.Init.Mode = DMA2D_M2M;
	lcdDMA.Init.ColorMode = DMA2D_OUTPUT_RGB565;
	lcdDMA.Init.OutputOffset = 0;
	lcdDMA.LayerCfg[1].InputOffset = 0;
	lcdDMA.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
	lcdDMA.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	lcdDMA.LayerCfg[1].InputAlpha = 0;

	HAL_DMA2D_Init(&lcdDMA);
	HAL_DMA2D_ConfigLayer(&lcdDMA, 1);
}

/**
 * Get handle of LTDC peripheral being used with display
 *
 * Return: LTDC handle of LTDC peripheral being used with display
 * */
LTDC_HandleTypeDef* display_get_ltdc_handle(void) {
	return &lcdLTDC;
}

/**
 * Initialise hardware for display
 *
 * Return: None
 * */
void display_hardware_init(void) {
	display_spi_gpio_init();
	display_spi_init();
	display_ltdc_gpio_init();
	display_ltdc_init();
	display_dma2d_init();
}

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
	uint16_t send = (uint16_t) cmd;
	LCD_CS_LOW();
	vTaskDelay(1);
	taskENTER_CRITICAL();
	HAL_SPI_Transmit(&hspi1, &send, 1, 100);
	taskEXIT_CRITICAL();
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
	uint16_t send = (1 << LCD_DC_BIT_POS) | data;
	LCD_CS_LOW();
	vTaskDelay(1);
	taskENTER_CRITICAL();
	HAL_SPI_Transmit(&hspi1, &send, 1, 100);
	taskEXIT_CRITICAL();
	vTaskDelay(1);
	LCD_CS_HIGH();
}

/**
 * Flush a frame buffer to display. Since using LTDC we simply change the
 * frame buffer address of the peripheral
 *
 * addr: Start address of frame buffer
 *
 * Return: None
 * */
void display_flush_frame_buffer(uint8_t* addr) {
	HAL_LTDC_SetAddress_NoReload(&lcdLTDC, (uint32_t) addr, 0);
	HAL_LTDC_Reload(&lcdLTDC, LTDC_RELOAD_VERTICAL_BLANKING);
}

/**
 * Initialise the LCD display
 *
 * Return: None
 * */
void display_init(void) {
	display_hardware_init();
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

	vTaskDelay(120);

	sendCommand(0x29);

	sendCommand(0x36);
	sendData(0x00);

	sendCommand(0x3A);
	sendData(0x60);//0x60 18bit 0x50 16bit
	sendCommand(0x13);
}


