#include "intallation_presenter.h"
#include <stdlib.h>
#include <stdint.h>

/**
 * @brief Internal presenter structure
 */
typedef struct InstallationPresenter
{
    Installation_ViewModelData_t data;
} InstallationPresenter_t;

/**
 * @brief Initialize the installation presenter
 */
InstallationPresenter_t* InstallationPresenter_Init(void)
{
    InstallationPresenter_t *presenter = (InstallationPresenter_t *)malloc(sizeof(InstallationPresenter_t));
    if (!presenter)
        return NULL;

    /* Initialize with default state */
    presenter->data.progress = 0;

    return presenter;
}

/**
 * @brief Deinitialize the installation presenter
 */
void InstallationPresenter_Deinit(InstallationPresenter_t *presenter)
{
    if (presenter)
        free(presenter);
}

/**
 * @brief Get the current data
 */
const Installation_ViewModelData_t* InstallationPresenter_GetData(InstallationPresenter_t *presenter)
{
    if (!presenter)
        return NULL;
    return &presenter->data;
}
