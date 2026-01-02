/*
 * dgas_ui.h
 *
 *  Created on: 6 Jun. 2025
 *      Author: rhett
 */

#ifndef INC_DGAS_UI_H_
#define INC_DGAS_UI_H_

#include <dgas_types.h>
#include <lvgl.h>
#include <ui.h>

// UI task priorities and stack sizes. If the UI task does any drawing
// it should have a reasonably large stack as the call chain of most LVGL
// functions is quite long. The LVGL tick task should have the highest priority
#define TASK_DGAS_UI_STACK_SIZE 			(configMINIMAL_STACK_SIZE * 8)
#define TASK_DGAS_UI_PRIORITY 				(tskIDLE_PRIORITY + 7)
#define TASK_DGAS_LVGL_TICK_STACK_SIZE		(configMINIMAL_STACK_SIZE * 2)
#define TASK_DGAS_LVGL_TICK_PRIORITY		(tskIDLE_PRIORITY + 8)

#define UI_EVENT_PARAMS_MAX		32
#define UI_EVENT_QUEUE_SIZE		8

#define UI_REQUEST_DATA_MAX		128
#define UI_REQUEST_PARAM_MAX	32
#define UI_REQUEST_STRING_MAX	32
#define UI_REQUEST_QUEUE_SIZE	8

#define UI_GAUGE_UPDATE_OBD_STAT_MAX_LEN	32
#define UI_GAUGE_LOAD_PARAM_NAME_MAX_LEN	32
#define UI_GAUGE_LOAD_PARAM_UNIT_MAX_LEN	8

#define UI_DEBUG_FLUSH_STR_LEN		256

#define UI_SUBSYS_COUNT		10

extern QueueHandle_t queueUIEvent;
extern QueueHandle_t queueUIRequest;

/**
 * UI object
 *
 * screen: LVGL screen associated with UI
 * group: Group of eventable LVGL objects
 * size: Number of objects in group
 * */
typedef struct {
	lv_obj_t* screen;
	lv_group_t* group;
	uint32_t size;
}UI;

/**
 * UI ID. ID to identify each unique UI, needed for event
 * dispatching and handling
 * */
typedef enum {
	UI_UID_GAUGE,
	UI_UID_MENU,
	UI_UID_MEAS,
	UI_UID_DEBUG,
	UI_UID_DTC,
	UI_UID_SELFTEST,
	UI_UID_SETTINGS,
	UI_UID_ABOUT
}UID;

/**
 * UI Event codes
 * */
typedef enum {
	UI_EVENT_MEAS_CHANGE_PARAM,

	UI_EVENT_DEBUG_PAUSE,
	UI_EVENT_DEBUG_RESUME,

	UI_EVENT_DTC_GET,
	UI_EVENT_DTC_CLEAR,

	UI_EVENT_SELFTEST_RUN,

	UI_EVENT_SETTINGS_BUS,
	UI_EVENT_SETTINGS_PARAM
}UIEventCode;

/**
 * UI Event descriptor
 *
 * eCode: Event code (see UIEventCode)
 * eParams: Parameters to be passed to event handler
 * eCount: Number of event parameters
 * */
typedef struct {
	UID eUid;
	UIEventCode eCode;
	uint32_t eParams[UI_EVENT_PARAMS_MAX];
	uint32_t eCount;
}UIEvent;

/**
 * UI sub systems
 * */
typedef enum {
	UI_SUBSYS_GLOBAL,
	UI_SUBSYS_GAUGE,
	UI_SUBSYS_MENU,
	UI_SUBSYS_DEBUG,
	UI_SUBSYS_DTC,
	UI_SUBSYS_SELFTEST,
	UI_SUBSYS_SETTINGS,
	UI_SUBSYS_ABOUT
}UISubSys;

/**
 * UI commands
 * */
typedef enum {
	UI_CMD_GAUGE_LOAD,
	UI_CMD_GAUGE_UPDATE,
	UI_CMD_GAUGE_ANIMATE,

	UI_CMD_DEBUG_FLUSH,

	UI_CMD_DTC_SHOW,

	UI_CMD_SELFTEST_RUN,
	UI_CMD_SELFTEST_PROGRESS,
	UI_CMD_SELFTEST_SHOW,
	UI_CMD_SELFTEST_RESET
}UICmd;

