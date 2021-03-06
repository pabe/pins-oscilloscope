#ifndef __CONFIG_H__
#define __CONFIG_H__

/* TODO: Clean! */
#define IPC_QUEUE_LEN_WATCHDOG        5
#define IPC_QUEUE_LEN_CONTROLLER      5
#define IPC_QUEUE_LEN_INPUT_TOUCH     5
#define IPC_QUEUE_LEN_DISPLAY         5
#define IPC_QUEUE_LEN_MEASURE         5

#define CONFIG_API_MEASURE_TRANSFER_SIZE 1
#define CONFIG_MEASURE_IRQ_QUEUE_LEN 1
#define CONFIG_SUBSCRIBE_MSG_HOOKS 2
#define CONFIG_SAMPLE_BUFFER_SIZE 320
#define API_MEASURE_DATA_CHUNK_SIZE CONFIG_SAMPLE_BUFFER_SIZE
#define CONFIG_SAMPLE_BUFFERS     2

#define CONFIG_DISPLAY_MULTIMETER_REFRESH_TIME  (300 / portTICK_RATE_MS)
#define CONFIG_IPC_WAIT (500 / portTICK_RATE_MS)
//#define CONFIG_ENABLE_PRINTER 1

#define CFG_TASK_WATCHDOG__POLLING_PERIOD       (300 / portTICK_RATE_MS)
#define CFG_TASK_INPUT_GPIO__POLLING_PERIOD     (10 / portTICK_RATE_MS)
#define CFG_TASK_INPUT_TOUCH__POLLING_PERIOD    (100 / portTICK_RATE_MS)

#endif /* __CONFIG_H__ */
