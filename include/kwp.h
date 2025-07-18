/*
 * kwp.h
 *
 *  Created on: Jun 1, 2025
 *      Author: rhett
 */

#ifndef INC_KWP_H_
#define INC_KWP_H_

#include <dgas_types.h>

// in bound and out bound queues for KWP bus
extern QueueHandle_t queueKwpRequest;
extern QueueHandle_t queueKwpResponse;

#ifdef DGAS_CONFIG_BUS_KWP_UART_INSTANCE
#define KWP_UART_INSTANCE DGAS_CONFIG_BUS_KWP_UART_INSTANCE
#else
#define KWP_UART_INSTANCE 			UART4
#define KWP_UART_INSTANCE_IRQN		UART4_IRQn
#define KWP_UART_TX_PORT 			GPIOC
#define KWP_UART_TX_PIN 			GPIO_PIN_10
#define KWP_UART_RX_PORT 			GPIOC
#define KWP_UART_RX_PIN 			GPIO_PIN_11
#define __KWP_UART_TX_PORT_CLK_EN() __HAL_RCC_GPIOC_CLK_ENABLE()
#define __KWP_UART_RX_PORT_CLK_EN() __HAL_RCC_GPIOC_CLK_ENABLE()
#define __KWP_UART_GPIO_CLK_EN()   {__KWP_UART_TX_PORT_CLK_EN();\
								    __KWP_UART_RX_PORT_CLK_EN();}
#define __KWP_UART_CLK_EN() 		__HAL_RCC_UART4_CLK_ENABLE()
#endif /* DGAS_CONFIG_BUS_KWP_UART_INSTANCE */

#ifdef DGAS_CONFIG_BUS_KWP_K_PORT
#define K_LINE_PORT DGAS_CONFIG_BUS_K_PORT
#define K_LINE_PIN DGAS_CONFIG_BUS_K_PIN
#else
#define K_LINE_PORT GPIOC
#define K_LINE_PIN GPIO_PIN_10
#endif /* DGAS_CONFIG_BUS_KWP_K_PORT */

#ifdef DGAS_CONFIG_BUS_KWP_L_PORT
#define L_LINE_PORT DGAS_CONFIG_BUS_KWP_L_PORT
#define L_LINE_PIN DGAS_CONFIG_BUS_KWP_L_PIN
#else
#define L_LINE_PORT GPIOA
#define L_LINE_PIN GPIO_PIN_10
#endif /* DGAS_CONFIG_BUS_KWP_L_PORT */

#ifdef DGAS_CONFIG_BUS_KWP_RX_BUFF_SIZE
#define KWP_UART_RX_BUFF_SIZE		DGAS_CONFIG_BUS_KWP_RX_BUFF_SIZE
#else
#define KWP_UART_RX_BUFF_SIZE		64
#endif

// K and L line high and low macros. Note L-Line control circuitry is active low
#define L_LINE_LOW()		HAL_GPIO_WritePin(L_LINE_PORT, L_LINE_PIN, 1)
#define L_LINE_HIGH()		HAL_GPIO_WritePin(L_LINE_PORT, L_LINE_PIN, 0)
#define K_LINE_LOW()		HAL_GPIO_WritePin(K_LINE_PORT, K_LINE_PIN, 0)
#define K_LINE_HIGH()		HAL_GPIO_WritePin(K_LINE_PORT, K_LINE_PIN, 1)

// timeout used for responses during initilisation of the bus
// These responses can take longer
#define KWP_BUS_INIT_TIMEOUT 100

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

#define TASK_KWP_PRIORITY (tskIDLE_PRIORITY + 5)
#define TASK_KWP_STACK_SIZE (configMINIMAL_STACK_SIZE * 6)
#define QUEUE_KWP_LENGTH 5

typedef struct {
	uint8_t sync;
	uint8_t kwOne;
	uint8_t kwTwo;
} KWPInit;

// Function prototypes
TaskHandle_t task_kwp_get_handle(void);
void kwp_bus_five_baud_init(void);
BusStatus kwp_bus_write_byte(uint8_t byte);
BusStatus kwp_bus_read_byte(uint8_t* dest, uint32_t timeout);
BusStatus kwp_bus_write(uint8_t* data, uint32_t len);
BusStatus kwp_bus_read(uint8_t* dest, uint32_t len, uint32_t timeout);
uint8_t kwp_bus_calc_checksum(uint8_t* data, uint32_t size);
BusStatus kwp_bus_get_init_response(KWPInit* init);
BusStatus kwp_bus_init(void);
BusStatus kwp_bus_make_request(BusRequest* req);
void kwp_bus_extract_data(uint8_t* response, uint32_t dataSize, uint8_t* dest);
BusStatus kwp_bus_get_response(BusResponse* resp, uint32_t timeout);
BusStatus kwp_bus_handle_request(BusRequest* busReq, BusResponse* busResp);
void task_init_kwp_bus(void);

#endif /* INC_KWP_H_ */
