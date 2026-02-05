#ifndef CORE_INC_PRESENTERS_WAITING_PRESENTER_H
#define CORE_INC_PRESENTERS_WAITING_PRESENTER_H

#include "waiting_view.h"
#include "input_task.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WaitingPresenter WaitingPresenter_t;

WaitingPresenter_t* WaitingPresenter_Init(WaitingView_t *view);
void WaitingPresenter_Deinit(WaitingPresenter_t *presenter);
void WaitingPresenter_Reset(WaitingPresenter_t *presenter);
void WaitingPresenter_Run(WaitingPresenter_t *presenter);
void WaitingPresenter_SetMessage(WaitingPresenter_t *presenter, const char *message);
void WaitingPresenter_HandleEvent(WaitingPresenter_t *presenter, const Input2VPEvent_t *event);
bool WaitingPresenter_IsComplete(WaitingPresenter_t *presenter);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_WAITING_PRESENTER_H */