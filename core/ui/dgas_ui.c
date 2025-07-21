/*
 * dgas_ui.c
 *
 *  Created on: 6 Jun. 2025
 *      Author: rhett
 */

#include <dgas_types.h>
#include <dgas_ui.h>
#include <dgas_gauge.h>
#include <dgas_debug.h>
#include <display.h>
#include <dram.h>
#include <flash.h>
#include <lvgl.h>
#include <buttons.h>
#include <ui.h>
#include <stdio.h>

// Stores handle of UI controller task
static TaskHandle_t taskHandleDgasUi;
// Stores handle of LVGL update task
static TaskHandle_t taskHandleLVGLUpdate;
// Stores handle of LVGL tick task
static TaskHandle_t taskHandleLVGLTick;
// semaphore to synchronise UI operations, any task which wishes to modify
// the UI in some way must acquire this semaphore beforehand as LVGL will
// update UI asynchronously to other tasks which can cause visual UI bugs
static SemaphoreHandle_t semaphoreUI;
// LVGL display
static lv_display_t* display;
// LVGL input device (encoder)
static lv_indev_t* indevEnc;
// UIs
static UI uiGauge, uiMenu, uiMeas, uiDebug, uiDTC, uiSelfTest, uiSettings, uiAbout;

/**
 * Take UI semaphore
 *
 * Return: None
 * */
BaseType_t ui_take_semaphore(void) {
	if (semaphoreUI != NULL) {
		return xSemaphoreTake(semaphoreUI, portMAX_DELAY);
	}
	return pdFALSE;
}

/**
 * Give UI semaphore
 *
 * Return: None
 * */
void ui_give_semaphore(void) {
	if (semaphoreUI != NULL) {
		xSemaphoreGive(semaphoreUI);
	}
}

/**
 * LVGL encoder read callback function. Although a physical encoder is not being
 * used, we can make LVGL think one is where navigation button presses correspond
 * to rotation of encoder and selection button pressed correspond to encoder presses
 *
 * indev: LVGL input device
 * data: LVGL input device data
 *
 * Return: None
 * */
static void encoder_read(lv_indev_t* indev, lv_indev_data_t* data) {
	EventBits_t uxBits = xEventGroupWaitBits(eventButtons, EVT_BUTTON_PRESSED,
											pdTRUE, pdFALSE, 0);

	if (uxBits & EVT_BUTTON_NAV_PRESSED) {
		// navigation button pressed so increment encoder position
		data->enc_diff++;
	}
	if (uxBits & EVT_BUTTON_SEL_PRESSED) {
		if (lv_screen_active() == objects.gauge_main_ui) {
			// special case when select button is pressed while on main gauge screen
			// Since there are no buttons we must load the menu UI manually here
			ui_load_screen(&uiMenu);
		} else {
			data->state = LV_INDEV_STATE_PRESSED;
		}
	} else {
		data->state = LV_INDEV_STATE_RELEASED;
	}
}

/**
 * Frame buffer flush function to be used with LVGL
 *
 * Return: None
 * */
static void ui_flush_frame_buffer(lv_display_t* disp, const lv_area_t* area, uint8_t* map) {
	// since we're using LTDC with DMA2D we simply need to change the LTDC frame
	// buffer pointer to the other frame buffer since double buffering is being used
	if(lv_display_is_double_buffered(disp) && lv_display_flush_is_last(disp)) {
		display_flush_frame_buffer(map);
	}
	lv_display_flush_ready(disp);
}

/**
 * LVGL event callback function for menu screen.
 *
 * evt: Pointer to LVGL event object
 *
 * Return: None
 * */
static void ui_event_callback_menu(lv_event_code_t code, lv_obj_t* focus) {
	if (code == LV_EVENT_CLICKED) {
		if (focus == objects.measure_btn) {
			ui_load_screen(&uiMeas);
		} else if (focus == objects.obd2_debug_btn) {
			ui_load_screen(&uiDebug);
		} else if (focus == objects.diagnose_btn) {
			ui_load_screen(&uiDTC);
		} else if (focus == objects.self_test_btn) {
			ui_load_screen(&uiSelfTest);
		} else if (focus == objects.settings_btn) {
			ui_load_screen(&uiSettings);
		} else if (focus == objects.about_btn) {
			ui_load_screen(&uiAbout);
		} else if (focus == objects.menu_exit_btn) {
			ui_load_screen(&uiGauge);
		}
	}
}

