/*
 * kwp.c
 *
 *  Created on: Jun 1, 2025
 *      Author: rhett
 */

#include <dgas_types.h>
#include <dgas_obd.h>
#include <kwp.h>
#include <bus.h>
#include <string.h>
#include "usart.h"

// queue for bus requests
QueueHandle_t queueKwpRequest;
// queue for bus responses
QueueHandle_t queueKwpResponse;

// stores task handle
static TaskHandle_t handleKwp;
// kwp UART bus
static UART_HandleTypeDef kwpBus;

/**
 * Get the task handle of the KWP bus task
 * */
TaskHandle_t task_kwp_get_handle(void) {
	return handleKwp;
}

/**
 * Change K line pin to a regular GPIO (needed for slow five baud init)
 * */
void kwp_bus_k_gpio(void) {
	GPIO_InitTypeDef gpioInit = {0};
	HAL_UART_DeInit(&huart4);

	gpioInit.Pin = K_LINE_PIN;
	gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
	gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
	gpioInit.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(K_LINE_PORT, &gpioInit);
}

void kwp_bus_init_uart(void) {
	kwpBus.Instance = KWP_UART_INSTANCE;
	kwpBus.Init.BaudRate = KWP_BUS_BAUD_RATE;
	kwpBus.Init.WordLength = UART_WORDLENGTH_8B;
	kwpBus.Init.StopBits = UART_STOPBITS_1;
	kwpBus.Init.Parity = UART_PARITY_NONE;
	kwpBus.Init.Mode = UART_MODE_TX_RX;
	kwpBus.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	kwpBus.Init.OverSampling = UART_OVERSAMPLING_16;
	kwpBus.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	kwpBus.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	HAL_UART_Init(&kwpBus);
}


/**
 * Perform 5-baud init of KWP bus
 * */
void kwp_bus_five_baud_init(void) {
	// ensure K pin is setup as GPIO pin
	kwp_bus_k_gpio();
	HAL_GPIO_WritePin(K_LINE_PORT, K_LINE_PIN, 1);
	HAL_GPIO_WritePin(L_LINE_PORT, L_LINE_PIN, 0);
	HAL_GPIO_WritePin(K_LINE_PORT, K_LINE_PIN, 0);
	HAL_GPIO_WritePin(L_LINE_PORT, L_LINE_PIN, 1);
	vTaskDelay(205);
	HAL_GPIO_WritePin(K_LINE_PORT, K_LINE_PIN, 1);
	HAL_GPIO_WritePin(L_LINE_PORT, L_LINE_PIN, 0);
	vTaskDelay(405);
	HAL_GPIO_WritePin(K_LINE_PORT, K_LINE_PIN, 0);
	HAL_GPIO_WritePin(L_LINE_PORT, L_LINE_PIN, 1);
	vTaskDelay(405);
	HAL_GPIO_WritePin(K_LINE_PORT, K_LINE_PIN, 1);
	HAL_GPIO_WritePin(L_LINE_PORT, L_LINE_PIN, 0);
	vTaskDelay(405);
	HAL_GPIO_WritePin(K_LINE_PORT, K_LINE_PIN, 0);
	HAL_GPIO_WritePin(L_LINE_PORT, L_LINE_PIN, 1);
	vTaskDelay(405);
	HAL_GPIO_WritePin(K_LINE_PORT, K_LINE_PIN, 1);
	HAL_GPIO_WritePin(L_LINE_PORT, L_LINE_PIN, 0);
	vTaskDelay(205);
	// change K pin back to TX ready for UART communication
	HAL_GPIO_DeInit(K_LINE_PORT, K_LINE_PIN);
	kwp_bus_init_uart();
}

/**
 * Write a single byte to KWP bus
 *
 * byte: Byte to write
 *
 * Return: status indicating success or failure
 * */
BusStatus kwp_bus_write_byte(uint8_t byte) {
	uint8_t tmp = byte;
	uint8_t echo;

	if (HAL_UART_Transmit(&kwpBus, &tmp, sizeof(uint8_t), 100) != HAL_OK) {
		return BUS_TX_ERROR;
	}
	// the byte we just sent will be echoed back since it's all on the K-Line so we will read it back
	if (HAL_UART_Receive(&kwpBus, &echo, sizeof(uint8_t), 100) != HAL_OK) {
		return BUS_ECHO_ERROR;
	}
	if (echo != tmp) {
		return BUS_ECHO_ERROR;
	}
	return BUS_OK;
}

