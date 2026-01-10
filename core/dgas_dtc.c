/*
 * dgas_dtc.c
 *
 *  Created on: 8 Jan. 2026
 *      Author: rhett
 */

#include <dgas_dtc.h>
#include <ui_dtc.h>
#include <dgas_obd.h>

/**
 * Determine DTC number from 'A' and 'B' OBD-II bytes (See wikiepdia)
 *
 * A: 'A' byte from OBD-II DTC response
 * B: 'B' byte from OBD-II DTC response
 *
 * return: 12-bit DTC number
 * */
uint16_t dgas_dtc_get_num(uint8_t A, uint8_t B) {
	uint16_t num = (((uint16_t) A) << 4) | B;
	return num;
}

/**
 * Decode DTC string from OBD-II response.
 *
 * A: 'A' byte from OBD-II DTC response
 * B: 'B' byte from OBD-II DTC response
 * result: Destination buffer
 *
 * Return: None
 * */
void dgas_dtc_decode_dtc(uint8_t A, uint8_t B, char* result) {
	char classLookUp[] = {'P', 'C', 'B', 'U'};
	char digLookUp[] = {'0', '1', '2', '3'};

	uint8_t nibble = DGAS_DTC_GET_UPPER_NIBBLE(A);
	uint8_t upperHalf = DGAS_DTC_GET_UPPER_HALF_OF_NIBBLE(nibble);
	uint8_t lowerHalf = DGAS_DTC_GET_LOWER_HALF_OF_NIBBLE(nibble);

	result[0] = classLookUp[upperHalf];
	result[1] = digLookUp[lowerHalf];

	sprintf(result + strlen(result), "%X", DGAS_DTC_GET_NUM(A, B));
}

/**
 * Get current OBD-II DTCs.
 *
 * Return: Status indicating success or failure
 * */
DStatus dgas_dtc_get(void) {
	OBDRequest req = {0};
	OBDResponse resp = {0};

	req.mode = OBD_MODE_DTC;
	req.pid = 0; // unused for DTC
	req.timeout = DGAS_DTC_OBD_TIMEOUT;

	if (xQueueSend(queueOBDRequest, &req, DGAS_DTC_OBD_TIMEOUT) != pdTRUE) {
		return DGAS_STATUS_ERROR;
	}
	while(xQueueReceive(queueOBDResponse, &resp, portMAX_DELAY) != pdTRUE);

}

