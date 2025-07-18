/*
 * adc.h
 *
 *  Created on: 18 Jul. 2025
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_DGAS_ADC_H_
#define DGOS_INCLUDE_DGAS_ADC_H_

#include <dgas_types.h>

/******************************** ADC *******************************/
// adc input pin, port and clock enable macro
#define ADC_INPUT_PIN			GPIO_PIN_2
#define ADC_INPUT_PORT			GPIOA
#define __ADC_GPIO_CLK_EN()		__HAL_RCC_GPIOA_CLK_ENABLE()

// ADC instance and clock enable macro
#define ADC_INSTANCE			ADC1
#define __ADC_CLK_EN()			__HAL_RCC_ADC1_CLK_ENABLE()

// clock prescaler
#define ADC_CLK_PRESCALER		ADC_CLOCK_SYNC_PCLK_DIV4

// conversion resolution
#define ADC_RESOLUTION			ADC_RESOLUTION_12B
// actual ADC resolution value
#define ADC_RESOLUTION_VALUE	(1 << 12)

// ADC channel, using PA2 so use ADC_CHANNEL_2
#define ADC_CHANNEL				ADC_CHANNEL_2

// sample time
#define ADC_SAMPLE_TIME			ADC_SAMPLETIME_3CYCLES

/******************************** DMA *******************************/

// Only DMA2 has streams which can access ADC1, check page 230 of reference manual
#define ADC_DMA_INSTANCE			DMA2
#define ADC_DMA_STREAM				DMA2_Stream0
#define ADC_DMA_CHANNEL				DMA_CHANNEL_2
#define __ADC_DMA_CLK_EN()			__HAL_RCC_DMA2_CLK_ENABLE()

/***************************** Hardware ****************************/

// On PCB we have a 1Meg and 282k voltage divider, so when we read 3V3 the actual
// supply voltage is around 15V -> factor is 4.545
#define ADC_VOLTAGE_DIVIDER_FACTOR		4.545
#define ADC_IO_SUPPLY_VOLTAGE			3.3


/***************************** FreeRTOS ****************************/

#define DGAS_TASK_ADC_PRIORITY		(tskIDLE_PRIORITY + 3)
#define DGAS_TASK_ADC_STACK_SIZE	(configMINIMAL_STACK_SIZE * 2)
#define ADC_QUEUE_LENGTH			1

extern QueueHandle_t queueADC;

TaskHandle_t dgas_task_adc_get_handle(void);
void dgas_task_adc_init(void);

#endif /* DGOS_INCLUDE_DGAS_ADC_H_ */
