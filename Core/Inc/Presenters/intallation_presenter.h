#ifndef CORE_INC_PRESENTERS_INSTALLATION_PRESENTER_H
#define CORE_INC_PRESENTERS_INSTALLATION_PRESENTER_H

#include "installation_viewmodel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Installation Presenter - displays "Installation..." indefinitely
 */

typedef struct InstallationPresenter InstallationPresenter_t;
typedef struct InstallationView InstallationView_t;

/**
 * @brief Initialize the installation presenter with a view
 */
InstallationPresenter_t* InstallationPresenter_Init(InstallationView_t *view);

/**
 * @brief Deinitialize the installation presenter
 */
void InstallationPresenter_Deinit(InstallationPresenter_t *presenter);

/**
 * @brief Get the current data
 */
const Installation_ViewModelData_t* InstallationPresenter_GetData(InstallationPresenter_t *presenter);

/**
 * @brief Periodic run/tick for presenter updates (call regularly)
 */
void InstallationPresenter_Run(InstallationPresenter_t *presenter, uint32_t current_tick);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_INSTALLATION_PRESENTER_H */