/**
 * LVGL event callback function for measure screen.
 *
 * evt: Pointer to LVGL event object
 *
 * Return: None
 * */
static void ui_event_callback_meas(lv_event_code_t code, lv_obj_t* focus) {
	if (code == LV_EVENT_CLICKED) {
		if (focus == objects.eng_speed_btn) {
			xEventGroupSetBits(eventGaugeParam, EVT_GAUGE_PARAM_RPM);
		} else if (focus == objects.vehicle_speed_btn) {
			xEventGroupSetBits(eventGaugeParam, EVT_GAUGE_PARAM_SPEED);
		} else if (focus == objects.eng_load_btn) {
			xEventGroupSetBits(eventGaugeParam, EVT_GAUGE_PARAM_ENGINE_LOAD);
		} else if (focus == objects.coolant_temp_btn) {
			xEventGroupSetBits(eventGaugeParam, EVT_GAUGE_PARAM_COOLANT_TEMP);
		} else if (focus == objects.boost_btn) {
			xEventGroupSetBits(eventGaugeParam, EVT_GAUGE_PARAM_BOOST);
		} else if (focus == objects.intake_temp_btn) {
			xEventGroupSetBits(eventGaugeParam, EVT_GAUGE_PARAM_INTAKE_TEMP);
		} else if (focus == objects.maf_btn) {
			xEventGroupSetBits(eventGaugeParam, EVT_GAUGE_PARAM_MAF);
		} else if (focus == objects.fuel_pressure_btn) {
			xEventGroupSetBits(eventGaugeParam, EVT_GAUGE_PARAM_FUEL_PRESSURE);
		}
		ui_load_screen(&uiGauge);
	}
}

/**
 * LVGL event callback function for debug screen.
 *
 * evt: Pointer to LVGL event object
 *
 * Return: None
 * */
static void ui_event_callback_debug(lv_event_code_t code, lv_obj_t* focus) {
	if (code == LV_EVENT_CLICKED) {
		if (focus == objects.obd2_exit_btn) {
			ui_load_screen(&uiMenu);
		} else if (focus == objects.obd2_pause_btn) {
			dgas_debug_pause();
		} else if (focus == objects.obd2_resume_btn) {
			dgas_debug_resume();
		}
	}

}

/**
 * LVGL event callback function for DTC screen.
 *
 * evt: Pointer to LVGL event object
 *
 * Return: None
 * */
static void ui_event_callback_dtc(lv_event_code_t code, lv_obj_t* focus) {
	if (code == LV_EVENT_CLICKED) {
		if (focus == objects.diagnose_exit_btn) {
			ui_load_screen(&uiMenu);
		} else if (focus == objects.diagnose_clear_btn) {
			// clear DTCs
		}
	}
}

/**
 * LVGL event callback function for self test screen.
 *
 * evt: Pointer to LVGL event object
 *
 * Return: None
 * */
static void ui_event_callback_self_test(lv_event_code_t code, lv_obj_t* focus) {
	if (code == LV_EVENT_CLICKED) {
		if (focus == objects.self_test_exit_btn) {
			ui_load_screen(&uiMenu);
		} else if (focus == objects.self_test_run_btn) {
			// run self test
		}
	}
}

/**
 * LVGL event callback function for settings screen.
 *
 * evt: Pointer to LVGL event object
 *
 * Return: None
 * */
static void ui_event_callback_settings(lv_event_code_t code, lv_obj_t* focus) {
	if (code == LV_EVENT_CLICKED) {
		if (focus == objects.settings_exit_btn) {
			ui_load_screen(&uiMenu);
		}
	}
}

/**
 * LVGL event callback function for about screen.
 *
 * evt: Pointer to LVGL event object
 *
 * Return: None
 * */
