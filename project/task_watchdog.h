/**
 * task_watchdog:
 *
 * TASK responsible for status leds and HW-watchdog reset circuts.
 */

#ifndef __TASK_WATCHDOG_H_
#define __TASK_WATCHDOG_H_

void task_watchdog_signal_error(void);
void task_watchdog(void *p);

#endif /* __TASK_WATCHDOG_H_ */
