/*
 * dgas_ui.c
 *
 *  Created on: 6 Jun. 2025
 *      Author: rhett
 */
/*
#include <dgas_types.h>
#include <dgas_ui.h>
#include <display.h>
#include <lvgl.h>
#include <ltdc.h>
#include <buttons.h>
//#include <ui.h>

// Stores handle of UI controller task
static TaskHandle_t taskHandleDgasUi;
// Stores handle of LVGL task
static TaskHandle_t taskHandleLVGL;
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
/*
static void ui_take_semaphore(void) {
	xSemaphoreTake(semaphoreUI, portMAX_DELAY);
}

/**
 * Give UI semaphore
 *
 * Return: None
 * */
/*
static void ui_give_semaphore(void) {
	xSemaphoreGive(semaphoreUI);
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
/*
static void encoder_read(lv_indev_t* indev, lv_indev_data_t* data) {
	EventBits_t uxBits = xEventGroupWaitBits(eventButton, EVT_BUTTON_PRESSED,
											pdTRUE, pdFALSE, 0);

	if (uxBits & EVT_BUTTON_NAV_PRESSED) {
		// navigation button pressed so increment encoder position
		data->enc_diff++;
	}
	if (uxBits & EVT_BUTTON_SEL_PRESSED) {
		data->state = LV_INDEV_STATE_PRESSED;
	} else {
		data->state = LV_INDEV_STATE_RELEASED;
	}
}

/**
 * Frame buffer flush function to be used with LVGL
 *
 * Return: None
 * */
/*
static void ui_flush_frame_buffer(lv_display_t* disp, const lv_area_t* area, uint8_t* map) {
	// since we're using LTDC with DMA2D we simply need to change the LTDC frame
	// buffer pointer to the other frame buffer since double buffering is being used
	if(lv_display_is_double_buffered(disp) && lv_display_flush_is_last(disp)) {
		HAL_LTDC_SetAddress_NoReload(&hltdc, (uint32_t)map, 0);
	    HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
	}
	lv_display_flush_ready(disp);
}

/**
 * LVGL event callback function for gauge screen.
 *
 * evt: Pointer to LVGL event object
 *
 * Return: None
 * */
/*
static void ui_event_callback_gauge(lv_event_t* evt) {
	lv_event_code_t code = lv_event_get_code(evt);

	if (code == LV_EVENT_CLICKED) {
		// navigate or select button was pressed so load menu
		ui_load_screen(&uiMenu);
	}
}

/**
 * LVGL event callback function for menu screen.
 *
 * evt: Pointer to LVGL event object
 *
 * Return: None
 * */
/*
static void ui_event_callback_menu(lv_event_t* evt) {
	lv_event_code_t code = lv_event_get_code(evt);
	lv_obj_t* focused = lv_group_get_focused(lv_indev_get_group(indevEnc));

	if (code == LV_EVENT_CLICKED) {
		if (focused == objects.measure_btn) {
			ui_load_screen(&uiMeas);
		} else if (focused == objects.obd2_debug_btn) {
			ui_load_screen(&uiDebug);
		} else if (focused == objects.diagnose_btn) {
			ui_load_screen(&uiDTC);
		} else if (focused == objects.self_test_btn) {
			ui_load_screen(&uiSelfTest);
		} else if (focused == objects.settings_btn) {
			ui_load_screen(&uiSettings);
		} else if (focused == objects.about_btn) {
			ui_load_screen(&uiAbout);
		} else if (focused == objects.menu_exit_btn) {
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
/*
static void ui_event_callback_meas(lv_event_t* evt) {
	lv_event_code_t code = lv_event_get_code(evt);

	if (code == LV_EVENT_CLICKED) {

	}
}

/**
 * LVGL event callback function for debug screen.
 *
 * evt: Pointer to LVGL event object
 *
 * Return: None
 * */
/*
static void ui_event_callback_debug(lv_event_t* evt) {

}

/**
 * LVGL event callback function for DTC screen.
 *
 * evt: Pointer to LVGL event object
 *
 * Return: None
 * */