static void ui_event_callback_about(lv_event_code_t code, lv_obj_t* focus) {
	if (code == LV_EVENT_CLICKED) {
		if (focus == objects.about_exit_btn) {
			ui_load_screen(&uiMenu);
		}
	}
}

/**
 * Main UI event callback function, this function will then call the corresponding
 * callback function based on the currently active UI
 *
 * evt: Pointer to LVGL event object
 *
 * Return: None
 * */
static void ui_event_callback(lv_event_t* evt) {
	lv_event_code_t code = lv_event_get_code(evt);
	lv_group_t* group = (lv_group_t*) lv_event_get_user_data(evt);
	lv_obj_t* focus = lv_group_get_focused(group);

	// call event callback function based on group passed
	if (group == uiMenu.group) {
		ui_event_callback_menu(code, focus);
	} else if (group == uiMeas.group) {
		ui_event_callback_meas(code, focus);
	} else if (group == uiDebug.group) {
		ui_event_callback_debug(code, focus);
	} else if (group == uiDTC.group) {
		ui_event_callback_dtc(code, focus);
	} else if (group == uiSelfTest.group) {
		ui_event_callback_self_test(code, focus);
	} else if (group == uiSettings.group) {
		ui_event_callback_settings(code, focus);
	} else if (group == uiAbout.group) {
		ui_event_callback_about(code, focus);
	}
}

/**
 * Get task handle of DGAS UI task
 *
 * Return: Task handle of DGAS UI task
 * */
TaskHandle_t task_dgas_ui_get_handle(void) {
	return taskHandleDgasUi;
}

/**
 * Get task handle of LVGL update task
 *
 * Return: Task handle of LVGL update task
 * */
TaskHandle_t task_lvgl_update_get_handle(void) {
	return taskHandleLVGLUpdate;
}

/**
 * Get task handle of LVGL tick task
 *
 * Return: Task handle of LVGL tick task
 * */
TaskHandle_t task_lvgl_tick_get_handle(void) {
	return taskHandleLVGLTick;
}

/**
 * Load a new screen onto display. Will load screen LVGL object and
 * set indev group to group for that specific screen
 *
 * Return: None
 * */
void ui_load_screen(UI* ui) {
	lv_obj_t* prevActive = lv_screen_active();
	lv_screen_load(ui->screen);
	lv_indev_set_group(indevEnc, ui->group);

	if ((ui == &uiMenu) && (prevActive != objects.gauge_main_ui)) {
		// simple trick to focus the last selected
		// button on the menu screen but only if the previous screen
		// wasn't the gauge (meaning the user just opened the menu)
		lv_group_focus_prev(ui->group);
		lv_group_focus_next(ui->group);
	} else {
		// otherwise set focus to first object in group
		lv_group_focus_obj(lv_group_get_obj_by_index(ui->group, 0));
	}
}

/**
 * Register an event callback function for a given UI.
 *
 * ui: Pointer to UI struct to use callback with
 * çb: Callback function to register
 * uData: Pointer to user data (NULL if not used)
 * opt: Callback option
 *
 * Return: None
 * */
void ui_register_event_callback(UI* ui, evtCallback cb, void* uData, UICallbackOpt opt) {
	for (uint32_t i = 0; i < ui->size; i++) {
		if(opt == UI_CALLBACK_USE_FOR_ALL) {
			// register callback with each object in UI group
			lv_obj_t* obj = lv_group_get_obj_by_index(ui->group, i);
			lv_obj_add_event_cb(obj, cb, LV_EVENT_ALL, uData);
		} else if (opt == UI_CALLBACK_USE_FOR_SCREEN) {
			lv_obj_add_event_cb(ui->screen, cb, LV_EVENT_ALL, uData);
			break;
		}
	}
}

/**
 * Initialise UI object.
 *
 * init: UI struct to initialise
 * scrn: Screen associated with UI
 * eventable: Screen objects which can cause event (i.e. user interactable)
 * cb: Event callback function
 * size: Number of eventable objects
 *
 * Return: None
 * */
