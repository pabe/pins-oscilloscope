
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

struct msg_head
{
	enum ipc_msg_id Id;
};

#define DEFINE_MSG(NAME, ARGS) \
struct ipc_msg_ ## NAME \
{ \
	enum ipc_msg_id head_id; \
	ARGS \
} NAME;

union ipc_msg
{
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
};

#define MSG_MAX_SIZE (sizeof(union ipc_msg))
#endif /* MSG_H */
