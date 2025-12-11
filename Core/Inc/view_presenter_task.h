#ifndef CORE_INC_VIEW_PRESENTER_TASK_H
#define CORE_INC_VIEW_PRESENTER_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#define VP_TASK_STACK_SIZE (1024U * 4U) // Important: Too small stack will cause hard faults!

void StartViewPresenterTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEW_PRESENTER_TASK_H */
