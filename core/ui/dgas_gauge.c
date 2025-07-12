/*
 * dgas_gauge.c
 *
 *  Created on: 11 Jul. 2025
 *      Author: rhett
 */

#include <dgas_gauge.h>
#include <dgas_param.h>
#include <dgas_ui.h>
#include <ui.h>
#include <lvgl.h>
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

	while(scale < 4200) {
		if (ui_take_semaphore() == pdTRUE) {
			lv_arc_set_value(objects.gauge_arc, scale);
			ui_give_semaphore();
		}
		vTaskDelay(5);
		scale += 50;
	}
	while (scale >= 0) {
		if (ui_take_semaphore() == pdTRUE) {
			lv_arc_set_value(objects.gauge_arc, scale);
			ui_give_semaphore();
		}
		vTaskDelay(5);
		scale -= 50;
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
 * Initialise gauge UI for use
 *
 * Return: None
 * */
void gauge_init(void) {
	gauge_animate();
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

	for(;;) {
		if (xQueueReceive(queueGaugeUpdate, &update, 10) == pdTRUE) {
			gauge_update(&update);
		}
		if ((uxBits = xEventGroupWaitBits(eventGaugeParam, EVT_GAUGE_PARAM, pdTRUE, pdFALSE, 10))) {
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

