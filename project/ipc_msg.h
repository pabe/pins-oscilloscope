
#ifndef MSG_H
#define MSG_H

#include "FreeRTOS.h"
#include "queue.h"

#include "ipc.h"

enum ipc_msg_id
{
	SUBSCRIBE,
	UNSUBSCRIBE,
	SET_SAMPLERATE,
	DATA,
};

struct ipc_msg_head
{
	enum ipc_msg_id Id;
//  enum ipc_modules src;
//  unsigned reply:1;
//  unsigned datagram:1;
};

#define DEFINE_MSG(NAME, ARGS) \
struct ipc_msg_ ## NAME \
{ \
	ARGS \
} NAME;

struct ipc_msg
{
  struct ipc_msg_head head;
  union {
	DEFINE_MSG(subscribe,
		int channel;
		enum ipc_modules Destination;
	)

	DEFINE_MSG(unsubscribe,
		int channel;
		enum ipc_modules Destination;
	)

	DEFINE_MSG(set_samplerate,
		int channel;
		int rate;
	)

	DEFINE_MSG(data,
		int channel;
		int value;
	)
  } payload;
};

//#define MSG_MAX_SIZE (sizeof(struct ipc_msg))
#endif /* MSG_H */
