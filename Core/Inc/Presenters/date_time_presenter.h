#ifndef CORE_INC_PRESENTERS_DATE_TIME_PRESENTER_H
#define CORE_INC_PRESENTERS_DATE_TIME_PRESENTER_H

#include "date_time_viewmodel.h"
#include "input_task.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief DateTime Presenter - handles logic for 3-page wizard:
 *        Page 0: Date selection (Day, Month, Year rollers)
 *        Page 1: Time selection (Hour, Minute rollers)
 *        Page 2: Summer time switch (On/Off)
 */

typedef struct DateTimePresenter DateTimePresenter_t;

/**
 * @brief Initialize the date/time presenter
 */
DateTimePresenter_t* DateTimePresenter_Init(void);

/**
 * @brief Deinitialize the date/time presenter
 */
void DateTimePresenter_Deinit(DateTimePresenter_t *presenter);

/**
 * @brief Handle input event for the current wizard page
 */
void DateTimePresenter_HandleEvent(DateTimePresenter_t *presenter, const Input2VPEvent_t *event);

/**
 * @brief Get current wizard page (0, 1, or 2)
 */
uint8_t DateTimePresenter_GetCurrentPage(DateTimePresenter_t *presenter);

/**
 * @brief Check if configuration is complete (all 3 pages done)
 */
bool DateTimePresenter_IsComplete(DateTimePresenter_t *presenter);

/**
 * @brief Get the current data
 */
const DateTime_ViewModelData_t* DateTimePresenter_GetData(DateTimePresenter_t *presenter);

/**
 * @brief Update view - called when state changes
 */
void DateTimePresenter_OnViewUpdateNeeded(DateTimePresenter_t *presenter);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_PRESENTERS_DATE_TIME_PRESENTER_H */
