#ifndef CORE_INC_VIEWS_LOADING_VIEW_H
#define CORE_INC_VIEWS_LOADING_VIEW_H

#include "loading_viewmodel.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Loading View - displays animated text with configurable parameters
 */

typedef struct LoadingView LoadingView_t;

/**
 * @brief Initialize the loading view with custom parameters
 * @param message Base message to display (e.g., "Installation", "Adaptation")
 * @param alignment Alignment of the object (e.g., LV_ALIGN_LEFT_MID)
 * @param x_ofs X offset from alignment position
 * @return Pointer to the initialized LoadingView or NULL on failure
 */
LoadingView_t* LoadingView_Init(const char *message, lv_align_t alignment, int16_t x_ofs);

/**
 * @brief Deinitialize the loading view
 */
void LoadingView_Deinit(LoadingView_t *view);

/**
 * @brief Render/update the view with data from view model
 */
void LoadingView_Render(LoadingView_t *view, const Loading_ViewModelData_t *data);

/**
 * @brief Update the message displayed
 */
void LoadingView_SetMessage(LoadingView_t *view, const char *message);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_LOADING_VIEW_H */
