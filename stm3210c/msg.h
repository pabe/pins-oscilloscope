
#ifndef MSG_H
#define MSG_H

#include "FreeRTOS.h"
#include "queue.h"

enum msg_id
{
	SUBSCRIBE,
	UNSUBSCRIBE,
	DATA,
	SET_SAMPLERATE,
	GET_SAMPLERATE
};

union foo
{
	struct msgSubscribe
	{
		int Channel;
		xQueueHandle Destination;
	} msg0;

	struct msgUnsubscribe
	{
		int Channel;
		xQueueHandle Destination;
	} msg1;
};


#endif /* MSG_H */
