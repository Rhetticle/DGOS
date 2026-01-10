/*
 * kwp.c
 *
 *  Created on: Jun 1, 2025
 *      Author: rhett
 */

#include <dgas_types.h>
#include <dgas_obd.h>
#include <dgas_debug.h>
#include <kwp.h>
#include <bus.h>
#include <string.h>
#include <stdbool.h>

// queue for bus requests
QueueHandle_t queueKwpRequest;
// queue for bus responses
QueueHandle_t queueKwpResponse;

// stores task handle
static TaskHandle_t handleKwp;
// kwp UART bus
static UART_HandleTypeDef kwpBus;
// buffer for storing responses from interrupt
static uint8_t rxBuff[KWP_UART_RX_BUFF_SIZE];
// stores current index of RX buffer to write to
static uint32_t rxByteCount;


/**
 * Initialise GPIO pins required for UART for KWP bus
 *
 * Return: None
 * */
static void kwp_bus_gpio_init(void) {
	__KWP_UART_GPIO_CLK_EN();
	GPIO_InitTypeDef init = {0};

	// initialise UART GPIO pins
	init.Alternate = GPIO_AF8_UART4;
	init.Mode = GPIO_MODE_AF_PP;
	init.Speed = GPIO_SPEED_HIGH;
	init.Pull = GPIO_NOPULL;

	init.Pin = KWP_UART_TX_PIN;

	HAL_GPIO_Init(KWP_UART_TX_PORT, &init);

	init.Pin = KWP_UART_RX_PIN;

	HAL_GPIO_Init(KWP_UART_RX_PORT, &init);

	// initialise L-Line GPIO Pin
	init.Alternate = 0;
	init.Mode = MODE_OUTPUT;
	init.Speed = GPIO_SPEED_HIGH;
	init.Pull = GPIO_NOPULL;
	init.Pin = L_LINE_PIN;

	HAL_GPIO_Init(L_LINE_PORT, &init);
}

/**
 * Change K line pin to a regular GPIO (needed for slow five baud init)
 *
 * Return: None
 * */
static void kwp_bus_k_gpio(void) {
	GPIO_InitTypeDef gpioInit = {0};
	// De-init UART ready for GPIO
	HAL_UART_DeInit(&kwpBus);

	gpioInit.Pin = K_LINE_PIN;
	gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
	gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
	gpioInit.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(K_LINE_PORT, &gpioInit);
}

/**
 * Initialise UART peripheral for KWP bus (10400 baud rate)
 *
 * Return: None
 * */
static void kwp_bus_uart_init(void) {
	// enable clocks required for UART
	__KWP_UART_CLK_EN();
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
	// setup UART handle for receive interrupt
	HAL_UART_Receive_IT(&kwpBus, rxBuff, sizeof(rxBuff));

	HAL_NVIC_SetPriority(KWP_UART_INSTANCE_IRQN, 6, 0);
	HAL_NVIC_EnableIRQ(KWP_UART_INSTANCE_IRQN);
}

/**
 * Interrupt handler for KWP UART bus
 *
 * Return: None
 * */
void UART4_IRQHandler(void) {
	//if (KWP_UART_INSTANCE->ISR & USART_ISR_RXNE) {
		if (rxByteCount == sizeof(rxBuff)) {
			return;
		}
		rxBuff[rxByteCount] = KWP_UART_INSTANCE->RDR;
		KWP_UART_INSTANCE->ISR &= ~USART_ISR_RXNE;
		rxByteCount++;
	//}
}

/**
 * Initialise hardware required for KWP bus
 *
 * Return: None
 * */
void kwp_bus_init_hardware(void) {
	kwp_bus_gpio_init();
	kwp_bus_uart_init();
}

/**
 * Get the task handle of the KWP bus task
 * */
TaskHandle_t task_kwp_get_handle(void) {
	return handleKwp;
}

/**
 * Perform 5-baud init of KWP bus
 *
 * Return: None
 * */