/**
 * Read a single byte from KWP bus
 *
 * dest: Pointer to store data to
 *
 * Return: Status indicating success or failure
 * */
BusStatus kwp_bus_read_byte(uint8_t* dest, uint32_t timeout) {
	if (HAL_UART_Receive(&kwpBus, dest, sizeof(uint8_t), timeout) != HAL_OK) {
		return BUS_RX_ERROR;
	}
	return BUS_OK;
}

/**
 * Write a stream of data to KWP bus
 *
 * data: Data to write
 * len: Length of data to write
 *
 * Return: Status indicating success or failure
 * */
BusStatus kwp_bus_write(uint8_t* data, uint32_t len) {
	BusStatus status;

	for (int i = 0; i < len; i++) {
		if ((status = kwp_bus_write_byte(data[i])) != BUS_OK) {
			return status;
		}
		vTaskDelay(KWP_INTERBYTE_DELAY); // must wait 5ms between bytes according to spec
	}
	return BUS_OK;
}

/**
 * Read a stream of data from KWP bus
 *
 * dest: Destination buffer
 * len: Expected length of data to receive
 *
 * Return: Status indicating success or failure
 * */
BusStatus kwp_bus_read(uint8_t* dest, uint32_t len, uint32_t timeout) {
	if (HAL_UART_Receive(&huart4, dest, len, timeout) != HAL_OK) {
		return BUS_RX_ERROR;
	}
	return BUS_OK;
}

/**
 * Calculate checksum for KWP bus message. Note this checksum will always be an
 * 8-bit number (i.e. overflow while calculating is ok and expected)
 *
 * data: Data being sent over bus
 * size: Number of bytes being sent
 *
 * Return: checksum value
 * */
uint8_t kwp_bus_calc_checksum(uint8_t* data, uint32_t size) {
	uint8_t checksum = 0;

	for (int i = 0; i < size; i++) {
		checksum += data[i];
	}
	return checksum;
}

/**
 * Get init response from ECU. After 5 baud init sequence, ECU will send three consecutive
 * bytes.
 *
 * init: Struct to store init bytes to
 *
 * Return: Status indicating success or failure
 * */
BusStatus kwp_bus_get_init_response(KWPInit* init) {
	BusStatus status;

	if ((status = kwp_bus_read_byte(&(init->sync), KWP_BUS_INIT_TIMEOUT)) != BUS_OK) {
		return status;
	}
	if ((status = kwp_bus_read_byte(&(init->kwOne), KWP_BUS_INIT_TIMEOUT)) != BUS_OK) {
		return status;
	}
	if ((status = kwp_bus_read_byte(&(init->kwTwo), KWP_BUS_INIT_TIMEOUT)) != BUS_OK) {
		return status;
	}
	return BUS_OK;
}

/**
 * Initialise the KWP bus for communication
 *
 * Return: Status indicating success or failure
 * */
BusStatus kwp_bus_init(void) {
	KWPInit init = {0};
	uint8_t nkwTwo;
	uint8_t nAddress;

	kwp_bus_five_baud_init();

	if (kwp_bus_get_init_response(&init) != BUS_OK) {
		// couldn't initialise bus
		return BUS_INIT_ERROR;
	}
	nkwTwo = ~init.kwTwo;

	if (kwp_bus_write_byte(nkwTwo) != BUS_OK) {
		return BUS_INIT_ERROR;
	}

	if (kwp_bus_read_byte(&nAddress, KWP_BUS_INIT_TIMEOUT) != BUS_OK) {
		return BUS_INIT_ERROR;
	}

	if (nAddress != ~KWP_BUS_ADDRESS) {
		return BUS_INIT_ERROR;
	}
	// bus is now initialised, note that requests must be made every 5 seconds or bus will
	// need to be initialised again
	return BUS_OK;
}

/**
 * Build a KWP packet for a given array of data
 *
 * dest: Destination array to store packet
 * data: Data to incorporate into packet
 * size: Size of data
 *
 * Return: Number of bytes copied to destination array
 * */
uint8_t kwp_bus_build_packet(uint8_t* dest, uint8_t* data, uint32_t size) {
	if (size > OBD_BUS_REQUEST_MAX) {
		return 0;
	}
	dest[0] = KWP_HEADER_ONE;
	dest[1] = KWP_HEADER_TWO;
	dest[2] = KWP_HEADER_THREE;
	memcpy(dest + KWP_OFFSET_DATA_START, data, size);

	// add checksum to end of array
	dest[KWP_OFFSET_DATA_START + size] = kwp_bus_calc_checksum(data, size);
	return KWP_OFFSET_DATA_START + size;
}

