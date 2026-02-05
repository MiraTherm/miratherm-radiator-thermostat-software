#include "loading_presenter.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "loading_view.h"
#include <stdint.h>
#include <stdlib.h>

#define ANIMATION_PERIOD_MS 500
#define ANIMATION_PERIOD_TICKS pdMS_TO_TICKS(ANIMATION_PERIOD_MS)

/**
 * @brief Internal presenter structure
 */
typedef struct LoadingPresenter {
  LoadingView_t *view; /* Reference to the view */
  LoadingViewData_t data;
  uint32_t last_animation_time; /* Timestamp of last animation frame change */
} LoadingPresenter_t;

/**
 * @brief Initialize the loading presenter
 */
LoadingPresenter_t *LoadingPresenter_Init(LoadingView_t *view) {
  LoadingPresenter_t *presenter =
      (LoadingPresenter_t *)malloc(sizeof(LoadingPresenter_t));
  if (!presenter)
    return NULL;

  presenter->view = view;

  /* Initialize with default state */
  presenter->data.progress = 0;
  presenter->data.animation_frame = 0;
  presenter->last_animation_time = 0;

  return presenter;
}

/**
 * @brief Deinitialize the loading presenter
 */
void LoadingPresenter_Deinit(LoadingPresenter_t *presenter) {
  if (presenter)
    free(presenter);
}

/**
 * @brief Get the current data
 */
const LoadingViewData_t *
LoadingPresenter_GetData(LoadingPresenter_t *presenter) {
  if (!presenter)
    return NULL;
  return &presenter->data;
}

/**
 * @brief Periodic run/tick for presenter updates
 */
void LoadingPresenter_Run(LoadingPresenter_t *presenter,
                          uint32_t current_tick) {
  if (!presenter || !presenter->view)
    return;

  /* Update animation frame only if ANIMATION_PERIOD_TICKS has elapsed */
  if (current_tick - presenter->last_animation_time >= ANIMATION_PERIOD_TICKS) {
    presenter->last_animation_time = current_tick;
    presenter->data.animation_frame = (presenter->data.animation_frame + 1) % 3;
  }

  /* Render the current state to the view */
  LoadingView_Render(presenter->view, &presenter->data);
}

/**
 * @brief Update the message displayed
 */
void LoadingPresenter_SetMessage(LoadingPresenter_t *presenter,
                                 const char *message) {
  if (!presenter || !presenter->view)
    return;

  LoadingView_SetMessage(presenter->view, message);
}
