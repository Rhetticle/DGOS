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
#include <ui_gauge.h>
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
 * Load a new gauge parameter onto gauge UI
 *
 * param: Pointer to parameter to load
 *
 * Return: None
 * */
void gauge_load_param(const GaugeParam* param) {
	UIGaugeLoad gLoad = {.lColour = param->colour,
						 .lMax = param->max,
						 .lMin = param->min};
	strcpy(gLoad.lName, param->name);
	strcpy(gLoad.lUnits, param->units);

	gState.param = param;
	gState.paramMax = 0;
	gState.paramVal = 0;
	// make request to update gauge UI
	ui_gauge_make_request(UI_CMD_GAUGE_LOAD, &gLoad);
}

/**
 * Update gauge with most recent parameter reading
 *
 * update: Gauge update received from queue
 *
 * Return: None
 * */
void gauge_update(void) {
	UIGaugeUpdate gUpdate = {.gVal = gState.paramVal,
							 .gVbat = gState.vBat};

	strcpy(gUpdate.gObd, gState.obdStat);
	// make request to UI to update gauge
	ui_gauge_make_request(UI_CMD_GAUGE_UPDATE, &gUpdate);
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
	ui_gauge_init();
	//gauge_load_param(&paramCoolant);
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
int gauge_update_state(OBDPid pid, uint32_t timeout) {
	OBDRequest req = {0};
	OBDResponse resp = {0};

	req.mode = OBD_MODE_LIVE;
	req.pid = pid;
	req.timeout = timeout;

	// get most recent voltage readings
	gauge_get_supply_voltage(&(gState.vBat));

	if (obd_take_semaphore() == pdTRUE) {
		// got semaphore so send request and get response

		xQueueSend(queueOBDRequest, &req, 10);
		while(xQueueReceive(queueOBDResponse, &resp, 10) != pdTRUE) {
			vTaskDelay(10);
		}
		obd_give_semaphore();

		// update the status string based on response
		gauge_set_obd_status_string(gState.obdStat, resp.status);
		if (resp.status == OBD_OK) {
			gState.paramVal = obd_pid_convert(pid, resp.data);
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
	EventBits_t uxBits;
	queueGaugeUpdate = xQueueCreate(1, sizeof(GaugeUpdate));
	eventGaugeParam = xEventGroupCreate();
	vTaskDelay(5000);
	gauge_init();

	for(;;) {
		/*
		if (gauge_update_state(gState.param->pid, 100) == 0) {
			// got successful PID value so update gauge
			gauge_update();
		}
		if ((uxBits = xEventGroupWaitBits(eventGaugeParam, EVT_GAUGE_PARAM, pdTRUE, pdFALSE, 10))) {
			// change parameter event occured
			gauge_param_change_handler(uxBits);
		}*/
		ui_gauge_make_request(UI_CMD_GAUGE_ANIMATE, NULL);
		vTaskDelay(5000);
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

