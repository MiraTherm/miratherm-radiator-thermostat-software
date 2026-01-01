#ifndef CORE_INC_PRESENTERS_FACTORY_RESET_PRESENTER_H
#define CORE_INC_PRESENTERS_FACTORY_RESET_PRESENTER_H

#include "input_task.h"
#include "system_task.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FactoryResetPresenter FactoryResetPresenter_t;

/**
 * @brief Initialize the Factory Reset Presenter
 * @param vp2system_queue Queue to send events to System Task
 */
FactoryResetPresenter_t* FactoryResetPresenter_Init(osMessageQueueId_t vp2system_queue);

/**
 * @brief Deinitialize the Factory Reset Presenter
 */
void FactoryResetPresenter_Deinit(FactoryResetPresenter_t *presenter);

/**
 * @brief Handle input events
 */
void FactoryResetPresenter_HandleEvent(FactoryResetPresenter_t *presenter, const Input2VPEvent_t *event);

/**
 * @brief Periodic run/tick
 */
void FactoryResetPresenter_Run(FactoryResetPresenter_t *presenter, uint32_t current_tick);

/**
 * @brief Check if the flow is complete (e.g. cancelled)
 */
bool FactoryResetPresenter_IsComplete(FactoryResetPresenter_t *presenter);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_FACTORY_RESET_PRESENTER_H */
