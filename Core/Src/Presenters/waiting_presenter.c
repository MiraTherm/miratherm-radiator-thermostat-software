#include "waiting_presenter.h"
#include <stdlib.h>

typedef struct WaitingPresenter {
  WaitingView_t *view;
  WaitingViewData_t data;
  bool is_complete;
} WaitingPresenter_t;

WaitingPresenter_t *WaitingPresenter_Init(WaitingView_t *view) {
  WaitingPresenter_t *presenter =
      (WaitingPresenter_t *)malloc(sizeof(WaitingPresenter_t));
  if (!presenter)
    return NULL;
  presenter->view = view;
  presenter->is_complete = false;
  return presenter;
}

void WaitingPresenter_Deinit(WaitingPresenter_t *presenter) {
  if (presenter)
    free(presenter);
}

void WaitingPresenter_Reset(WaitingPresenter_t *presenter) {
  if (presenter) {
    presenter->is_complete = false;
  }
}

void WaitingPresenter_Run(WaitingPresenter_t *presenter) {
  if (presenter && presenter->view) {
    WaitingView_Render(presenter->view, &presenter->data);
  }
}

void WaitingPresenter_SetMessage(WaitingPresenter_t *presenter,
                                 const char *message) {
  if (presenter && presenter->view) {
    WaitingView_SetMessage(presenter->view, message);
  }
}

void WaitingPresenter_HandleEvent(WaitingPresenter_t *presenter,
                                  const Input2VPEvent_t *event) {
  if (!presenter || !event)
    return;

  if (event->type == EVT_MIDDLE_BTN &&
      event->button_action == BUTTON_ACTION_PRESSED) {
    presenter->is_complete = true;
  }
}

bool WaitingPresenter_IsComplete(WaitingPresenter_t *presenter) {
  if (!presenter)
    return false;
  return presenter->is_complete;
}

