#ifndef CORE_INC_PRESENTERS_WAITING_PRESENTER_H
#define CORE_INC_PRESENTERS_WAITING_PRESENTER_H

#include "waiting_view.h"
#include "waiting_viewmodel.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WaitingPresenter WaitingPresenter_t;

WaitingPresenter_t* WaitingPresenter_Init(WaitingView_t *view);
void WaitingPresenter_Deinit(WaitingPresenter_t *presenter);
void WaitingPresenter_Run(WaitingPresenter_t *presenter);
void WaitingPresenter_SetMessage(WaitingPresenter_t *presenter, const char *message);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_WAITING_PRESENTER_H */