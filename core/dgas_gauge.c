/*
 * dgas_gauge.c
 *
 *  Created on: 11 Jul. 2025
 *      Author: rhett
 */

#include <dgas_gauge.h>
#include <dgas_param.h>
#include <dgas_adc.h>
#include <dgas_obd.h>
#include <dgas_ui.h>
#include <string.h>
#include <stdio.h>

// Task handle for gauge task
static TaskHandle_t taskHandleGauge;
// Stores gauge state for gauge
static GaugeState gState;
// Queue for receiving gauge updates
QueueHandle_t queueGaugeUpdate;
// event group for changing gauge parameters
EventGroupHandle_t eventGaugeParam;

/**
 * Get task handle of gauge task
 *
 * Return: Task handle of gauge task
 * */
TaskHandle_t task_dgas_get_handle_gauge(void) {
	return taskHandleGauge;
}

/**
 * Perform start-up animation of gauge.
 *
 * Return: None
 * */
void gauge_animate(void) {
	int16_t scale = 0;

	while(scale < GAUGE_ANIM_ARC_END_VALUE) {
		if (ui_take_semaphore() == pdTRUE) {
			lv_arc_set_value(objects.gauge_arc, scale);
			ui_give_semaphore();
		}
		vTaskDelay(5);
		scale += GAUGE_ANIM_ARC_STEP_SIZE;
	}
	while (scale >= GAUGE_ANIM_ARC_START_VALUE) {
		if (ui_take_semaphore() == pdTRUE) {
			lv_arc_set_value(objects.gauge_arc, scale);
			ui_give_semaphore();
		}
		vTaskDelay(5);
		scale -= GAUGE_ANIM_ARC_STEP_SIZE;
	}
}

/**
 * Adjust the gauge tick labels for a new parameter.
 *
 * param: New parameter being loaded
 * scaleLabels: Array of scale tick objects to update
 *
 * Return: None
 * */
void gauge_adjust_scale_labels(const GaugeParam* param, lv_obj_t** scaleLabels) {
	uint16_t step = (param->max - param->min) / (GAUGE_ARC_TICK_COUNT - 1);

	for (int i = 0; i < GAUGE_ARC_TICK_COUNT; i++) {
		char buff[GAUGE_TICK_BUFF_LEN];
		sprintf(buff, "%ld", param->min + (i * step));
		lv_label_set_text(scaleLabels[i], buff);
	}
}

/**
 * Load a new gauge parameter onto gauge UI
 *
 * param: Pointer to parameter to load
 *
 * Return: None
 * */
