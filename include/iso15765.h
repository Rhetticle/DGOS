/*
 * obd_can.h
 *
 *  Created on: Jun 17, 2025
 *      Author: rhett
 */

#ifndef INC_OBD_CAN_H_
#define INC_OBD_CAN_H_

#include <dgas_types.h>
#include <bus.h>

extern QueueHandle_t queueCANRequest;
extern QueueHandle_t queueCANResponse;

#ifndef DGAS_CONFIG_OBD_CAN_INSTANCE
#define OBD_CAN_INSTANCE CAN1
#define OBD_CAN_RX_PORT GPIOA
#define OBD_CAN_RX_PIN GPIO_PIN_11
#define OBD_CAN_TX_PORT GPIOA
#define OBD_CAN_TX_PIN GPIO_PIN_12
#define __OBD_CAN_TX_CLK_EN() __HAL_RCC_GPIOA_CLK_ENABLE()
#define __OBD_CAN_RX_CLK_EN() __HAL_RCC_GPIOA_CLK_ENABLE()
#define __OBD_CAN_GPIO_CLK_EN() {__OBD_CAN_TX_CLK_EN();\
								 __OBD_CAN_RX_CLK_EN();}
#define __OBD_CAN_CLK_EN() 	  __HAL_RCC_CAN1_CLK_ENABLE()
#endif

#ifndef DGAS_CONFIG_OBD_CAN_FIFO
#define OBD_CAN_FIFO CAN_RX_FIFO0
#define OBD_CAN_FILTER_FIFO CAN_FILTER_FIFO0
#define OBD_CAN_NOTIFICATION CAN_IT_RX_FIFO0_MSG_PENDING
#endif

// CAN ID to use when requesting data
#define OBD_CAN_ID_REQUEST 0x7DF

// CAN ID that ECU will respond with. Will be in range of
// 0x7E8 to 0x7EF and most commonly is 0x7E8
#define OBD_CAN_ID_RESPONSE_LOWER 0x7E8
#define OBD_CAN_ID_RESPONSE_UPPER 0x7EF

#define DGAS_TASK_OBD_CAN_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define DGAS_TASK_OBD_CAN_PRIORITY   (tskIDLE_PRIORITY + 5)

// Function prototypes
TaskHandle_t task_obd_can_get_handle(void);
BusStatus obd_can_send_data(uint8_t* data, uint32_t len);
uint32_t obd_can_get_data(uint8_t* dest);
BusStatus obd_can_make_request(BusRequest* req, BusResponse* resp);
void task_init_obd_can(void);

#endif /* INC_OBD_CAN_H_ */
