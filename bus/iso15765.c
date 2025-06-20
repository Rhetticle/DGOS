/*
 * obd_can.c
 *
 *  Created on: Jun 17, 2025
 *      Author: rhett
 */

#include <iso15765.h>
#include <bus.h>
#include <can.h>
#include <stdbool.h>

// stores CAN Bus handle used for OBD CAN messages
static CAN_HandleTypeDef canBus;
// stores current CAN filter configuration
static CAN_FilterTypeDef canFilt;
// stores task handle for this controller
static TaskHandle_t taskHandleOBDCAN;
// flag to indicate if CAN Bus response has been received
static bool canGotMsg;
// in bound and out bound queues for CAN bus
QueueHandle_t queueCANRequest;
QueueHandle_t queueCANResponse;

/**
 * Initialise CAN peripheral for OBD use
 *
 * Return: None
 * */
static void obd_can_init(void) {
	canFilt.FilterActivation = CAN_FILTER_ENABLE;
	canFilt.FilterFIFOAssignment = OBD_CAN_FILTER_FIFO;
	canFilt.FilterMode = CAN_FILTERMODE_IDMASK;

	// set ID low and high to bound of ECU CAN ID
	canFilt.FilterIdHigh = OBD_CAN_ID_RESPONSE_UPPER;
	canFilt.FilterIdLow = OBD_CAN_ID_RESPONSE_LOWER;

	// not using extended ID's so FilterIdLow and FilterIdLow mask don't matter
	canFilt.FilterMaskIdHigh = 0x0000;
	canFilt.FilterMaskIdLow = 0x0000;

	canBus.Instance = OBD_CAN_INSTANCE;
	canBus.Init.Prescaler = 16;
	canBus.Init.Mode = CAN_MODE_NORMAL;
	canBus.Init.SyncJumpWidth = CAN_SJW_1TQ;
	canBus.Init.TimeSeg1 = CAN_BS1_1TQ;
	canBus.Init.TimeSeg2 = CAN_BS2_1TQ;
	canBus.Init.TimeTriggeredMode = DISABLE;
	canBus.Init.AutoBusOff = DISABLE;
	canBus.Init.AutoWakeUp = DISABLE;
	canBus.Init.AutoRetransmission = DISABLE;
	canBus.Init.ReceiveFifoLocked = DISABLE;
	canBus.Init.TransmitFifoPriority = DISABLE;
	// configure ID filter for CAN peripheral
	HAL_CAN_ConfigFilter(&canBus, &canFilt);
	// enable interrupt callback when message is pending for OBD CAN FIFO
	HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
	// initialise CAN peripheral
	HAL_CAN_Init(&canBus);
	HAL_CAN_Start(&canBus);
}

/**
 * Get task handle of OBD CAN controller
 *
 * Return: Task handle of OBD CAN controller
 * */
TaskHandle_t task_obd_can_get_handle(void) {
	return taskHandleOBDCAN;
}

// This function is called by HAL in the HAL_CAN_IRQHandler()
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan) {
	if (hcan->Instance == OBD_CAN_INSTANCE) {
		canGotMsg = true;
	}
}

/**
 * Send OBD data to CAN bus
 *
 * data: Data to send
 * len: Number of bytes to send
 *
 * Returns: Status indicating success or failure
 * */
BusStatus obd_can_send_data(uint8_t* data, uint32_t len) {
	uint32_t ptx;

	if (len > BUS_REQUEST_MAX) {
		return BUS_BUFFER_ERROR;
	}
	CAN_TxHeaderTypeDef txHeader = {0};

	// number of data bytes
	txHeader.DLC = len;
	// use standard 11-bit CAN ID
	txHeader.IDE = CAN_ID_STD;
	// set CAN ID to OBD CAN request ID (0x7DF)
	txHeader.StdId = OBD_CAN_ID_REQUEST;
	// set data type to a data fram
	txHeader.RTR = CAN_RTR_DATA;

	if (HAL_CAN_AddTxMessage(&canBus, &txHeader, data, &ptx) != HAL_OK) {
		return BUS_TX_ERROR;
	}
	return BUS_OK;
}

/**
 * Read data from CAN bus
 *
 * dest: Destination buffer
 *
 * Return: Number of data bytes received
 * */
uint32_t obd_can_get_data(uint8_t* dest) {
	CAN_RxHeaderTypeDef rxHeader = {0};

	if (HAL_CAN_GetRxMessage(&canBus, OBD_CAN_FIFO, &rxHeader, dest) != HAL_OK) {
		return 0;
	}
	// return number of data bytes read
	return rxHeader.DLC;
}

/**
 * Make request and get response on OBD CAN bus
 *
 * req: Request to make
 * resp: Pointer to struct to store response
 *
 * Return: Status indicating success or failure
 * */
BusStatus obd_can_make_request(BusRequest* req, BusResponse* resp) {
	BusStatus status;
	uint32_t timeout = 0;
	// make sure to reset flag from previous use before making request
	canGotMsg = false;

	if ((status = obd_can_send_data(req->data, req->dataLen)) != BUS_OK) {
		return status;
	}
	// request has been made, now wait for response or until timeout
	while(!canGotMsg && (timeout < req->timeout)) {
		timeout++;
		vTaskDelay(1);
	}
	if (timeout == req->timeout) {
		return BUS_RX_ERROR;
	}
	resp->dataLen = obd_can_get_data(resp->data);

	if (resp->dataLen == 0) {
		return BUS_RX_ERROR;
	}
	return BUS_OK;
}

/**
 * Thread function for OBD CAN controller task
 *
 * Return: None
 * */
void task_obd_can(void) {
	BusRequest req = {0};
	BusResponse resp = {0};

	// init CAN peripheral and in bound and out bound queues
	obd_can_init();
	queueCANRequest = xQueueCreate(1, sizeof(BusRequest));
	queueCANResponse = xQueueCreate(1, sizeof(BusResponse));

	for (;;) {
		if (xQueueReceive(queueCANRequest, &req, 10) == pdTRUE) {
			// got request so send over bus and get response
			resp.status = obd_can_make_request(&req, &resp);
			xQueueSend(queueCANResponse, &resp, 10);
		}
	}
}

/**
 * Initialise OBD CAN controller task
 *
 * Return: None
 * */
void task_init_obd_can(void) {
	xTaskCreate((void*) &task_obd_can, "TaskOBDCAN", DGAS_TASK_OBD_CAN_STACK_SIZE,
			NULL, DGAS_TASK_OBD_CAN_PRIORITY, &taskHandleOBDCAN);
}
