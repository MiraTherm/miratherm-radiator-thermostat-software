#ifndef CORE_INC_VIEWS_DATE_TIME_VIEW_H
#define CORE_INC_VIEWS_DATE_TIME_VIEW_H

#include "date_time_presenter.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief DateTime View - renders the 3-page wizard
 */

typedef struct DateTimeView DateTimeView_t;

/**
 * @brief Initialize the date/time view
 */
DateTimeView_t* DateTimeView_Init(DateTimePresenter_t *presenter);

/**
 * @brief Deinitialize the date/time view
 */
void DateTimeView_Deinit(DateTimeView_t *view);

/**
 * @brief Render/update the current page
 */
void DateTimeView_Render(DateTimeView_t *view);

/**
 * @brief Handle page transitions (next/previous)
 */
void DateTimeView_NextPage(DateTimeView_t *view);
void DateTimeView_PreviousPage(DateTimeView_t *view);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_DATE_TIME_VIEW_H */
