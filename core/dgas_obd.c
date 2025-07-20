/*
 * dgas_obd.c
 *
 *  Created on: 2 Jun. 2025
 *      Author: rhett
 */

/**
 * DGAS OBD bus controller. The bus controller handles sending and receiving of data
 * over the currently active OBD-II bus. Other tasks can ask controller to make OBD
 * requests on their behalf e.g. dgas_sys
 * */
#include <dgas_types.h>
#include <dgas_sys.h>
#include <dgas_obd.h>
#include <kwp.h>
#include <iso15765.h>
#include <iso9141.h>
#include <bus.h>
#include <string.h>
#include <stdbool.h>

// stores FreeRTOS handle of bus controller task
static TaskHandle_t handleBusControl;
// stores currently active bus being used to make OBD requests
static BusHandle bus;
// semaphore for exclusion to access to OBD controller
static SemaphoreHandle_t semaphoreOBD;
// queue for making OBD requests
QueueHandle_t queueOBDRequest;
// queue for sending OBD responses
QueueHandle_t queueOBDResponse;
// event group to change OBD bus dynamically
EventGroupHandle_t eventOBDChangeBus;

/**
 * Get task handle of OBD controller task
 *
 * Return: Task handle of OBD controller
 * */
TaskHandle_t task_dgas_obd_get_handle(void) {
	return handleBusControl;
}

/**
 * Take OBD semaphore
 *
 * Return: pdTRUE if taken, pdFALSE otherwise
 * */
BaseType_t obd_take_semaphore(void) {
	if (semaphoreOBD != NULL) {
		return xSemaphoreTake(semaphoreOBD, portMAX_DELAY);
	}
	return pdFALSE;
}

/**
 * Give OBD semaphore
 *
 * Return: None
 * */
void obd_give_semaphore(void) {
	xSemaphoreGive(semaphoreOBD);
}

/**
 * Get the bus ID of the currently active OBD-II bus
 *
 * Return: Bus ID of the currently active OBD-II bus
 * */
BusID obd_get_active_bus(void) {
	return bus.bid;
}

/**
 * Convert from the four raw OBD-II bytes (A, B, C, D) to the actual parameter
 * value depending on PID.
 *
 * pid: PID to get value of
 * data: Raw OBD-II bytes from request for PID
 *
 * Return: Actual PID value
 * */
int obd_pid_convert(OBDPid pid, uint8_t* data) {
	uint8_t A = data[0];
	uint8_t B = data[1];

	if (pid == OBD_PID_LIVE_ENGINE_SPEED) {
		return OBD_CONV_ENGINE_SPEED(A, B);
	} else if (pid == OBD_PID_LIVE_COOLANT_TEMP) {
		return OBD_CONV_COOLANT_TEMP(A);
	} else if ((pid == OBD_PID_LIVE_LTFT_BANK_1) || (pid == OBD_PID_LIVE_LTFT_BANK_2) ||
				(pid == OBD_PID_LIVE_STFT_BANK_1) || (pid == OBD_PID_LIVE_STFT_BANK_2)) {
		return OBD_CONV_FUEL_TRIM(A);
	} else if (pid == OBD_PID_LIVE_FUEL_PRESSURE) {
		return OBD_CONV_FUEL_PRESSURE(A);
	} else if (pid == OBD_PID_LIVE_ENGINE_LOAD) {
		return OBD_CONV_ENGINE_LOAD(A);
	} else if (pid == OBD_PID_LIVE_TIMING_ADVANCE) {
		return OBD_CONV_TIMING_ADVANCE(A);
	} else if (pid == OBD_PID_LIVE_INTAKE_AIR_TEMP) {
		return OBD_CONV_INTAKE_AIR_TEMP(A);
	} else if (pid == OBD_PID_LIVE_MAF_FLOW_RATE) {
		return OBD_CONV_MAF(A, B);
	} else if (pid == OBD_PID_LIVE_THROTTLE_POSITION) {
		return OBD_CONV_THROTTLE_POSITION(A);
	}
	return 0;
}

/**
 * Get a measurement parameter
 *
 * pid: PID to get
 * mode: OBD mode to use (both live and vehicle info mode use PIDs)
 * dest: Destination array to store received data
 * timeout: Time to wait for response
 *
 * Return: Status indicating success or failure
 * */
OBDStatus dgas_obd_get_pid(OBDPid pid, OBDMode mode, uint8_t* dest, uint32_t timeout) {
	BusRequest req = {0};
	BusResponse resp = {0};

	req.data[0] = mode;
	req.data[1] = pid;
	req.dataLen = sizeof(uint8_t) + sizeof(uint8_t); // mode and pid are both size uint8_t
	req.timeout = timeout;

	// send request to bus
	xQueueSend(*(bus.outBound), &req, 10);
	// wait for response, we should get a response regardless
	// since we set a timeout on the request
	while(xQueueReceive(*(bus.inBound), &resp, 10) != pdTRUE) {
		vTaskDelay(10);
	}
	// we got response, as per OBD-II spec we should get data of form
	// [OBD mode + 0x40, pid, A, B, C, D] where A, B, C, D are the pid
	// data bytes
	if (resp.status != BUS_OK) {
		return OBD_ERROR;
	} else {
		memcpy(dest, resp.data + 2, resp.dataLen - 2);
	}
	return OBD_OK;
}

