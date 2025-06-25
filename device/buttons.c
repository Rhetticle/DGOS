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
// event group for button presses
EventGroupHandle_t eventButtons;

/**
 * Initialise GPIO pins for navigation and selection buttons
 *
 * Return: None
 * */
void buttons_init(void) {
	GPIO_InitTypeDef init = {0};
	EXTI_HandleTypeDef hexti;
	EXTI_ConfigTypeDef conf = {0};

	// configure buttons as GPIO inputs
	init.Mode = MODE_INPUT;
	init.Pin = BTN_NAV_PIN;
	init.Speed = GPIO_SPEED_FREQ_HIGH;
	init.Pull = GPIO_PULLUP;
	// initialise navigation button
	HAL_GPIO_Init(BTN_NAV_PORT, &init);
	init.Pin = BTN_SEL_PIN;
	// initialise select button
	HAL_GPIO_Init(BTN_SEL_PORT, &init);

	// configure EXTI to interrupt on navigation and select buttons
	conf.GPIOSel = BTN_NAV_PIN;
	conf.Line = EXTI_LINE_14;
	conf.Trigger = EXTI_TRIGGER_FALLING;
	conf.Mode = EXTI_MODE_INTERRUPT;

	HAL_EXTI_SetConfigLine(&hexti, &conf);

	conf.GPIOSel = BTN_SEL_PIN;
	conf.Line = EXTI_LINE_15;

	HAL_EXTI_SetConfigLine(&hexti, &conf);

	// Enable EXTI15_10 interrupt in NVIC and set priority
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 12, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
 * HAL GPIO external interrupt callback function
 *
 * pin: Source of interrupt
 *
 * Return: None
 * */
void HAL_GPIO_EXTI_Callback(uint16_t pin) {
	if ((pin == BTN_NAV_PIN) && (HAL_GetTick() > lastNav + BTN_DEBOUNCE_INTERVAL)) {
		xEventGroupSetBits(eventButtons, EVT_BUTTON_NAV_PRESSED);
		lastNav = HAL_GetTick();
	} else if ((pin == BTN_SEL_PIN) && (HAL_GetTick() > lastSel + BTN_DEBOUNCE_INTERVAL)) {
		xEventGroupSetBits(eventButtons, EVT_BUTTON_SEL_PRESSED);
		lastSel = HAL_GetTick();
	}
}

/**
 * Initialise buttons task (just creates event group no actual task needed)
 *
 * Return: None
 * */
void task_buttons_init(void) {
	buttons_init();
	eventButtons = xEventGroupCreate();
}