/**
 * Make KWP bus request
 *
 * req: BusRequest struct specifying request to make
 *
 * Return: Status indicating success or failure
 * */
BusStatus kwp_bus_make_request(BusRequest* req) {
	uint8_t msg[OBD_BUS_REQUEST_MAX];
	uint8_t len;

	if ((len = kwp_bus_build_packet(msg, req->data, req->dataLen)) == 0) {
		return BUS_BUFFER_ERROR;
	}

	return kwp_bus_write(msg, len);
}

/**
 * Extract the data from a complete response packet
 *
 * response: Raw KWP response
 * dataSize: Number of bytes in response which are data (given from format byte)
 * dest: Array to store data
 *
 * Return: None
 * */
void kwp_bus_extract_data(uint8_t* response, uint32_t dataSize, uint8_t* dest) {
	// we want to extract the data only from a received packet
	// an example packet may look like 0x83 0xF1 0x11 0x41 0x0D 0x00 0xD3
	// First three bytes are header (1st is format byte), we can see that
	// 0x83 & 0b111111 = 3 so [0x41, 0x0D, 0x00] is our data (0xD3 is checksum)

	for (int i = 0; i < dataSize - 1; i++) {
		dest[i] = response[i + KWP_HEADER_SIZE];
	}
}

/**
 * Get response to request over KWP bus
 *
 * resp: BusReponse struct to store response
 *
 * Return: Status indicating success or failure
 * */
BusStatus kwp_bus_get_response(BusResponse* resp) {
	uint8_t fByte, remain, msgSize;
	uint8_t msg[OBD_BUS_RESPONSE_MAX];
	BusStatus status;

	if ((status = kwp_bus_read(msg, sizeof(uint8_t), 100)) != BUS_OK) {
		return status;
	}

	fByte = msg[0];
	remain = (fByte & KWP_DATA_SIZE_MASK) + KWP_HEADER_SIZE;
	// +1 for format byte
	msgSize = KWP_GET_MSG_SIZE_FROM_FBYTE(fByte);

	if (remain > sizeof(msg)) {
		return BUS_BUFFER_ERROR;
	}

	// we've read the format byte so store remaining starting from msg + 1
	if ((status = kwp_bus_read(msg + 1, remain, 100)) != BUS_OK) {
			return status;
	}
	uint8_t checksum = KWP_GET_CHECKSUM_FROM_MSG(msg, msgSize);

	if (kwp_bus_calc_checksum(msg, msgSize) != checksum) {
		return BUS_CHECKSUM_ERROR;
	}
	kwp_bus_extract_data(msg, fByte & KWP_DATA_SIZE_MASK, resp->data);
	return BUS_OK;
}

/**
 * Send and receive a KWP bus request
 *
 * busReq: Bus request
 * busResp: Struct to store bus response
 *
 * Return: Status indicating success or failure
 * */
BusStatus kwp_bus_handle_request(BusRequest* busReq, BusResponse* busResp) {
	BusStatus status;

	if ((status = kwp_bus_make_request(busReq)) != BUS_OK) {
		return status;
	}

	if ((status = kwp_bus_get_response(busResp)) != BUS_OK) {
		return status;
	}

	return BUS_OK;
}

/**
 * Thread function for KWP task
 *
 * Return: None
 * */
void task_kwp_bus(void) {
	kwp_bus_init();

	BusRequest req = {0};
	BusResponse resp = {0};

	queueKwpRequest = xQueueCreate(QUEUE_KWP_LENGTH, sizeof(BusRequest));
	queueKwpResponse = xQueueCreate(QUEUE_KWP_LENGTH, sizeof(BusResponse));

	for (;;) {
		if (xQueueReceive(queueKwpRequest, &req, 10) == pdTRUE) {
			// OBD controller made a request
			resp.status = kwp_bus_handle_request(&req, &resp);
			// send response to out bound queue
			xQueueSend(queueKwpResponse, &resp, 10);
		}
		vTaskDelay(10);
	}
}

/**
 * Initialise KWP thread
 *
 * Return: None
 * */
void task_init_kwp_bus(void) {
	kwpBus = huart4;
	xTaskCreate((void*) &task_kwp_bus, "TaskKwpBus", TASK_KWP_STACK_SIZE,
			NULL, TASK_KWP_PRIORITY, &handleKwp);
}

