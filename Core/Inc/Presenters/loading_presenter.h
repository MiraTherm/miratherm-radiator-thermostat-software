#ifndef CORE_INC_PRESENTERS_LOADING_PRESENTER_H
#define CORE_INC_PRESENTERS_LOADING_PRESENTER_H

#include "loading_viewmodel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Loading Presenter - generic presenter for animated loading views
 */

typedef struct LoadingPresenter LoadingPresenter_t;
typedef struct LoadingView LoadingView_t;

/**
 * @brief Initialize the loading presenter with a view
 */
LoadingPresenter_t* LoadingPresenter_Init(LoadingView_t *view);

/**
 * @brief Deinitialize the loading presenter
 */
void LoadingPresenter_Deinit(LoadingPresenter_t *presenter);

/**
 * @brief Get the current data
 */
const Loading_ViewModelData_t* LoadingPresenter_GetData(LoadingPresenter_t *presenter);

/**
 * @brief Periodic run/tick for presenter updates (call regularly)
 */
void LoadingPresenter_Run(LoadingPresenter_t *presenter, uint32_t current_tick);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_LOADING_PRESENTER_H */
