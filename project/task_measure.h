/*
 * Direct shmem-interface for the ADCs.
 */

#ifndef __MEASURE_H__
#define __MEASURE_H__

portBASE_TYPE task_measure_init(void);
void task_measure_cmd(void* params);
void task_measure(void* params);



/* TODO: CLEAN! */
#define NUMBER_OF_CHANNELS 2  //Number of probes on the oscilloscope
#define ADC_MAX ((1<<12)-1)

#endif /* __MEASURE_H__ */
