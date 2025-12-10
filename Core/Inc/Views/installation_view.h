#ifndef CORE_INC_VIEWS_INSTALLATION_VIEW_H
#define CORE_INC_VIEWS_INSTALLATION_VIEW_H

#include "intallation_presenter.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Installation View - displays "Installation..." indefinitely
 */

typedef struct InstallationView InstallationView_t;

/**
 * @brief Initialize the installation view
 */
InstallationView_t* InstallationView_Init(InstallationPresenter_t *presenter);

/**
 * @brief Deinitialize the installation view
 */
void InstallationView_Deinit(InstallationView_t *view);

/**
 * @brief Render/update the view
 */
void InstallationView_Render(InstallationView_t *view);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_INSTALLATION_VIEW_H */
