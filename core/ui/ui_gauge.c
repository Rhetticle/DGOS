/*
 * dgas_ui_gauge.c
 *
 *  Created on: 22 Nov. 2025
 *      Author: rhett
 */

/**
 * Gauge UI logic. Tasks which wish to modify gauge UI make request to UI
 * controller task through ui_gauge_make_request().
 * */

#include <ui_gauge.h>
#include <dgas_ui.h>
#include <stdio.h>
#include <string.h>

// Most recent gauge update
static UIGaugeUpdate lastUpdate;
// Current max value of parameter
static float gMax;

/**
 * LVGL animation callback function to animate the gauge
 * arc on startup
 *
 * var: Arc object
 * val: Value to set arc to
 *
 * Return: None
 * */
static void ui_gauge_animate_cb(void* var, int32_t val) {
	lv_arc_set_value((lv_obj_t*) var, val);
}

/**
 * Perform startup gauge animation
 *
 * Return: None
 * */
static void ui_gauge_animate(void) {
    lv_anim_t a;
    lv_anim_init(&a);

    lv_anim_set_var(&a, objects.gauge_arc);
    lv_anim_set_exec_cb(&a, ui_gauge_animate_cb);

    lv_anim_set_time(&a, UI_GAUGE_ANIM_TIME);
    lv_anim_set_values(&a, 0, UI_GAUGE_ANIM_ARC_END_VALUE);

    lv_anim_set_playback_time(&a, UI_GAUGE_ANIM_TIME);

    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);

    lv_anim_start(&a);
}

/**
 * Adjust scale labels on gauge arc
 *
 * min: Min value of new scale
 * max: Max value of new scale
 * scaleLabels: Scale label objects
 *
 * Return: None
 * */
static void ui_gauge_adjust_scale_labels(int32_t min, int32_t max, lv_obj_t** scaleLabels) {
	uint16_t step = (max - min) / (UI_GAUGE_ARC_TICK_COUNT - 1);

	for (int i = 0; i < UI_GAUGE_ARC_TICK_COUNT; i++) {
		char buff[UI_GAUGE_TICK_BUFF_LEN];
		sprintf(buff, "%ld", min + (i * step));
		lv_label_set_text(scaleLabels[i], buff);
	}
}

/**
 * Load new parameter onto gauge UI
 *
 * gLoad: UI gauge load struct
 *
 * Return: None
 * */
static void ui_gauge_load(UIGaugeLoad* gLoad) {
	gMax = 0;
	lv_obj_t* scaleLabels[] = {objects.gauge_tick_0, objects.gauge_tick_1, objects.gauge_tick_2,
							   objects.gauge_tick_3, objects.gauge_tick_4, objects.gauge_tick_5,
							   objects.gauge_tick_6};

	lv_arc_set_range(objects.gauge_arc, gLoad->lMin, gLoad->lMax);
	lv_scale_set_range(objects.gauge_scale, gLoad->lMin, gLoad->lMax);
	lv_label_set_text(objects.parameter_label, gLoad->lName);
	lv_label_set_text(objects.param_units_label, gLoad->lUnits);

	// update colour of colour-coded objects
	lv_obj_set_style_arc_color(objects.gauge_arc, lv_color_hex(gLoad->lColour),
								LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(objects.param_val, lv_color_hex(gLoad->lColour),
								LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(objects.parameter_label, lv_color_hex(gLoad->lColour),
								LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(objects.param_max_label, lv_color_hex(gLoad->lColour),
								LV_PART_MAIN | LV_STATE_DEFAULT);
	ui_gauge_adjust_scale_labels(gLoad->lMin, gLoad->lMax, scaleLabels);
}

/**
 * Update gauge UI
 *
 * gUpdate: UI gauge update struct
 *
 * Return: None
 * */
static void ui_gauge_update(UIGaugeUpdate* gUpdate) {
	if (gUpdate->gVal != lastUpdate.gVal) {
		char buff[UI_GAUGE_PARAM_VAL_BUFF_LEN];
		sprintf(buff, "%i", gUpdate->gVal);
		// parameter value has changed so update it
		lv_arc_set_value(objects.gauge_arc, gUpdate->gVal);
		lv_label_set_text(objects.param_val, buff);
		if (gUpdate->gVal > lastUpdate.gVal) {
			// param value has exceeded current maximum so update label
			lv_label_set_text(objects.param_max_label, buff);
			gMax = gUpdate->gVal;
		}
	}
	if (strcmp(gUpdate->gObd, lastUpdate.gObd)) {
		// status string is different so update it
		lv_label_set_text(objects.obd_status_label, gUpdate->gObd);
		// update stat
		strcpy(lastUpdate.gObd, gUpdate->gObd);
	}
	if (gUpdate->gVbat != lastUpdate.gVbat) {
		char buff[UI_GAUGE_VBAT_BUFF_LEN];
		sprintf(buff, "%.1fV", gUpdate->gVbat);
		lv_label_set_text(objects.vbat_label, buff);
	}
}

/**
 * Handle a UI request. This function is called by the main
 * UI controller task (dgas_ui.c)
 *
 * req: UI request to execute
 *
 * Return: None
 * */
static void ui_gauge_handle_request(UIRequest* req) {
	switch(req->uCmd) {
	case UI_CMD_GAUGE_LOAD: {
		UIGaugeLoad gLoad = {0};
		memcpy(&gLoad, req->uData, sizeof(UIGaugeLoad));
		ui_gauge_load(&gLoad);
		break;
	}
	case UI_CMD_GAUGE_UPDATE: {
		UIGaugeUpdate gUpdate = {0};
		memcpy(&gUpdate, req->uData, sizeof(UIGaugeUpdate));
		ui_gauge_update(&gUpdate);
		break;
	}
	case UI_CMD_GAUGE_ANIMATE:
		ui_gauge_animate();
		break;
	default:
		// invalid command
		break;
	}
}

/**
 * Make a gauge UI request
 *
 * cmd: Gauge UI command to execute
 * arg: Option argument
 *
 * Return: None
 * */
void ui_gauge_make_request(UICmd cmd, void* arg) {
	UIRequest req = {0};

	req.uSys = UI_SUBSYS_GAUGE;
	req.uCmd = cmd;

	if (cmd == UI_CMD_GAUGE_LOAD) {
		UIGaugeLoad* gLoad = (UIGaugeLoad*) arg;
		memcpy(req.uData, gLoad, sizeof(UIGaugeLoad));
	} else if (cmd == UI_CMD_GAUGE_UPDATE) {
		UIGaugeUpdate* gUpdate = (UIGaugeUpdate*) arg;
		memcpy(req.uData, gUpdate, sizeof(UIGaugeUpdate));
	}
	ui_make_request(&req);
}

/**
 * Initialise gauge UI. Registers request callback with dgas_ui and performs
 * startup animation.
 * */
void ui_gauge_init(void) {
	ui_request_register_callback(UI_SUBSYS_GAUGE, &ui_gauge_handle_request);
	ui_gauge_make_request(UI_CMD_GAUGE_ANIMATE, NULL);
	vTaskDelay(3 * UI_GAUGE_ANIM_TIME);
	lastUpdate.gVal = -1;
}