void ui_init_struct(UI* init, lv_obj_t* scrn, lv_obj_t** eventable, uint32_t size) {
	init->screen = scrn;
	init->group = lv_group_create();
	init->size = size;

	for (uint32_t i = 0; i < size; i++) {
		// add objects to UI group
		lv_group_add_obj(init->group, eventable[i]);
	}
}

/**
 * Intialise ui structs
 *
 * Return: None
 * */
void ui_init_all_uis(void) {
	// eventable/interactable objects for each UI
	lv_obj_t* menuEventable[] 	  = {objects.measure_btn,
									 objects.obd2_debug_btn,
									 objects.diagnose_btn,
									 objects.self_test_btn,
									 objects.settings_btn,
									 objects.about_btn,
									 objects.menu_exit_btn};

	lv_obj_t* measEventable[]     = {objects.eng_speed_btn,
								     objects.vehicle_speed_btn,
								     objects.eng_load_btn,
								     objects.coolant_temp_btn,
								     objects.boost_btn,
								     objects.intake_temp_btn,
								     objects.maf_btn,
								     objects.fuel_pressure_btn};

	lv_obj_t* debugEventable[] 	  = {objects.obd2_pause_btn,
								  	 objects.obd2_resume_btn,
								     objects.obd2_exit_btn};

	lv_obj_t* dtcEventable[]      = {objects.diagnose_clear_btn,
								  	 objects.diagnose_exit_btn};

	lv_obj_t* selfTestEventable[] = {objects.self_test_run_btn,
									 objects.self_test_exit_btn};

	lv_obj_t* settingsEventable[] = {objects.settings_param_dropdown,
									 objects.settings_bus_dropdown,
									 objects.settings_exit_btn};

	lv_obj_t* aboutEventable[]    = {objects.about_exit_btn};

	// initialise UI structs
	ui_init_struct(&uiGauge, objects.gauge_main_ui, NULL, 0);

	ui_init_struct(&uiMenu, objects.menu, menuEventable, sizeof(menuEventable)/sizeof(lv_obj_t*));

	ui_init_struct(&uiMeas, objects.measure, measEventable, sizeof(measEventable)/sizeof(lv_obj_t*));

	ui_init_struct(&uiDebug, objects.obd2_debug, debugEventable, sizeof(debugEventable)/sizeof(lv_obj_t*));

	ui_init_struct(&uiDTC, objects.diagnose, dtcEventable, sizeof(dtcEventable)/sizeof(lv_obj_t*));

	ui_init_struct(&uiSelfTest, objects.self_test, selfTestEventable, sizeof(selfTestEventable)/sizeof(lv_obj_t*));

	ui_init_struct(&uiSettings, objects.settings, settingsEventable, sizeof(settingsEventable)/sizeof(lv_obj_t*));

	ui_init_struct(&uiAbout, objects.about, aboutEventable, sizeof(aboutEventable)/sizeof(lv_obj_t*));

	// register the callback functions for each UI
	ui_register_event_callback(&uiMenu, &ui_event_callback, (void*) uiMenu.group,
			UI_CALLBACK_USE_FOR_ALL);

	ui_register_event_callback(&uiMeas, &ui_event_callback, (void*) uiMeas.group,
			UI_CALLBACK_USE_FOR_ALL);

	ui_register_event_callback(&uiDebug, &ui_event_callback, (void*) uiDebug.group,
			UI_CALLBACK_USE_FOR_ALL);

	ui_register_event_callback(&uiDTC, &ui_event_callback, (void*) uiDTC.group,
			UI_CALLBACK_USE_FOR_ALL);

	ui_register_event_callback(&uiSelfTest, &ui_event_callback, (void*) uiSelfTest.group,
			UI_CALLBACK_USE_FOR_ALL);

	ui_register_event_callback(&uiSettings, &ui_event_callback, (void*) uiSettings.group,
			UI_CALLBACK_USE_FOR_ALL);

	ui_register_event_callback(&uiAbout, &ui_event_callback, (void*) uiAbout.group,
			UI_CALLBACK_USE_FOR_ALL);
}

/**
 * Initialise LVGL related functionality for UI
 *
 * Return: None
 * */
