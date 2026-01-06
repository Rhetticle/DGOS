/*
 * dgas_sys.h
 *
 *  Created on: 2 Jun. 2025
 *      Author: rhett
 */

#ifndef INC_DGAS_SYS_H_
#define INC_DGAS_SYS_H_

#include <dgas_types.h>


#define TASK_DGAS_SYS_PRIORITY (tskIDLE_PRIORITY + 6)
#define TASK_DGAS_SYS_STACK_SIZE (configMINIMAL_STACK_SIZE * 6)

#define DGAS_SYS_BOOT_TIMEOUT_UI	10000
#define DGAS_SYS_BOOT_TIMEOUT_OBD	10000
#define DGAS_SYS_BOOT_TIMEOUT_DEV	1000

#define DGAS_SYS_BOOT_OK			0
#define DGAS_SYS_BOOT_ERROR_UI		1
#define DGAS_SYS_BOOT_ERROR_OBD		2
#define DGAS_SYS_BOOT_ERROR_DEV		3

void task_dgas_sys_init(void);

#endif /* INC_DGAS_SYS_H_ */
