/*
 * dgas_dtc.h
 *
 *  Created on: 8 Jan. 2026
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_DGAS_DTC_H_
#define DGOS_INCLUDE_DGAS_DTC_H_

#include <dgas_types.h>

#define DGAS_DTC_OBD_TIMEOUT	1000

#define DGAS_DTC_GET_UPPER_NIBBLE(byte)	(byte & (0xF0))
#define DGAS_DTC_GET_UPPER_HALF_OF_NIBBLE(nib)	(nib & 0xC0)
#define DGAS_DTC_GET_LOWER_HALF_OF_NIBBLE(nib)	(nib & 0x30)

typedef struct {
	char dStr[10][6];
	uint32_t dCount;
	uint8_t dOverflow;
}DTCReport;

#endif /* DGOS_INCLUDE_DGAS_DTC_H_ */
