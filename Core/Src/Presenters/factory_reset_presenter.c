#include "factory_reset_presenter.h"
#include "loading_presenter.h"
#include "loading_view.h"
#include "lvgl_port_display.h"
#include "set_bool_presenter.h"
#include "set_bool_view.h"
#include <stdio.h>
#include <stdlib.h>

typedef enum { FR_STATE_CONFIRM, FR_STATE_PROGRESS } FactoryResetState_t;

struct FactoryResetPresenter {
  FactoryResetState_t state;

  /* Sub-components */
  SetBoolView_t *confirm_view;
  SetBoolPresenter_t *confirm_presenter;

  LoadingView_t *progress_view;
  LoadingPresenter_t *progress_presenter;

  /* System interface */
  osMessageQueueId_t vp2system_queue;

  bool is_complete;
};

FactoryResetPresenter_t *
FactoryResetPresenter_Init(osMessageQueueId_t vp2system_queue) {
  FactoryResetPresenter_t *presenter =
      (FactoryResetPresenter_t *)malloc(sizeof(FactoryResetPresenter_t));
  if (!presenter)
    return NULL;

  presenter->vp2system_queue = vp2system_queue;
  presenter->state = FR_STATE_CONFIRM;
  presenter->is_complete = false;

  presenter->confirm_view = NULL;
  presenter->confirm_presenter = NULL;
  presenter->progress_view = NULL;
  presenter->progress_presenter = NULL;

  /* Initialize Confirmation View */
  presenter->confirm_view =
      SetBoolView_Init("Factory reset?", "Yes", "No", true);
  if (presenter->confirm_view) {
    SetBoolView_Show(presenter->confirm_view);
    presenter->confirm_presenter =
        SetBoolPresenter_Init(presenter->confirm_view);
  }

  return presenter;
}

void FactoryResetPresenter_Deinit(FactoryResetPresenter_t *presenter) {
  if (!presenter)
    return;

  if (presenter->confirm_presenter) {
    SetBoolPresenter_Deinit(presenter->confirm_presenter);
  }
  if (presenter->confirm_view) {
    SetBoolView_Deinit(presenter->confirm_view);
  }

  if (presenter->progress_presenter) {
    LoadingPresenter_Deinit(presenter->progress_presenter);
  }
  if (presenter->progress_view) {
    LoadingView_Deinit(presenter->progress_view);
  }

  free(presenter);
}

void FactoryResetPresenter_HandleEvent(FactoryResetPresenter_t *presenter,
                                       const Input2VPEvent_t *event) {
  if (!presenter || !event)
    return;

  if (presenter->state == FR_STATE_CONFIRM) {
    if (presenter->confirm_presenter) {
      SetBoolPresenter_HandleEvent(presenter->confirm_presenter, event);

      if (SetBoolPresenter_IsComplete(presenter->confirm_presenter)) {
        const SetBool_ViewModelData_t *data =
            SetBoolPresenter_GetData(presenter->confirm_presenter);
        if (data && data->value) {
          /* Yes selected -> Transition to Progress */
          presenter->state = FR_STATE_PROGRESS;

          /* Init Progress */
          presenter->progress_view =
              LoadingView_Init("Factory Reset", LV_ALIGN_LEFT_MID, 10);
          if (presenter->progress_view) {
            presenter->progress_presenter =
                LoadingPresenter_Init(presenter->progress_view);
          }

          /* Cleanup Confirm */
          SetBoolPresenter_Deinit(presenter->confirm_presenter);
          presenter->confirm_presenter = NULL;
          SetBoolView_Deinit(presenter->confirm_view);
          presenter->confirm_view = NULL;

          /* Send Event */
          if (presenter->vp2system_queue) {
            VP2SystemEventTypeDef evt = EVT_FACTORY_RST_REQ;
            osMessageQueuePut(presenter->vp2system_queue, &evt, 0, 0);
          }
        } else {
          /* No selected -> Cancel */
          presenter->is_complete = true;
        }
      }
    }
  } else if (presenter->state == FR_STATE_PROGRESS) {
    /* No input handling in progress state */
  }
}

void FactoryResetPresenter_Run(FactoryResetPresenter_t *presenter,
                               uint32_t current_tick) {
  if (!presenter)
    return;

  if (presenter->state == FR_STATE_CONFIRM) {
    if (presenter->confirm_presenter) {
      SetBoolPresenter_Run(presenter->confirm_presenter, current_tick);
    }
  } else if (presenter->state == FR_STATE_PROGRESS) {
    if (presenter->progress_presenter) {
      LoadingPresenter_Run(presenter->progress_presenter, current_tick);
    }
  }
}

bool FactoryResetPresenter_IsComplete(FactoryResetPresenter_t *presenter) {
  return presenter ? presenter->is_complete : true;
}
