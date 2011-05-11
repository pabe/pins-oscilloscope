
#ifndef MSG_H
#define MSG_H

#include "FreeRTOS.h"
#include "queue.h"

enum msg_id
{
	SUBSCRIBE,
	UNSUBSCRIBE,
	SET_SAMPLERATE,
	DATA,
};

struct msg_head
{
	enum msg_id Id;
};

#define MSGID() msg##__LINE__

#define DEFINE_MSG(NAME, ARGS) \
struct msg_ ## NAME \
{ \
	struct msg_head head; \
	ARGS \
} NAME;

union msg_union
{
	DEFINE_MSG(subscribe,
		int channel;
		xQueueHandle Destination;
	)

	DEFINE_MSG(unsubscribe,
		int channel;
		xQueueHandle Destination;
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

#define MSG_MAX_SIZE (sizeof(union msg_union))
#endif /* MSG_H */
