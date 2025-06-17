/*
 * kwp.h
 *
 *  Created on: Jun 1, 2025
 *      Author: rhett
 */

#ifndef INC_KWP_H_
#define INC_KWP_H_

#include <dgas_types.h>

#define K_LINE_PORT GPIOC
#define K_LINE_PIN GPIO_PIN_10

#define L_LINE_PORT GPIOA
#define L_LINE_PIN GPIO_PIN_10

// timeout used for responses during initilisation of the bus
// These responses can take longer
#define KWP_BUS_INIT_TIMEOUT 2000

#define KWP_BUS_BAUD_RATE 10400

#define KWP_BUS_ADDRESS 0x33

#define KWP_INTERBYTE_DELAY 5

#define KWP_HEADER_SIZE 3 // 3 header bytes
#define KWP_HEADER_ONE 0xC2
#define KWP_HEADER_TWO 0x33
#define KWP_HEADER_THREE 0xF1
#define KWP_DATA_SIZE_MASK 0b111111 // mask to apply to format byte to know how many bytes are to follow
#define KWP_OFFSET_DATA_START 3
#define KWP_GET_MSG_SIZE_FROM_FBYTE(fByte) ((fByte & KWP_DATA_SIZE_MASK) + KWP_HEADER_SIZE + 1)
#define KWP_GET_CHECKSUM_FROM_MSG(msg, msgSize) (msg[msgSize - 1]) // checksum is last byte of message

// this task performs time critical functionality so give it very high priority
#define TASK_KWP_PRIORITY (tskIDLE_PRIORITY + 5)
#define TASK_KWP_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define QUEUE_KWP_LENGTH 5

extern QueueHandle_t queueKwpRequest;
extern QueueHandle_t queueKwpResponse;

typedef struct {
	uint8_t sync;
	uint8_t kwOne;
	uint8_t kwTwo;
} KWPInit;

void task_init_kwp_bus(void);

#endif /* INC_KWP_H_ */
