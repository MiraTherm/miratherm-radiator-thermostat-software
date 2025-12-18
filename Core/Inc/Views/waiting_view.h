#ifndef CORE_INC_VIEWS_WAITING_VIEW_H
#define CORE_INC_VIEWS_WAITING_VIEW_H

#include "waiting_viewmodel.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WaitingView WaitingView_t;

/**
 * @brief Initialize the waiting view
 * @param message Text message to display
 * @param y_ofs Vertical offset for the message label
 * @return Pointer to the initialized WaitingView or NULL on failure
 */
WaitingView_t* WaitingView_Init(const char *message, int16_t y_ofs);

/**
 * @brief Deinitialize the waiting view
 */
void WaitingView_Deinit(WaitingView_t *view);

/**
 * @brief Render/update the view with data from view model
 */
void WaitingView_Render(WaitingView_t *view, const Waiting_ViewModelData_t *data);

/**
 * @brief Set the message displayed in the waiting view
 */
void WaitingView_SetMessage(WaitingView_t *view, const char *message);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_WAITING_VIEW_H */