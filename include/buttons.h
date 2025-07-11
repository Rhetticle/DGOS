/*
 * buttons.h
 *
 *  Created on: 18 Jun. 2025
 *      Author: rhett
 */

#ifndef DGOS_INCLUDE_BUTTONS_H_
#define DGOS_INCLUDE_BUTTONS_H_

#include <dgas_types.h>

extern EventGroupHandle_t eventButtons;

#define BTN_NAV_PORT 			GPIOB
#define BTN_NAV_PIN 			GPIO_PIN_15

#define BTN_SEL_PORT 			GPIOB
#define BTN_SEL_PIN 			GPIO_PIN_14

#define EVT_BUTTON_NAV_PRESSED 	(1 << 0)
#define EVT_BUTTON_SEL_PRESSED 	(1 << 1)
#define EVT_BUTTON_PRESSED 		(EVT_BUTTON_NAV_PRESSED | EVT_BUTTON_SEL_PRESSED)

// Function prototypes
void task_init_buttons(void);

#endif /* DGOS_INCLUDE_BUTTONS_H_ */
