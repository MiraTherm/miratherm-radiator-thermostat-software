#include "set_value_presenter.h"
#include "set_value_view.h"
#include <stdlib.h>

typedef struct SetValuePresenter {
  SetValueView_t *view;
  SetValue_ViewModelData_t data;
  uint16_t max_index;
  bool is_complete;
} SetValuePresenter_t;

SetValuePresenter_t *SetValuePresenter_Init(SetValueView_t *view,
                                            uint16_t initial_index,
                                            uint16_t max_index) {
  SetValuePresenter_t *presenter =
      (SetValuePresenter_t *)malloc(sizeof(SetValuePresenter_t));
  if (!presenter)
    return NULL;

  presenter->view = view;
  presenter->data.selected_index = initial_index;
  presenter->data.options_str = NULL; /* Managed by view or external setter if
                                         needed, here we just use index */
  presenter->max_index = max_index;
  presenter->is_complete = false;

  return presenter;
}

void SetValuePresenter_Deinit(SetValuePresenter_t *presenter) {
  if (presenter)
    free(presenter);
}

void SetValuePresenter_HandleEvent(SetValuePresenter_t *presenter,
                                   const Input2VPEvent_t *event) {
  if (!presenter || !event)
    return;

  if (event->type == EVT_CTRL_WHEEL_DELTA) {
    int32_t new_index = (int32_t)presenter->data.selected_index + event->delta;
    if (new_index < 0)
      new_index = 0;
    if (new_index > presenter->max_index)
      new_index = presenter->max_index;

    if (presenter->data.selected_index != (uint16_t)new_index) {
      presenter->data.selected_index = (uint16_t)new_index;
      SetValueView_Render(presenter->view, &presenter->data);
    }
  } else if (event->type == EVT_MIDDLE_BTN &&
             event->button_action == BUTTON_ACTION_PRESSED) {
    presenter->is_complete = true;
  }
}

bool SetValuePresenter_IsComplete(SetValuePresenter_t *presenter) {
  return presenter ? presenter->is_complete : false;
}

uint16_t SetValuePresenter_GetSelectedIndex(SetValuePresenter_t *presenter) {
  return presenter ? presenter->data.selected_index : 0;
}

void SetValuePresenter_Reset(SetValuePresenter_t *presenter) {
  if (presenter) {
    presenter->is_complete = false;
  }
}

void SetValuePresenter_SetMaxIndex(SetValuePresenter_t *presenter,
                                   uint16_t max_index) {
  if (presenter) {
    presenter->max_index = max_index;
    if (presenter->data.selected_index > max_index) {
      presenter->data.selected_index = max_index;
      SetValueView_Render(presenter->view, &presenter->data);
    }
  }
}

void SetValuePresenter_SetSelectedIndex(SetValuePresenter_t *presenter,
                                        uint16_t index) {
  if (presenter) {
    if (index > presenter->max_index)
      index = presenter->max_index;
    presenter->data.selected_index = index;
    SetValueView_Render(presenter->view, &presenter->data);
  }
}