void kwp_bus_five_baud_init(void) {
	// ensure K pin is setup as GPIO pin
	kwp_bus_k_gpio();
	K_LINE_HIGH();
	L_LINE_HIGH();
	vTaskDelay(1000);
	K_LINE_LOW();
	L_LINE_LOW();
	vTaskDelay(205);
	K_LINE_HIGH();
	L_LINE_HIGH();
	vTaskDelay(405);
	K_LINE_LOW();
	L_LINE_LOW();
	vTaskDelay(405);
	K_LINE_HIGH();
	L_LINE_HIGH();
	vTaskDelay(405);
	K_LINE_LOW();
	L_LINE_LOW();
	vTaskDelay(405);
	K_LINE_HIGH();
	L_LINE_HIGH();
	vTaskDelay(205);
	// change K pin back to TX ready for UART communication
	HAL_GPIO_DeInit(K_LINE_PORT, K_LINE_PIN);
	kwp_bus_uart_init();
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
	BusStatus status;

	taskENTER_CRITICAL();
	if (HAL_UART_Transmit(&kwpBus, &tmp, sizeof(uint8_t), 100) != HAL_OK) {
		taskEXIT_CRITICAL();
		return BUS_TX_ERROR;
	}
	taskEXIT_CRITICAL();
	// the byte we just sent will be echoed back since it's all on the K-Line so we will read it back

	if ((status = kwp_bus_read_byte(&echo, 100)) != BUS_OK) {
		return status;
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
 * timeout: Timeout to use when waiting
 *
 * Return: Status indicating success or failure
 * */
BusStatus kwp_bus_read_byte(uint8_t* dest, uint32_t timeout) {
	uint32_t tickStart = HAL_GetTick();

	while (rxByteCount == 0) {
		if (HAL_GetTick() > tickStart + timeout) {
			return BUS_RX_ERROR;
		}
	}
	*dest = rxBuff[--rxByteCount];
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
			DGAS_DEBUG_LOG_MSG_ERROR_TRANSMIT(BUS_ID_KWP, status);
			return status;
		}
		vTaskDelay(KWP_INTERBYTE_DELAY); // must wait 5ms between bytes according to spec
	}
	DGAS_DEBUG_LOG_MSG_DATA_TRANSMIT(data, len, BUS_ID_KWP);
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
	BusStatus status;

	for (uint32_t i = 0; i < len; i++) {
		if ((status = kwp_bus_read_byte(&dest[i], timeout)) != BUS_OK) {
			DGAS_DEBUG_LOG_MSG_ERROR_RECEIVE(BUS_ID_KWP, status);
			return status;
		}
	}
	DGAS_DEBUG_LOG_MSG_DATA_RECEIVE(dest, len, BUS_ID_KWP);
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
 * bytes. The first byte is known as the synchronisation byte which is followed by two key
 * word bytes.
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

	// initialise hardware
	kwp_bus_init_hardware();
	// perform 5 baud init sequence
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

	if (nAddress != KWP_BUS_NADDRESS) {
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
static uint8_t kwp_bus_build_packet(uint8_t* dest, uint8_t* data, uint32_t size) {
	if (size > OBD_BUS_REQUEST_MAX) {
		return 0;
	}
	// setup headers
	dest[0] = KWP_HEADER_ONE;
	dest[1] = KWP_HEADER_TWO;
	dest[2] = KWP_HEADER_THREE;
	// copy data after headers
	memcpy(dest + KWP_OFFSET_DATA_START, data, size);

	// add checksum to end of array
	dest[KWP_OFFSET_DATA_START + size] = kwp_bus_calc_checksum(dest, size + KWP_HEADER_SIZE);
	return KWP_OFFSET_DATA_START + size + 1; // +1 for checksum
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

	for (uint32_t i = 0; i < dataSize; i++) {
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
BusStatus kwp_bus_get_response(BusResponse* resp, uint32_t timeout) {
	uint8_t fByte, remain, msgSize;
	uint8_t msg[OBD_BUS_RESPONSE_MAX] = {0};
	BusStatus status;

	if ((status = kwp_bus_read(&msg[0], sizeof(uint8_t), timeout)) != BUS_OK) {
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
	if ((status = kwp_bus_read(msg + 1, remain, timeout)) != BUS_OK) {
			return status;
	}
	uint8_t checksum = KWP_GET_CHECKSUM_FROM_MSG(msg, msgSize);

	if (kwp_bus_calc_checksum(msg, msgSize) != checksum) {
		return BUS_CHECKSUM_ERROR;
	}
	kwp_bus_extract_data(msg, fByte & KWP_DATA_SIZE_MASK, resp->data);
	// update response struct with number of data bytes
	resp->dataLen = fByte & KWP_DATA_SIZE_MASK;
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

	if ((status = kwp_bus_get_response(busResp, busReq->timeout)) != BUS_OK) {
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
	//while(kwp_bus_init() != BUS_OK) {
	//	vTaskDelay(100);
	//}
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
	xTaskCreate((void*) &task_kwp_bus, "TaskKwpBus", TASK_KWP_STACK_SIZE,
			NULL, TASK_KWP_PRIORITY, &handleKwp);
}

