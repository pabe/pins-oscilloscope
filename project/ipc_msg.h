
#ifndef MSG_H
#define MSG_H

#include "FreeRTOS.h"
#include "queue.h"

#include "ipc.h"

/*
 * id that start with _ do now have payload body
 */
enum ipc_msg_id
{
  ACKNOWLEDGE,
  SUBSCRIBE,
  UNSUBSCRIBE,
  SET_SAMPLERATE,
  DATA,
  WATCHDOG_CMD,
  ID_SET,
  ID_GET,
  ID_SUBSCRIBE,
  _ID_BAD_REQUEST
};

struct ipc_msg_head
{
  enum ipc_msg_id Id;
  struct ipc_addr src;
  unsigned reply:1;
};

#define DEFINE_MSG(NAME, ARGS) \
struct ipc_msg_ ## NAME \
{ \
  ARGS \
} NAME

struct ipc_fullmsg
{
  struct ipc_msg_head head;
  union ipc_msg
  {
  
    DEFINE_MSG(set,
        uint32_t cmd;
        uint8_t tag;
        );

    DEFINE_MSG(get,
        uint32_t cmd;
        uint8_t tag;
        );

    DEFINE_MSG(subscribe2,
        int what;
        ipc_modules_t subscriber;
    );

    DEFINE_MSG(watchdog_cmd,
        uint8_t cmd;
        );

    DEFINE_MSG(acknowledge,
        char dummy;
        );

    DEFINE_MSG(subscribe,
        int channel;
        enum ipc_modules Destination;
        );

    DEFINE_MSG(unsubscribe,
        int channel;
        enum ipc_modules Destination;
        );

    DEFINE_MSG(set_samplerate,
        int channel;
        int rate;
        );

    DEFINE_MSG(data,
        int channel;
        int value;
        );
  } payload;
};

//#define MSG_MAX_SIZE (sizeof(struct ipc_msg))
#endif /* MSG_H */
