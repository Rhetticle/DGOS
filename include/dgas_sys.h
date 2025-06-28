/*
 * dgas_sys.h
 *
 *  Created on: 2 Jun. 2025
 *      Author: rhett
 */

#ifndef INC_DGAS_SYS_H_
#define INC_DGAS_SYS_H_

#include <dgas_types.h>

#define TASK_DGAS_SYS_PRIORITY (tskIDLE_PRIORITY + 3)
#define TASK_DGAS_SYS_STACK_SIZE (configMINIMAL_STACK_SIZE * 4)

void task_dgas_sys_init(void);

#endif /* INC_DGAS_SYS_H_ */