/*
static void ui_event_callback_dtc(lv_event_t* evt) {

}

/**
 * LVGL event callback function for self test screen.
 *
 * evt: Pointer to LVGL event object
 *
 * Return: None
 * */
/*
static void ui_event_callback_self_test(lv_event_t* evt) {

}

/**
 * LVGL event callback function for settings screen.
 *
 * evt: Pointer to LVGL event object
 *
 * Return: None
 * */
/*
static void ui_event_callback_settings(lv_event_t* evt) {

}

/**
 * LVGL event callback function for about screen.
 *
 * evt: Pointer to LVGL event object
 *
 * Return: None
 * */
/*
static void ui_event_callback_about(lv_event_t* evt) {

}

/**
 * Get task handle of DGAS UI task
 *
 * Return: Task handle of DGAS UI task
 * */
/*
TaskHandle_t task_dgas_ui_get_handle(void) {
	return taskHandleDgasUi;
}

/**
 * Get task handle of LVGL task
 *
 * Return: Task handle of LVGL task
 * */
/*
TaskHandle_t task_lvgl_get_handle(void) {
	return taskHandleLVGL;
}

/**
 * Initialise external DRAM for UI frame buffers
 *
 * Return: None
 * */
/*
void dram_init(void) {
	taskENTER_CRITICAL();
	FMC_SDRAM_CommandTypeDef Command;
	 Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
	 Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	 Command.AutoRefreshNumber      = 1;
	 Command.ModeRegisterDefinition = 0;
	 HAL_SDRAM_SendCommand(&hsdram1, &Command, 0xfff);
	 HAL_Delay(1);
	 Command.CommandMode            = FMC_SDRAM_CMD_PALL;
	 HAL_SDRAM_SendCommand(&hsdram1, &Command, 0xfff);
	 Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
	 Command.AutoRefreshNumber      = 2;
	 HAL_SDRAM_SendCommand(&hsdram1, &Command, 0xfff);
	 Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
	 Command.ModeRegisterDefinition =  (uint32_t)0 | 0<<3 | 2<<4 | 0<<7 | 1<<9;
	 HAL_SDRAM_SendCommand(&hsdram1, &Command, 0xfff);
	 HAL_SDRAM_ProgramRefreshRate(&hsdram1, 1105);
	 taskEXIT_CRITICAL();
}*/

/**
 * Load a new screen onto display. Will load screen LVGL object and
 * set indev group to group for that specific screen
 *
 * Return: None
 * */
/*
void ui_load_screen(UI* ui) {
	ui_take_semaphore();
	lv_screen_load(ui->screen);
	ui_give_semaphore();
	lv_indev_set_group(indevEnc, ui->group);
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
/*
void ui_init_struct(UI* init, lv_obj_t* scrn, lv_obj_t** eventable,
					uint32_t size, evtCallback cb, UICallbackOpt opt) {
	init->screen = scrn;
	init->group = lv_group_create();

	if (opt == UI_CALLBACK_USE_FOR_ALL) {
		for (int i = 0; i < size; i++) {
			lv_obj_add_event_cb(eventable[i], cb, LV_EVENT_ALL, NULL);
			lv_group_add_obj(init->group, eventable[i]);
		}
	} else if (opt == UI_CALLBACK_USE_FOR_SCREEN) {
		lv_obj_add_event_cb(scrn, cb, LV_EVENT_ALL, NULL);
		lv_group_add_obj(init->group, scrn);
	}
}

/**
 * Intialise ui structs
 *
 * Return: None
 * */
