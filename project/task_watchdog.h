/**
 * task_watchdog:
 *
 * TASK responsible for status leds and HW-watchdog reset circuts.
 * (For now a pure SW that toogles a LED.)
 */

#ifndef __TASK_WATCHDOG__H_
#define __TASK_WATCHDOG__H_

void task_watchdog_signal_error(void);
void task_watchdog(void *p);

#endif /* __TASK_WATCHDOG__H_ */
