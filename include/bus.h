/*
 * bus.h
 *
 *  Created on: 6 Jun. 2025
 *      Author: rhett
 */

#ifndef INC_BUS_H_
#define INC_BUS_H_

#include <dgas_types.h>

#ifndef DGAS_CONFIG_BUS_REQUEST_MAX
#define BUS_REQUEST_MAX 64
#else
#define BUS_REQUEST_MAX DGAS_CONFIG_BUS_REQUEST_MAX
#endif /* DGAS_CONFIG_BUS_REQUEST_MAX */

#ifndef DGAS_CONFIG_BUS_RESPONSE_MAX
#define BUS_RESPONSE_MAX 64
#else
#define BUS_RESPONSE_MAX DGAS_CONFIG_BUS_RESPONSE_MAX
#endif /* DGAS_CONFIG_BUS_RESPONSE_MAX */

#if (BUS_RESPONSE_MAX >= BUS_REQUEST_MAX)
#define BUS_TRANSACTION_MAX		BUS_RESPONSE_MAX
#else
#define BUS_TRANSACTION_MAX		BUS_REQUEST_MAX
#endif

// ID's for different OBD buses
typedef enum {
	BUS_ID_KWP,
	BUS_ID_9141,
	BUS_ID_CAN
}BusID;

// Bus status error/success codes
typedef enum {
	BUS_NULL,
	BUS_OK,
	BUS_TX_ERROR,
	BUS_RX_ERROR,
	BUS_ECHO_ERROR,
	BUS_BUFFER_ERROR,
	BUS_CHECKSUM_ERROR,
	BUS_INIT_ERROR
}BusStatus;

// Bus directions
typedef enum {
	BUS_DIR_TRANSMIT,
	BUS_DIR_RECEIVE
}BusDirection;

/**
 * BusRequest
 *
 * Stores information required to make a request on a given OBD bus
 *
 * data: Array of data to send over bus
 * dataLen: Length of data to send
 * timeout: Timeout to use when waiting for response
 * */
typedef struct {
	uint8_t data[BUS_REQUEST_MAX];
	uint32_t dataLen;
	uint32_t timeout;
} BusRequest;

/**
 * BusResponse
 *
 * Stores the response of OBD bus to a request
 *
 * data: Response
 * dataLen: Length of response
 * status: Status of response
 * */
typedef struct {
	uint8_t data[BUS_RESPONSE_MAX];
	uint32_t dataLen;
	BusStatus status;
} BusResponse;

/**
 * BusHandle
 *
 * Stores information about an OBD bus.
 *
 * bid: Bus ID (BUS_ID_KWP etc.)
 * outBound: Pointer to queue for making requests on the bus
 * inBound: Pointer to queue for receiving responses to requests
 * */
typedef struct {
	BusID bid;
	QueueHandle_t* outBound;
	QueueHandle_t* inBound;
} BusHandle;

#endif /* INC_BUS_H_ */
