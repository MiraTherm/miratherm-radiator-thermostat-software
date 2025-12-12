#ifndef CORE_INC_VIEW_PRESENTER_TASK_H
#define CORE_INC_VIEW_PRESENTER_TASK_H

#include "input_task.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VP_TASK_STACK_SIZE (1024U * 4U) // Important: Too small stack will cause hard faults!

typedef struct
{
	osMessageQueueId_t input2vp_event_queue;
} ViewPresenterTaskArgsTypeDef;

void StartViewPresenterTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEW_PRESENTER_TASK_H */