/**
 * Gauge update struct for UI update
 *
 * paramVal: Most recent parameter value
 * obdStat: OBD status string
 * vBat: Battery voltage
 * */
typedef struct {
	int gVal;
	char gObd[UI_GAUGE_UPDATE_OBD_STAT_MAX_LEN];
	float gVbat;
}UIGaugeUpdate;

/**
 * Gauge load struct. Used to load new gauge UI
 *
 * lColour: New colour code to use
 * lName: Parameter name
 * lMin: Min arc value
 * lMax: Max arc value
 * */
typedef struct {
	uint32_t lColour;
	char lName[UI_GAUGE_LOAD_PARAM_NAME_MAX_LEN];
	char lUnits[UI_GAUGE_LOAD_PARAM_UNIT_MAX_LEN];
	int32_t lMin;
	int32_t lMax;
}UIGaugeLoad;

/**
 * Debug flush struct. Used to send debug string to flush
 * to UI.
 *
 * dStr: String to flush to debug UI
 * */
typedef struct {
	char dStr[UI_DEBUG_FLUSH_STR_LEN];
}UIDebugFlush;

/**
 * UI accelerometer self test statistics
 *
 * aX: Acceleration in x axis (g's)
 * aY: Acceleration in y axis (g's)
 * aZ: Acceleration in z axis (g's)
 * aWhoAmI: Value of who am I register
 * */
typedef struct {
	float aX;
	float aZ;
	float aY;
	uint8_t aWhoAmI;
}UISelfTestAccStats;

/**
 * UI memory device self test statistics
 *
 * mUsed: Used memory (MiB)
 * mFree: Free memory (MiB)
 * mReadTime: Time taken to read (ms)
 * mWriteTime: Time taken to write (ms)
 * */
typedef struct {
	uint32_t mReadTime;
	uint32_t mSpeed;
	uint32_t mUsed;
	uint32_t mTime;
}UISelfTestMemStats;

/**
 * UISelfTestReport struct
 *
 * sDest: Destination textarea to display device report
 * sAcc: Accelerometer self test statistics
 * sMem: Memory device self test statistics
 * */
typedef struct {
	lv_obj_t* sDest;
	UISelfTestAccStats sAcc;
	UISelfTestMemStats sFlash;
	UISelfTestMemStats sDram;
}UISelfTestReport;

/**
 * UI request struct. Used to request a change to UI
 *
 * uSys: UI subsystem to modify
 * uCmd: UI command to execute
 * uData: Data associated with request
 * */
typedef struct {
	UISubSys uSys;
	UICmd uCmd;
	uint8_t uData[UI_REQUEST_DATA_MAX];
}UIRequest;

typedef void (*UIReqCallback) (UIRequest*);
typedef void (*evtCallback) (lv_event_t*);

/**
 * UI callback function options
 * */
typedef enum {
	UI_CALLBACK_USE_FOR_ALL,
	UI_CALLBACK_USE_FOR_SCREEN
}UICallbackOpt;

// Function prototypes
BaseType_t ui_take_semaphore(void);
void ui_give_semaphore(void);
TaskHandle_t task_dgas_ui_get_handle(void);
TaskHandle_t task_lvgl_update_get_handle(void);
TaskHandle_t task_lvgl_tick_get_handle(void);
void ui_load_screen(UI* ui);
void ui_register_event_callback(UI* ui, evtCallback cb, void* uData, UICallbackOpt opt);
void ui_init_struct(UI* init, lv_obj_t* scrn, lv_obj_t** eventable, uint32_t size);
void ui_dispatch_event(UID eUid, UIEventCode eCode, uint32_t* eParams, uint32_t eCount);
uint8_t ui_request_callback_exists(UISubSys uSys);
void ui_request_register_callback(UISubSys uSys, UIReqCallback cb);
void ui_handle_request(UIRequest* req);
void ui_make_request(UIRequest* req);
void task_dgas_lvgl_update_init(void);
void task_dgas_lvgl_tick_init(void);
void task_dgas_ui_init(void);

#endif /* INC_DGAS_UI_H_ */
