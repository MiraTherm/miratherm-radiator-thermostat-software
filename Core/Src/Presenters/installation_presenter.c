#include "intallation_presenter.h"
#include "installation_view.h"
#include <stdlib.h>
#include <stdint.h>

/**
 * @brief Internal presenter structure
 */
typedef struct InstallationPresenter
{
    InstallationView_t *view;    /* Reference to the view */
    Installation_ViewModelData_t data;
} InstallationPresenter_t;

/**
 * @brief Initialize the installation presenter
 */
InstallationPresenter_t* InstallationPresenter_Init(InstallationView_t *view)
{
    InstallationPresenter_t *presenter = (InstallationPresenter_t *)malloc(sizeof(InstallationPresenter_t));
    if (!presenter)
        return NULL;

    presenter->view = view;

    /* Initialize with default state */
    presenter->data.progress = 0;
    presenter->data.animation_frame = 0;

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

/**
 * @brief Periodic run/tick for presenter updates
 */
void InstallationPresenter_Run(InstallationPresenter_t *presenter)
{
    if (!presenter || !presenter->view)
        return;

    /* Increment animation frame */
    presenter->data.animation_frame = (presenter->data.animation_frame + 1) % 4;

    /* Render the current state to the view */
    InstallationView_Render(presenter->view, &presenter->data);
}
