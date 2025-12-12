#ifndef CORE_INC_VIEWS_INSTALLATION_VIEW_H
#define CORE_INC_VIEWS_INSTALLATION_VIEW_H

#include "installation_viewmodel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Installation View - displays "Installation..." indefinitely
 */

typedef struct InstallationView InstallationView_t;
typedef struct InstallationPresenter InstallationPresenter_t;

/**
 * @brief Initialize the installation view
 */
InstallationView_t* InstallationView_Init(void);

/**
 * @brief Deinitialize the installation view
 */
void InstallationView_Deinit(InstallationView_t *view);

/**
 * @brief Render/update the view with data from view model
 */
void InstallationView_Render(InstallationView_t *view, const Installation_ViewModelData_t *data);


#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_INSTALLATION_VIEW_H */
