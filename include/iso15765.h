/*
 * obd_can.h
 *
 *  Created on: Jun 17, 2025
 *      Author: rhett
 */

#ifndef INC_OBD_CAN_H_
#define INC_OBD_CAN_H_

#include <dgas_types.h>

#ifndef DGAS_CONFIG_OBD_CAN_INSTANCE
#define OBD_CAN_INSTANCE CAN1
#else
#define OBD_CAN_INSTANCE DGAS_CONFIG_OBD_CAN_INSTANCE
#endif

#ifndef DGAS_CONFIG_OBD_CAN_FIFO
#define OBD_CAN_FIFO CAN_RX_FIFO0
#define OBD_CAN_FILTER_FIFO CAN_FILTER_FIFO0
#define OBD_CAN_NOTIFICATION CAN_IT_RX_FIFO0_MSG_PENDING
#else
#define OBD_CAN_FIFO DGAS_CONFIG_OBD_CAN_FIFO
#define OBD_CAN_FILTER_FIFO DGAS_CONFIG_CAN_FILTER_FIFO
#define OBD_CAN_NOTIFICATION DGAS_CONFIG_CAN_IT_NOTIFICATION
#endif

// CAN ID to use when requesting data
#define OBD_CAN_ID_REQUEST 0x7DF

// CAN ID that ECU will respond with. Will be in range of
// 0x7E8 to 0x7EF and most commonly is 0x7E8
#define OBD_CAN_ID_RESPONSE_LOWER 0x7E8
#define OBD_CAN_ID_RESPONSE_UPPER 0x7EF

#define DGAS_TASK_OBD_CAN_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define DGAS_TASK_OBD_CAN_PRIORITY   (tskIDLE_PRIORITY + 5)

extern QueueHandle_t queueCANRequest;
extern QueueHandle_t queueCANResponse;


#endif /* INC_OBD_CAN_H_ */
