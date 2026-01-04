/*
 * dgas_settings.h
 *
 *  Created on: 2 Jan. 2026
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_DGAS_SETTINGS_H_
#define DGOS_INCLUDE_DGAS_SETTINGS_H_

#include <dgas_param.h>
#include <bus.h>

typedef struct {
	GaugeParamID gid;
	BusID bid;
}GaugeConfig;

#endif /* DGOS_INCLUDE_DGAS_SETTINGS_H_ */
