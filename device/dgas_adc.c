/*
 * adc.c
 *
 *  Created on: 18 Jul. 2025
 *      Author: Rhett Humphreys
 *
 *  Driver for DGAS ADC functionality. On-chip ADC of STM32 is used
 *  to measure DGAS supply voltage to be displayed on gauge. Using
 *  DMA to automatically move conversions into RAM
 */

#include <dgas_adc.h>

// ADC Handle
static ADC_HandleTypeDef adcHandle;
// Task handle for ADC controller task
static TaskHandle_t taskHandleADC;
// 16-bit uint to store most recent conversion from ADC
static uint32_t lastConv;
// Queue for sending conversions
QueueHandle_t queueADC;

/**
 * Initialise GPIO pins for ADC use
 *
 * Return: None
 * */
static void adc_gpio_init(void) {
	GPIO_InitTypeDef init = {0};
	__ADC_GPIO_CLK_EN();

	init.Mode = GPIO_MODE_ANALOG;
	init.Pin = ADC_INPUT_PIN;
	init.Speed = GPIO_SPEED_HIGH;
	init.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(ADC_INPUT_PORT, &init);
}

/**
 * Initialise ADC peripheral for use
 *
 * Return: None
 * */
static void adc_periph_init(void) {
	ADC_ChannelConfTypeDef conf = {0};
	__ADC_CLK_EN();
	adcHandle.Instance = ADC_INSTANCE;

	adcHandle.Init.ClockPrescaler = ADC_CLK_PRESCALER;
	adcHandle.Init.ContinuousConvMode = ENABLE;
	adcHandle.Init.DMAContinuousRequests = ENABLE;
	adcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	adcHandle.Init.DiscontinuousConvMode = DISABLE;
	adcHandle.Init.EOCSelection = DISABLE;
	adcHandle.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
	adcHandle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIG_EDGE_NONE;
	adcHandle.Init.NbrOfConversion = 1;
	adcHandle.Init.NbrOfDiscConversion = 0;
	adcHandle.Init.Resolution = ADC_RESOLUTION;
	adcHandle.Init.ScanConvMode = DISABLE;

	HAL_ADC_Init(&adcHandle);

	conf.Channel = ADC_CHANNEL;
	conf.Offset = 0;
	conf.Rank = ADC_REGULAR_RANK_1;
	conf.SamplingTime = ADC_SAMPLE_TIME;

	HAL_ADC_ConfigChannel(&adcHandle, &conf);
}

/**
 * Initialise DMA for use with ADC
 *
 * Return: None
 * */
static void adc_dma_init(void) {
	__ADC_DMA_CLK_EN();

	// setup stream for ADC_DMA_CHANNEL with circular mode
	ADC_DMA_STREAM->CR |= (ADC_DMA_CHANNEL) | (DMA_MBURST_SINGLE) |
						  (DMA_PBURST_SINGLE) | (DMA_PRIORITY_HIGH) |
						  (DMA_SxCR_MSIZE_1) | (DMA_SxCR_PSIZE_1) |
						  (DMA_CIRCULAR);
	// only one transfer
	ADC_DMA_STREAM->NDTR = 1;
	// setup peripheral and memory addresses
	ADC_DMA_STREAM->PAR = (uint32_t) &(ADC_INSTANCE->DR);
	ADC_DMA_STREAM->M0AR = (uint32_t) &(lastConv);

	// finally must modify CR2 of ADC to allow DMA transfers
	ADC_INSTANCE->CR2 |= ADC_CR2_DMA;
	// start DMA transfers
	ADC_DMA_STREAM->CR |= DMA_SxCR_EN;
}

/**
 * Initialise relevant hardware for ADC
 *
 * Return: None
 * */
static void adc_hardware_init(void) {
	adc_gpio_init();
	adc_periph_init();
	adc_dma_init();
}

/**
 * Get task handle of ADC controller task
 *
 * Return: Task handle of ADC controller task
 * */
TaskHandle_t dgas_task_adc_get_handle(void) {
	return taskHandleADC;
}

/**
 * Convert a raw ADC reading to a voltage
 *
 * raw: Raw 12-bit conversion
 *
 * Return: Voltage as a float
 * */
float adc_conv_raw_to_voltage(uint16_t raw) {
	float tmp = ((float) raw) / ADC_RESOLUTION_VALUE;
	tmp = tmp * ADC_IO_SUPPLY_VOLTAGE;

	return (tmp * ADC_VOLTAGE_DIVIDER_FACTOR);
}

/**
 * Thread function for DGAS ADC task
 *
 * Return: None
 * */
void task_adc(void) {
	float voltage;
	// init hardware and start conversions
	adc_hardware_init();
	HAL_ADC_Start(&adcHandle);

	queueADC = xQueueCreate(ADC_QUEUE_LENGTH, sizeof(float));
	for (;;) {
		voltage = adc_conv_raw_to_voltage(lastConv);
		xQueueSend(queueADC, &voltage, 10);
		vTaskDelay(100);
	}
}

/**
 * Initialise ADC controller task
 *
 * Return: None
 * */
void dgas_task_adc_init(void) {
	xTaskCreate((void*) &task_adc, "ADC", DGAS_TASK_ADC_STACK_SIZE, NULL,
			DGAS_TASK_ADC_PRIORITY, &taskHandleADC);
}
