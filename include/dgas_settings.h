/*
 * dgas_settings.h
 *
 *  Created on: 2 Jan. 2026
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_DGAS_SETTINGS_H_
#define DGOS_INCLUDE_DGAS_SETTINGS_H_

#include <dgas_param.h>
#include <dgas_types.h>
#include <bus.h>

typedef struct {
	GaugeParamID gid;
	BusID bid;
}GaugeConfig;

#define DGAS_SETTINGS_FLASH_CONFIG_ADDR	0x00000000
#define DGAS_SETTINGS_FLASH_TIMEOUT		1000

DStatus dgas_settings_config_save(GaugeConfig* conf);
DStatus dgas_settings_config_read(GaugeConfig* dest);

#endif /* DGOS_INCLUDE_DGAS_SETTINGS_H_ */
