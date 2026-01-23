#include "set_bool_presenter.h"
#include "set_bool_view.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct SetBoolPresenter {
  SetBoolView_t *view;
  SetBool_ViewModelData_t data;
  bool is_complete;
} SetBoolPresenter_t;

SetBoolPresenter_t *SetBoolPresenter_Init(SetBoolView_t *view) {
  SetBoolPresenter_t *presenter =
      (SetBoolPresenter_t *)malloc(sizeof(SetBoolPresenter_t));
  if (!presenter)
    return NULL;

  presenter->view = view;
  presenter->is_complete = false;
  presenter->data.value = false;

  return presenter;
}

void SetBoolPresenter_Deinit(SetBoolPresenter_t *presenter) {
  if (presenter)
    free(presenter);
}

void SetBoolPresenter_HandleEvent(SetBoolPresenter_t *presenter,
                                  const Input2VPEvent_t *event) {
  if (!presenter || !event)
    return;

  bool state_changed = false;

  if (event->type == EVT_CTRL_WHEEL_DELTA) {
    if (event->delta < 0) {
      presenter->data.value = false;
    } else if (event->delta > 0) {
      presenter->data.value = true;
    }
    state_changed = true;
  } else if (event->type == EVT_MIDDLE_BTN &&
             event->button_action == BUTTON_ACTION_PRESSED) {
    presenter->is_complete = true;
    state_changed = true;
  }

  if (state_changed && presenter->view) {
    SetBoolView_Render(presenter->view, &presenter->data);
  }
}

void SetBoolPresenter_Reset(SetBoolPresenter_t *presenter) {
  if (presenter) {
    presenter->is_complete = false;
  }
}

bool SetBoolPresenter_IsComplete(SetBoolPresenter_t *presenter) {
  if (!presenter)
    return false;
  return presenter->is_complete;
}

const SetBool_ViewModelData_t *
SetBoolPresenter_GetData(SetBoolPresenter_t *presenter) {
  if (!presenter)
    return NULL;
  return &presenter->data;
}

void SetBoolPresenter_Run(SetBoolPresenter_t *presenter,
                          uint32_t current_tick) {
  (void)current_tick;
  if (presenter && presenter->view) {
    SetBoolView_Render(presenter->view, &presenter->data);
  }
}