void gauge_load_param(const GaugeParam* param) {
	lv_obj_t* scaleLabels[] = {objects.gauge_tick_0, objects.gauge_tick_1, objects.gauge_tick_2,
							   objects.gauge_tick_3, objects.gauge_tick_4, objects.gauge_tick_5,
							   objects.gauge_tick_6};

	gState.param = param;
	gState.paramMax = 0;
	gState.paramVal = 0;

	if (ui_take_semaphore() == pdTRUE) {
		// update gauge arc, parameter name and units
		lv_arc_set_range(objects.gauge_arc, param->min, param->max);
		lv_scale_set_range(objects.gauge_scale, param->min, param->max);
		lv_label_set_text(objects.parameter_label, param->name);
		lv_label_set_text(objects.param_units_label, param->units);

		// update colour of colour-coded objects
		lv_obj_set_style_arc_color(objects.gauge_arc, lv_color_hex(param->colour),
									LV_PART_INDICATOR | LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(objects.param_val, lv_color_hex(param->colour),
									LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(objects.parameter_label, lv_color_hex(param->colour),
									LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(objects.param_max_label, lv_color_hex(param->colour),
									LV_PART_MAIN | LV_STATE_DEFAULT);

		gauge_adjust_scale_labels(param, scaleLabels);
		ui_give_semaphore();
	}

}

/**
 * Update gauge with most recent parameter reading
 *
 * update: Gauge update received from queue
 *
 * Return: None
 * */
void gauge_update(GaugeUpdate* update) {
	// UI operations are expensive so only update items if their
	// values have actually changed
	if (update->paramVal != gState.paramVal) {
		char buff[GAUGE_PARAM_VAL_BUFF_LEN];
		sprintf(buff, "%i", update->paramVal);
		// parameter value has changed so update it
		if (ui_take_semaphore() == pdTRUE) {
			lv_arc_set_value(objects.gauge_arc, update->paramVal);
			lv_label_set_text(objects.param_val, buff);
			ui_give_semaphore();
		}
		// update state
		gState.paramVal = update->paramVal;

		if (update->paramVal > gState.paramMax) {
			// param value has exceeded current maximum so update label
			if (ui_take_semaphore() == pdTRUE) {
				lv_label_set_text(objects.param_max_label, buff);
				ui_give_semaphore();
			}
			gState.paramMax = update->paramVal;
		}
	}
	if (strcmp(update->obdStat, gState.obdStat)) {
		// status string is different so update it
		if (ui_take_semaphore() == pdTRUE) {
			lv_label_set_text(objects.obd_status_label, update->obdStat);
			ui_give_semaphore();
		}
		// update stat
		strcpy(gState.obdStat, update->obdStat);
	}
	if (update->vBat != gState.vBat) {
		char buff[GAUGE_VBAT_BUFF_LEN];
		sprintf(buff, "%.1fV", update->vBat);

		if (ui_take_semaphore() == pdTRUE) {
			lv_label_set_text(objects.vbat_label, buff);
			ui_give_semaphore();
		}
	}
}

/**
 * Set the OBD status string based on OBDStatus value
 *
 * dest: Destination string
 * status: OBDStatus returned
 *
 * Return: None
 * */
void gauge_set_obd_status_string(char* dest, OBDStatus status) {
	if (status == OBD_OK) {
		sprintf(dest, "#00FF00 OK#");
	} else if (status == OBD_ERROR) {
		sprintf(dest, "#FF0000 ERROR#");
	} else if (status == OBD_INIT) {
		sprintf(dest, "#00FFFF INIT#");
	}
}

/**
 * Initialise gauge UI for use
 *
 * Return: None
 * */
void gauge_init(void) {
	gauge_animate();
	gauge_load_param(&paramCoolant);
}

/**
 * Event handler for changing gauge parameter event
 *
 * uxBits: Event bits read from event group
 *
 * Return: None
 * */
void gauge_param_change_handler(EventBits_t uxBits) {
	// load parameter based on event bits
	if (uxBits & EVT_GAUGE_PARAM_RPM) {
		gauge_load_param(&paramRPM);
	} else if (uxBits & EVT_GAUGE_PARAM_SPEED) {
		gauge_load_param(&paramSpeed);
	} else if (uxBits & EVT_GAUGE_PARAM_ENGINE_LOAD) {
		gauge_load_param(&paramEngineLoad);
	} else if (uxBits & EVT_GAUGE_PARAM_COOLANT_TEMP) {
		gauge_load_param(&paramCoolant);
	} else if (uxBits & EVT_GAUGE_PARAM_BOOST) {
		gauge_load_param(&paramBoost);
	} else if (uxBits & EVT_GAUGE_PARAM_INTAKE_TEMP) {
		gauge_load_param(&paramAirTemp);
	} else if (uxBits & EVT_GAUGE_PARAM_MAF) {
		gauge_load_param(&paramMAF);
	} else if (uxBits & EVT_GAUGE_PARAM_FUEL_PRESSURE) {
		gauge_load_param(&paramFuelPressure);
	}
}

/**
 * Get most recent supply voltage measurement
 *
 * dest: Pointer to variable to store result
 *
 * Return: None
 * */
void gauge_get_supply_voltage(float* dest) {
	if (queueADC != NULL) {
		xQueueReceive(queueADC, dest, 0);
	}
}

/**
 * Get update on a given OBD-II parameter to display on gauge
 *
 * update: Pointer to GaugeUpdate struct to populate
 * pid: PID of parameter to get update on
 * timeout: Timeout to use when waiting for response
 *
 * Return: 0 if successfull update was received, 1 if failure occured
 * */
int gauge_get_update(GaugeUpdate* update, OBDPid pid, uint32_t timeout) {
	OBDRequest req = {0};
	OBDResponse resp = {0};

	req.mode = OBD_MODE_LIVE;
	req.pid = pid;
	req.timeout = timeout;

	// get most recent voltage readings
	gauge_get_supply_voltage(&(update->vBat));

	if (obd_take_semaphore() == pdTRUE) {
		// got semaphore so send request and get response

		xQueueSend(queueOBDRequest, &req, 10);
		while(xQueueReceive(queueOBDResponse, &resp, 10) != pdTRUE) {
			vTaskDelay(10);
		}
		obd_give_semaphore();

		// update the status string based on response
		gauge_set_obd_status_string(update->obdStat, resp.status);
		if (resp.status == OBD_OK) {
			update->paramVal = obd_pid_convert(pid, resp.data);
		}
		return 0;
	}
	return 1;
}

/**
 * Task function for gauge task
 *
 * Return: None
 * */
void task_dgas_gauge(void) {
	GaugeUpdate update = {0};
	EventBits_t uxBits;
	queueGaugeUpdate = xQueueCreate(1, sizeof(GaugeUpdate));
	eventGaugeParam = xEventGroupCreate();
	gauge_init();
	dgas_task_adc_init();

	for(;;) {
		if (gauge_get_update(&update, gState.param->pid, 100) == 0) {
			// got successfull PID value so update gauge
			gauge_update(&update);
		}
		if ((uxBits = xEventGroupWaitBits(eventGaugeParam, EVT_GAUGE_PARAM, pdTRUE, pdFALSE, 10))) {
			// change parameter event occured
			gauge_param_change_handler(uxBits);
		}
		vTaskDelay(10);
	}
}

/**
 * Initialise gauge task
 *
 * Return: None
 * */
void task_dgas_gauge_init(void) {
	xTaskCreate((void*) &task_dgas_gauge, "Gauge", TASK_DGAS_GAUGE_STACK_SIZE,
			NULL, TASK_DGAS_GAUGE_PRIORITY, &taskHandleGauge);
}