/*
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
	ui_init_struct(&uiGauge, objects.gauge_main_ui, NULL, 0,
					&ui_event_callback_gauge, UI_CALLBACK_USE_FOR_SCREEN);

	ui_init_struct(&uiMenu, objects.menu, menuEventable, sizeof(menuEventable),
					&ui_event_callback_menu, UI_CALLBACK_USE_FOR_ALL);

	ui_init_struct(&uiMeas, objects.measure, measEventable, sizeof(menuEventable),
					&ui_event_callback_meas, UI_CALLBACK_USE_FOR_ALL);

	ui_init_struct(&uiDebug, objects.obd2_debug, debugEventable, sizeof(debugEventable),
					&ui_event_callback_debug, UI_CALLBACK_USE_FOR_ALL);

	ui_init_struct(&uiDTC, objects.diagnose, dtcEventable, sizeof(dtcEventable),
					&ui_event_callback_dtc, UI_CALLBACK_USE_FOR_ALL);

	ui_init_struct(&uiSelfTest, objects.self_test, selfTestEventable, sizeof(selfTestEventable),
					&ui_event_callback_self_test, UI_CALLBACK_USE_FOR_ALL);

	ui_init_struct(&uiSettings, objects.settings, settingsEventable, sizeof(settingsEventable),
					&ui_event_callback_settings, UI_CALLBACK_USE_FOR_ALL);

	ui_init_struct(&uiAbout, objects.about, aboutEventable, sizeof(aboutEventable),
					&ui_event_callback_about, UI_CALLBACK_USE_FOR_ALL);
}

/**
 * Initialise LVGL related functionality for UI
 *
 * Return: None
 * */
/*
void ui_init_lvgl(void) {
	lv_init();
	// set LVGL tick callback
	lv_tick_set_cb(HAL_GetTick);
	// create display object and set start address of both frame buffers
	display = lv_display_create(LCD_RESOLUTION_X, LCD_RESOLUTION_Y);
#ifdef DGAS_CONFIG_USE_DOUBLE_BUFFERING
	lv_display_set_buffers(display, (void*) UI_FRAME_BUFF_ONE_ADDR, (void*) UI_FRAME_BUFF_TWO_ADDR,
							UI_FRAME_BUFF_SIZE, LV_DISP_RENDER_MODE_DIRECT);
	// set the flush callback function for LVGL
#else
	lv_display_set_buffers(display, (void*) UI_FRAME_BUFF_ONE_ADDR, (void*) NULL,
							UI_FRAME_BUFF_SIZE, LV_DISP_RENDER_MODE_DIRECT);
#endif /* DGAS_CONFIG_USE_DOUBLE_BUFFERING *//*
	lv_display_set_flush_cb(display, ui_flush_frame_buffer);
	// create input device and set type and read callback function
	indevEnc = lv_indev_create();
	lv_indev_set_type(indevEnc, LV_INDEV_TYPE_ENCODER);
	lv_indev_set_read_cb(indevEnc, encoder_read);
}

/**
 * Thread function for DGAS UI task
 *
 * Return: None
 * */
/*
void task_dgas_ui(void) {
	taskENTER_CRITICAL();
	semaphoreUI = xSemaphoreCreateBinary();
	display_init();
	// dram_init();
	ui_init_lvgl();
	// EEZ init
	ui_init();
	// DGAS UI init
	ui_init_all_uis();
	ui_load_screen(&uiGauge);
	taskEXIT_CRITICAL();

	for(;;) {
		vTaskDelay(30);
	}
}

/**
 * LVGL tick task function.
 *
 * Return: None
 * */
/*
void task_lvgl_tick(void) {
	lv_init();

	for(;;) {
		ui_take_semaphore();
		// got semaphore, update UI
		lv_task_handler();
		ui_give_semaphore();
		vTaskDelay(10);
	}
}

/**
 * Initialise UI controller task
 *
 * Return: None
 * */
/*
void task_dgas_ui_init(void) {
	// create UI controller task
	xTaskCreate((void*) &task_dgas_ui, "TaskDgasUi", TASK_DGAS_UI_STACK_SIZE,
			NULL, TASK_DGAS_UI_PRIORITY, &taskHandleDgasUi);
	// create LVGL tick task
	xTaskCreate((void*) &task_lvgl_tick, "TaskLVGL", TASK_DGAS_LVGL_STACK_SIZE,
			NULL, TASK_DGAS_LVGL_PRIORITY, &taskHandleLVGL);
}*/
