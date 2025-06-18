/*
 * buttons.c
 *
 *  Created on: 18 Jun. 2025
 *      Author: rhett
 */

#include <buttons.h>

// variables to store tick for debouncing
static uint32_t lastNav;
static uint32_t lastSel;

EventGroupHandle_t eventButtons;

/**
 * Initialise GPIO pins for navigation and selection buttons
 *
 * Return: None
 * */
void buttons_init(void) {
	GPIO_InitTypeDef init = {0};
	init.Mode = MODE_OUTPUT;
	init.Pin = BTN_NAV_PIN;
	init.Speed = GPIO_SPEED_FREQ_HIGH;
	init.Pull = GPIO_PULLUP;
	// initialise navigation button
	HAL_GPIO_Init(BTN_NAV_PORT, &init);
	init.Pin = BTN_SEL_PIN;
	// initialise select button
	HAL_GPIO_Init(BTN_SEL_PORT, &init);
}

/**
 * HAL GPIO external interrupt callback function
 *
 * pin: Source of interrupt
 *
 * Return: None
 * */
void HAL_GPIO_EXTI_Callback(uint16_t pin) {
	if ((pin == BTN_NAV_PIN) && (HAL_GetTick() > lastNav + 200)) {
		xEventGroupSetBits(eventButtons, EVT_NAV_PRESSED);
		lastNav = HAL_GetTick();
	} else if ((pin == BTN_SEL_PIN) && (HAL_GetTick() > lastSel + 200)) {
		xEventGroupSetBits(eventButtons, EVT_SEL_PRESSED);
		lastSel = HAL_GetTick();
	}
}

/**
 * Initialise buttons task (just creates event group no actual task needed)
 *
 * Return: None
 * */
void task_buttons_init(void) {
	eventButtons = xEventGroupCreate();
}