void ui_init_lvgl(void) {
	lv_init();
	// create display object and set start address of both frame buffers
	display = lv_display_create(LCD_RESOLUTION_X, LCD_RESOLUTION_Y);
#ifdef DGAS_CONFIG_USE_DOUBLE_BUFFERING
	lv_display_set_buffers(display, (void*) LCD_FRAME_BUFF_ONE_ADDR, (void*) LCD_FRAME_BUFF_TWO_ADDR,
							LCD_FRAME_BUFF_SIZE, LV_DISP_RENDER_MODE_DIRECT);
	// set the flush callback function for LVGL
#else
	lv_display_set_buffers(display, (void*) UI_FRAME_BUFF_ONE_ADDR, (void*) NULL,
							UI_FRAME_BUFF_SIZE, LV_DISP_RENDER_MODE_DIRECT);
#endif /* DGAS_CONFIG_USE_DOUBLE_BUFFERING */
	lv_display_set_flush_cb(display, ui_flush_frame_buffer);
	// create input device and set type and read callback function
	indevEnc = lv_indev_create();
	lv_indev_set_type(indevEnc, LV_INDEV_TYPE_ENCODER);
	lv_indev_set_read_cb(indevEnc, &encoder_read);
}

/**
 * LVGL update task function.
 *
 * Return: None
 * */
void task_lvgl(void) {
	for(;;) {
		if (ui_take_semaphore() == pdTRUE) {
			lv_task_handler();
			ui_give_semaphore();
		}
		vTaskDelay(5);
	}
}

/**
 * Thread function for DGAS UI task
 *
 * Return: None
 * */
void task_dgas_ui(void) {
	uint8_t byte = 0xAA;
	uint32_t count = 0;
	TaskHandle_t task;
	semaphoreUI = xSemaphoreCreateBinary();
	ui_give_semaphore();
	dram_init();
	display_init();
	flash_init();
	flash_enable_memory_mapped();
	task_init_buttons();
	ui_init_lvgl();
	// EEZ init
	ui_init();
	ui_init_all_uis();
	ui_load_screen(&uiGauge);
	task_dgas_lvgl_tick_init();
	task_dgas_lvgl_update_init();
	task_dgas_gauge_init();
	task_dgas_debug_init();

	for(;;) {
		if (streamDebug != NULL) {
			xStreamBufferSend(streamDebug, &byte, 1, 0);
			count++;
			if (count == 10) {
				task = task_dgas_debug_get_handle();
				if (task != NULL) {
					xTaskNotify(task, 10, eNoAction);
				}
				count = 0;
			}
		}
		vTaskDelay(100);
	}
}

/**
 * LVGL tick task function. Increments LVGL's tick counter
 * every 1ms.
 *
 * Return: None
 * */
void task_lvgl_tick(void) {
	for(;;) {
		lv_tick_inc(1);
		vTaskDelay(1);
	}
}

/**
 * Initialise LVGL update/drawing task
 *
 * Return: None
 * */
void task_dgas_lvgl_update_init(void) {
	xTaskCreate((void*) &task_lvgl, "TaskLVGL", TASK_DGAS_LVGL_UPDATE_STACK_SIZE,
			NULL, TASK_DGAS_LVGL_UPDATE_PRIORITY, &taskHandleLVGLUpdate);
}

/**
 * Initialise LVGL tick task
 *
 * Return: None
 * */
void task_dgas_lvgl_tick_init(void) {
	// create LVGL tick task
	xTaskCreate((void*) &task_lvgl_tick, "TaskLVGLTick", TASK_DGAS_LVGL_TICK_STACK_SIZE,
			NULL, TASK_DGAS_LVGL_TICK_PRIORITY, &taskHandleLVGLTick);
}

/**
 * Initialise UI controller task
 *
 * Return: None
 * */
void task_dgas_ui_init(void) {
	// create UI controller task
	xTaskCreate((void*) &task_dgas_ui, "TaskDgasUi", TASK_DGAS_UI_STACK_SIZE,
			NULL, TASK_DGAS_UI_PRIORITY, &taskHandleDgasUi);
}


