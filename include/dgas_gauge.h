/*
 * dgas_gauge.h
 *
 *  Created on: 11 Jul. 2025
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_DGAS_GAUGE_H_
#define DGOS_INCLUDE_DGAS_GAUGE_H_

#include <dgas_types.h>
#include <lvgl.h>
#include <dgas_obd.h>

#define GAUGE_OBD_STATUS_BUFF_LEN		32
#define GAUGE_PARAM_VAL_BUFF_LEN		32
#define GAUGE_VBAT_BUFF_LEN				32
#define GAUGE_PARAM_MAX_BUFF_LEN		32
#define GAUGE_TICK_BUFF_LEN				32

/**
 * GaugeParam
 *
 * Stores information about a gauge parameter e.g. engine speed, fuel pressure etc
 *
 * min: Minimum value of parameter to show on gauge arc
 * max: Maximum value of parameter to show on gauge arc
 * units: Units of parameter
 * name: Parameter name
 * colour: Colour to use to display value/arc with
 * */
typedef struct {
	OBDPid pid;
	uint32_t min;
	uint32_t max;
	char* units;
	char* name;
	uint32_t colour;
}GaugeParam;

/**
 * GaugeUpdate
 *
 * Stores information needed to update the gauge with latest readings
 *
 * paramVal: Most recent value of parameter taken from OBD-II bus
 * bStat: Most recent bus status
 * vBat: Battery voltage
 * */
typedef struct {
	int paramVal;
	char obdStat[GAUGE_OBD_STATUS_BUFF_LEN];
	float vBat;
}GaugeUpdate;

/**
 * GaugeState
 *
 * Stores information about the current state of the gauge
 *
 * paramVal: Current parameter value
 * obdStat: Current OBD-II bus status
 * vBat: Current battery voltage
 * paramMax: Current maximum value of parameter
 * param: Pointer to currently active gauge parameter
 * */
typedef struct {
	int paramVal;
	char obdStat[GAUGE_OBD_STATUS_BUFF_LEN];
	float vBat;
	int paramMax;
	const GaugeParam* param;
}GaugeState;

extern QueueHandle_t queueGaugeUpdate;
extern EventGroupHandle_t eventGaugeParam;

#define GAUGE_ARC_TICK_COUNT			7 // 7 ticks along main gauge arc

#define GAUGE_ANIM_ARC_START_VALUE		0
#define GAUGE_ANIM_ARC_END_VALUE		4200
#define GAUGE_ANIM_ARC_STEP_SIZE		150

// event definitions
#define EVT_GAUGE_PARAM_RPM				1 << 0
#define EVT_GAUGE_PARAM_SPEED			1 << 1
#define EVT_GAUGE_PARAM_ENGINE_LOAD		1 << 2
#define EVT_GAUGE_PARAM_COOLANT_TEMP	1 << 3
#define EVT_GAUGE_PARAM_BOOST			1 << 4
#define EVT_GAUGE_PARAM_INTAKE_TEMP		1 << 5
#define EVT_GAUGE_PARAM_MAF				1 << 6
#define EVT_GAUGE_PARAM_FUEL_PRESSURE	1 << 7

#define EVT_GAUGE_PARAM					EVT_GAUGE_PARAM_RPM 			| 	EVT_GAUGE_PARAM_SPEED		 | \
										EVT_GAUGE_PARAM_ENGINE_LOAD 	| 	EVT_GAUGE_PARAM_COOLANT_TEMP | \
										EVT_GAUGE_PARAM_BOOST 			| 	EVT_GAUGE_PARAM_INTAKE_TEMP	 | \
										EVT_GAUGE_PARAM_MAF				| 	EVT_GAUGE_PARAM_FUEL_PRESSURE

#define TASK_DGAS_GAUGE_PRIORITY		(tskIDLE_PRIORITY + 4)
#define TASK_DGAS_GAUGE_STACK_SIZE		(configMINIMAL_STACK_SIZE * 8)

TaskHandle_t task_dgas_get_handle_gauge(void);
void gauge_animate(void);
void gauge_adjust_scale_labels(const GaugeParam* param, lv_obj_t** scaleLabels);
void gauge_load_param(const GaugeParam* param);
void gauge_update(GaugeUpdate* update);
void gauge_init(void);
void task_dgas_gauge_init(void);

#endif /* DGOS_INCLUDE_DGAS_GAUGE_H_ */
