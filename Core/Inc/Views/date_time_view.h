#ifndef CORE_INC_VIEWS_DATE_TIME_VIEW_H
#define CORE_INC_VIEWS_DATE_TIME_VIEW_H

#include "date_time_viewmodel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief DateTime View - renders the 3-page wizard
 */

typedef struct DateTimeView DateTimeView_t;
typedef struct DateTimePresenter DateTimePresenter_t;

/**
 * @brief Initialize the date/time view
 */
DateTimeView_t* DateTimeView_Init(void);

/**
 * @brief Deinitialize the date/time view
 */
void DateTimeView_Deinit(DateTimeView_t *view);

/**
 * @brief Render/update the current page with data from view model
 */
void DateTimeView_Render(DateTimeView_t *view, const DateTime_ViewModelData_t *data);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_DATE_TIME_VIEW_H */
