#include "waiting_presenter.h"
#include <stdlib.h>

typedef struct WaitingPresenter {
  WaitingView_t *view;
  Waiting_ViewModelData_t data;
} WaitingPresenter_t;

WaitingPresenter_t *WaitingPresenter_Init(WaitingView_t *view) {
  WaitingPresenter_t *presenter =
      (WaitingPresenter_t *)malloc(sizeof(WaitingPresenter_t));
  if (!presenter)
    return NULL;
  presenter->view = view;
  return presenter;
}

void WaitingPresenter_Deinit(WaitingPresenter_t *presenter) {
  if (presenter)
    free(presenter);
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
