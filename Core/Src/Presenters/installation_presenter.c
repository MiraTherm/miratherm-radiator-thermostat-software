#include "intallation_presenter.h"
#include "installation_view.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include <stdlib.h>
#include <stdint.h>

#define ANIMATION_PERIOD_MS 500
#define ANIMATION_PERIOD_TICKS pdMS_TO_TICKS(ANIMATION_PERIOD_MS)

/**
 * @brief Internal presenter structure
 */
typedef struct InstallationPresenter
{
    InstallationView_t *view;    /* Reference to the view */
    Installation_ViewModelData_t data;
    uint32_t last_animation_time;  /* Timestamp of last animation frame change */
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
    presenter->last_animation_time = 0;

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
void InstallationPresenter_Run(InstallationPresenter_t *presenter, uint32_t current_tick)
{
    if (!presenter || !presenter->view)
        return;

    /* Update animation frame only if ANIMATION_PERIOD_TICKS has elapsed */
    if (current_tick - presenter->last_animation_time >= ANIMATION_PERIOD_TICKS)
    {
        presenter->last_animation_time = current_tick;
        presenter->data.animation_frame = (presenter->data.animation_frame + 1) % 3;
    }

    /* Render the current state to the view */
    InstallationView_Render(presenter->view, &presenter->data);
}