/**
 * Get diagnostic trouble codes
 *
 * dest: Destination buffer to store response
 *
 * Return: Status to indicate success or failure
 * */
OBDStatus dgas_obd_get_dtc(uint8_t* dest) {
	BusRequest req = {0};
	BusResponse resp = {0};

	req.data[0] = OBD_MODE_DTC;
	req.dataLen = sizeof(uint8_t);

	// send request to bus
	xQueueSend(*(bus.outBound), &req, 10);
	while(xQueueReceive(*(bus.inBound), &resp, 10) != pdTRUE) {
		vTaskDelay(10);
	}

	if (resp.status != BUS_OK) {
		return OBD_ERROR;
	} else {
		memcpy(dest, resp.data, resp.dataLen);
	}
	return OBD_OK;
}

/**
 * Get a vehicle info PID from vehicle
 *
 * pid: Vehicle info PID to measure
 * dest: Destination buffer to store result
 *
 * Return: status indicating success or failure
 * */
OBDStatus dgas_obd_get_vehicle_info(OBDPid pid, uint8_t* dest) {
	return dgas_obd_get_pid(pid, OBD_MODE_VEHICLE_INFO, dest, 100);
}

/**
 * Handle an OBD request
 *
 * req: Request to make
 * resp: Struct to store response
 *
 * Return: status indicating success or failure
 * */
OBDStatus dgas_obd_handle_request(OBDRequest* req, OBDResponse* resp) {
	if (req->mode == OBD_MODE_LIVE) {
		return dgas_obd_get_pid(req->pid, OBD_MODE_LIVE, resp->data, req->timeout);
	} else if (req->mode == OBD_MODE_DTC) {
		return dgas_obd_get_dtc(resp->data);
	} else if (req->mode == OBD_MODE_VEHICLE_INFO) {
		return dgas_obd_get_vehicle_info(req->pid, resp->data);
	}
	return OBD_OK;
}

/**
 * Handle a OBD bus change. Used to dynamically change which bus is used
 * to make OBD requests
 *
 * uxBits: Event bits from eventOBDChangeBus
 *
 * Return: Status indicating success or failure
 * */
OBDStatus dgas_obd_bus_change_handler(EventBits_t uxBits) {
	if (uxBits & EVT_OBD_BUS_CHANGE_KWP) {
		if (bus.bid == BUS_ID_KWP) {
			// same bus as already being used so don't do anything
			return OBD_OK;
		}
		// initialise bus and set in bound and out bound queues
		task_init_kwp_bus();
		bus.inBound = &queueKwpResponse;
		bus.outBound = &queueKwpRequest;
	} else if (uxBits & EVT_OBD_BUS_CHANGE_9141) {
		return OBD_OK;
	} else if (uxBits & EVT_OBD_BUS_CHANGE_CAN) {
		return OBD_OK;
	}
	return OBD_OK;
}

/**
 * Check if OBD-II bus is ready for communication
 *
 * Return: True if bus is ready, false otherwise
 * */
bool obd_bus_ready(void) {
	if ((*(bus.inBound) == NULL) || (*(bus.outBound) == NULL)) {
		return false;
	}
	return true;
}

/**
 * Thread function for OBD controller task
 *
 * Return: None
 * */
void task_dgas_obd(void) {
	// on init, bus controller will need to be told which bus to use
	// by dgas_sys
	OBDRequest obdReq = {0};
	OBDResponse obdResp = {0};
	EventBits_t uxBits;
	// TODO: get dgas_sys to tell this OBD controller which bus to use
	// based on config stored in flash. The task shouldn't start until it knows which
	// bus to use. For now just default to KWP
	task_init_kwp_bus();

	bus.inBound = &queueKwpResponse;
	bus.outBound = &queueKwpRequest;
	bus.bid = BUS_ID_KWP;

	queueOBDResponse = xQueueCreate(TASK_BUS_CONTROL_QUEUE_LENGTH, sizeof(OBDResponse));
	queueOBDRequest = xQueueCreate(TASK_BUS_CONTROL_QUEUE_LENGTH, sizeof(OBDRequest));
	eventOBDChangeBus = xEventGroupCreate();
	semaphoreOBD = xSemaphoreCreateBinary();
	obd_give_semaphore();

	for(;;) {
		if (xQueueReceive(queueOBDRequest, &obdReq, 10) == pdTRUE) {
			if (!obd_bus_ready()) {
				obdResp.status = OBD_INIT;
			} else {
				// keep track of the mode
				obdResp.mode = obdReq.mode;
				// got request from dgas_sys
				obdResp.status = dgas_obd_handle_request(&obdReq, &obdResp);
			}
			// send the response to dgas_sys
			xQueueSend(queueOBDResponse, &obdResp, 10);
		}
		if ((uxBits = xEventGroupWaitBits(eventOBDChangeBus,
				EVT_OBD_BUS_CHANGE, pdTRUE, pdFALSE, 0))) {
			// got bus change event
			if (dgas_obd_bus_change_handler(uxBits) != OBD_OK) {
				// let dgas_sys know about failure
			}
		}
		vTaskDelay(10);
	}
}

/**
 * Initialise OBD controller task
 *
 * Return: None
 * */
void task_dgas_obd_init(void) {
	xTaskCreate((void*)&task_dgas_obd, "TaskOBD", TASK_BUS_CONTROL_STACK_SIZE,
			NULL, TASK_BUS_CONTROL_PRIORITY, &handleBusControl);
}
