/**
 * task_watchdog:
 *
 * TASK responsible for status leds and HW-watchdog reset circuts.
 * (For now a pure SW that toogles a LED.)
 */

#ifndef __TASK_WATCHDOG_H_
#define __TASK_WATCHDOG_H_

#define TASK_WATCHDOG_CMD_AUX_LED_LIT    0
#define TASK_WATCHDOG_CMD_AUX_LED_QUENCH 1

void task_watchdog_signal_error(void);
void task_watchdog(void *p);

#endif /* __TASK_WATCHDOG_H_ */
