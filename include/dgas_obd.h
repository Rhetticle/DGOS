/*
 * dgas_obd.h
 *
 *  Created on: 2 Jun. 2025
 *      Author: rhett
 */

#ifndef INC_DGAS_OBD_H_
#define INC_DGAS_OBD_H_

#include <dgas_types.h>
#include <bus.h>

extern QueueHandle_t queueOBDRequest;
extern QueueHandle_t queueOBDResponse;
extern EventGroupHandle_t eventOBDChangeBus;

#define OBD_BUS_REQUEST_MAX 64
#define OBD_BUS_RESPONSE_MAX 64

typedef enum {
	OBD_OK,
	OBD_INIT,
	OBD_TIMEOUT,
	OBD_ERROR
}OBDStatus;

// OBD modes
typedef enum {
	OBD_MODE_LIVE             = 0x01,
	OBD_MODE_FREEZE           = 0x02,
	OBD_MODE_DTC              = 0x03,
	OBD_MODE_DTC_CLEAR        = 0x04,
	OBD_MODE_MONITOR_OXYGEN   = 0x05,
	OBD_MODE_MONITOR_OTHER    = 0x06,
	OBD_MODE_DTC_PENDING      = 0x07,
	OBD_MODE_ON_BOARD_CONTROL = 0x08,
	OBD_MODE_VEHICLE_INFO     = 0x09,
	OBD_MODE_DTC_PERMANENT    = 0x0A
}OBDMode;

// OBD parameter ID's (PIDs) (note not all are given here only the most common)

#define OBD_PID_LIVE_SUPPORTED_PID     0x00
#define OBD_PID_LIVE_MONITOR_LAMP      0x01
#define OBD_PID_LIVE_FREEZE_CAUSE      0x02
#define OBD_PID_LIVE_FUEL_SYS_STATUS   0x03
#define OBD_PID_LIVE_ENGINE_LOAD       0x04
#define OBD_PID_LIVE_COOLANT_TEMP      0x05
#define OBD_PID_LIVE_STFT_BANK_1       0x06
#define OBD_PID_LIVE_LTFT_BANK_1       0x07
#define OBD_PID_LIVE_STFT_BANK_2 	   0x08
#define OBD_PID_LIVE_LTFT_BANK_2       0x09
#define OBD_PID_LIVE_FUEL_PRESSURE     0x0A
#define OBD_PID_LIVE_BOOST			   0x0B
#define OBD_PID_LIVE_ENGINE_SPEED      0x0C
#define OBD_PID_LIVE_VEHICLE_SPEED     0x0D
#define OBD_PID_LIVE_TIMING_ADVANCE    0x0E
#define OBD_PID_LIVE_INTAKE_AIR_TEMP   0x0F
#define OBD_PID_LIVE_MAF_FLOW_RATE     0x10
#define OBD_PID_LIVE_THROTTLE_POSITION 0x11

// OBD PIDs for OBD mode 9 (vehicle info)

#define OBD_PID_VEHICLE_INFO_SUPPORTED_PID 					  0x00
#define OBD_PID_VEHICLE_INFO_VIN_MESSAGE_COUNT 				  0x01
#define OBD_PID_VEHICLE_INFO_VIN 							  0x02
#define OBD_PID_VEHICLE_INFO_CALIBRATION_MESSAGE_COUNT 		  0x03
#define OBD_PID_VEHICLE_INFO_CALIBRATION_MESSAGE 			  0x04
#define OBD_PID_VEHICLE_INFO_CVN_MESSAGE_COUNT 				  0x05
#define OBD_PID_VEHICLE_INFO_CVN 							  0x06
#define OBD_PID_VEHICLE_INFO_PERFORMANCE_TRACKING 			  0x07
#define OBD_PID_VEHICLE_INFO_SPARK_IGINITION_PERFORMANCE 	  0x08
#define OBD_PID_VEHICLE_INFO_ECU_NAME_MESSAGE_COUNT 		  0x09
#define OBD_PID_VEHICLE_INFO_ECU_NAME 						  0x0A
#define OBD_PID_VEHICLE_INFO_COMPRESSION_IGNITION_PERFORMANCE 0x0B

// conversion macros from OBD-II A,B,C,D bytes to parameter value

#define OBD_CONV_ENGINE_LOAD(A)			(A / 2.55)
#define OBD_CONV_COOLANT_TEMP(A)		(A - 40)
#define OBD_CONV_FUEL_TRIM(A)			((A / 1.28) - 100)
#define OBD_CONV_FUEL_PRESSURE(A)		(3 * A)
#define OBD_CONV_ENGINE_SPEED(A, B)		((256 * A + B) / 4)
#define OBD_CONV_TIMING_ADVANCE(A)		((A / 2) - 64)
#define OBD_CONV_INTAKE_AIR_TEMP(A)		(A - 40)
#define OBD_CONV_MAF(A, B)				((256 * A + B) / 100)
#define OBD_CONV_THROTTLE_POSITION(A)	(A / 2.55)

// index of OBD mode, pid etc within a standard OBD-II response packet
// Responses will take form such as [Mode + 0x40, pid, A, B, C, D] (typical PID response)
// where A, B, C, D are the four data bytes which are to be converted to actual PID value
#define OBD_RESPONSE_MODE_INDEX				0
#define OBD_RESPONSE_PID_INDEX				1
#define OBD_RESPONSE_DATA_START_INDEX		2
#define OBD_RESPONSE_GET_NUMBER_OF_DATA_BYTES(len)		(len - 2)

typedef uint8_t OBDPid;

typedef struct {
	OBDMode mode;
	OBDPid pid;
	uint32_t timeout;
} OBDRequest;

typedef struct {
	OBDMode mode;
	uint8_t data[OBD_BUS_RESPONSE_MAX];
	uint32_t dataLen;
	OBDStatus status;
} OBDResponse;


#define TASK_BUS_CONTROL_PRIORITY 		(tskIDLE_PRIORITY + 3)
#define TASK_BUS_CONTROL_STACK_SIZE 	(configMINIMAL_STACK_SIZE * 4)
#define TASK_BUS_CONTROL_QUEUE_LENGTH 	5

#define EVT_OBD_BUS_CHANGE_KWP 			(1 << 0)
#define EVT_OBD_BUS_CHANGE_9141 		(1 << 1)
#define EVT_OBD_BUS_CHANGE_CAN 			(1 << 2)
#define EVT_OBD_BUS_CHANGE 				(EVT_OBD_BUS_CHANGE_KWP | EVT_OBD_BUS_CHANGE_9141 | EVT_OBD_BUS_CHANGE_CAN)

// Function prototypes
TaskHandle_t task_dgas_obd_get_handle(void);
BaseType_t obd_take_semaphore(void);
void obd_give_semaphore(void);
BusID obd_get_active_bus(void);
int obd_pid_convert(OBDPid pid, uint8_t* data);
uint32_t dgas_obd_get_pid(OBDPid pid, OBDMode mode, uint8_t* dest, uint32_t timeout);
uint32_t dgas_obd_get_dtc(uint8_t* dest);
uint32_t dgas_obd_get_vehicle_info(OBDPid pid, uint8_t* dest);
OBDStatus dgas_obd_handle_request(OBDRequest* req, OBDResponse* resp);
OBDStatus dgas_obd_bus_change_handler(EventBits_t uxBits);
void task_dgas_obd_init(void);

#endif /* INC_DGAS_OBD_H_ */
