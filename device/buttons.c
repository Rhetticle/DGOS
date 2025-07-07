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
static void buttons_init(void) {
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	GPIO_InitTypeDef init = {0};
	EXTI_HandleTypeDef hexti = {0};
	EXTI_ConfigTypeDef conf = {0};

	// configure buttons as GPIO inputs
	init.Mode = MODE_INPUT;
	init.Pin = BTN_NAV_PIN;
	init.Speed = GPIO_SPEED_FREQ_HIGH;
	init.Pull = GPIO_NOPULL;
	// initialise navigation button
	HAL_GPIO_Init(BTN_NAV_PORT, &init);
	init.Pin = BTN_SEL_PIN;
	// initialise select button
	HAL_GPIO_Init(BTN_SEL_PORT, &init);

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	// configure EXTI to interrupt on navigation and select buttons
	conf.GPIOSel = EXTI_GPIOB;
	conf.Line = EXTI_LINE_14;
	conf.Trigger = EXTI_TRIGGER_FALLING;
	conf.Mode = EXTI_MODE_INTERRUPT;

	HAL_EXTI_SetConfigLine(&hexti, &conf);

	conf.Line = EXTI_LINE_15;

	HAL_EXTI_SetConfigLine(&hexti, &conf);

	// Enable EXTI15_10 interrupt in NVIC and set priority
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
 * HAL GPIO external interrupt callback function
 *
 * pin: Source of interrupt
 *
 * Return: None
 * */
void EXTI15_10_IRQHandler(void) {
	NVIC_ClearPendingIRQ(EXTI15_10_IRQn);

	if (((EXTI->PR & EXTI_PR_PR14) == EXTI_PR_PR14) &&
			(HAL_GetTick() > lastNav + BTN_DEBOUNCE_INTERVAL)) {
		EXTI->PR |= EXTI_PR_PR14;
		xEventGroupSetBitsFromISR(eventButtons, EVT_BUTTON_NAV_PRESSED, 0);
		lastNav = HAL_GetTick();
	} else if (((EXTI->PR & EXTI_PR_PR15) == EXTI_PR_PR15) &&
			(HAL_GetTick() > lastSel + BTN_DEBOUNCE_INTERVAL)) {
		EXTI->PR |= EXTI_PR_PR15;
		xEventGroupSetBitsFromISR(eventButtons, EVT_BUTTON_SEL_PRESSED, 0);
		lastSel = HAL_GetTick();
	}

}

/**
 * Initialise buttons task (just creates event group no actual task needed)
 *
 * Return: None
 * */
void task_init_buttons(void) {
	buttons_init();
	eventButtons = xEventGroupCreate();
}
