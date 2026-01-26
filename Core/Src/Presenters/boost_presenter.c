#include "boost_presenter.h"
#include "boost_viewmodel.h"
#include "cmsis_os2.h"
#include "view_presenter_router.h"
#include <stdio.h>
#include <stdlib.h>

struct BoostPresenter {
  BoostView_t *view;
  SystemContextAccessTypeDef *system_context;
};

BoostPresenter_t *
BoostPresenter_Init(BoostView_t *view,
                    SystemContextAccessTypeDef *system_context) {
  if (!view || !system_context)
    return NULL;

  BoostPresenter_t *presenter =
      (BoostPresenter_t *)malloc(sizeof(BoostPresenter_t));
  if (!presenter)
    return NULL;

  presenter->view = view;
  presenter->system_context = system_context;

  return presenter;
}

void BoostPresenter_Deinit(BoostPresenter_t *presenter) {
  if (presenter) {
    free(presenter);
  }
}

void BoostPresenter_HandleEvent(BoostPresenter_t *presenter,
                                const Input2VPEvent_t *event) {
  if (!presenter || !event)
    return;

  /* Handle button presses */
  if (event->button_action == BUTTON_ACTION_PRESSED) {
    switch (event->type) {
    case EVT_MIDDLE_BTN:
      /* Close boost mode and return to home */
      printf("Boost: Close button pressed, exiting boost mode\n");

      /* Restore previous mode before boost */
      if (presenter->system_context) {
        if (osMutexAcquire(presenter->system_context->mutex, 10) == osOK) {
          SystemMode_t previous_mode =
              presenter->system_context->data.mode_before_boost;
          presenter->system_context->data.mode = previous_mode;
          printf("Boost: Restored previous mode (%d)\n", previous_mode);
          osMutexRelease(presenter->system_context->mutex);
        }
      }

      Router_GoToRoute(ROUTE_HOME);
      break;
    default:
      break;
    }
  }
}

void BoostPresenter_Run(BoostPresenter_t *presenter, uint32_t current_tick) {
  if (!presenter || !presenter->view)
    return;

  Boost_ViewModelData_t model = {0};

  /* Calculate remaining time */
  if (osMutexAcquire(presenter->system_context->mutex, 10) == osOK) {
    uint32_t elapsed_ticks = osKernelGetTickCount() -
                             presenter->system_context->data.boost_begin_time;
    uint32_t elapsed_seconds = elapsed_ticks / 1000; /* Convert ms to seconds */

    if (elapsed_seconds >= 300) {
      model.remaining_seconds = 0;

      /* Boost timeout - restore previous mode and return to home */
      printf("Boost: Countdown expired, exiting boost mode\n");
      SystemMode_t previous_mode =
          presenter->system_context->data.mode_before_boost;
      presenter->system_context->data.mode = previous_mode;
      printf("Boost: Restored previous mode (%d)\n", previous_mode);
      osMutexRelease(presenter->system_context->mutex);

      Router_GoToRoute(ROUTE_HOME);
      return;
    } else {
      model.remaining_seconds = 300 - elapsed_seconds;
    }

    osMutexRelease(presenter->system_context->mutex);
  }

  BoostView_Render(presenter->view, &model);
}
