#ifndef CORE_INC_VIEWS_BOOST_VIEW_H
#define CORE_INC_VIEWS_BOOST_VIEW_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef BoostViewData_t
 * @brief View data for boost mode screen
 */
typedef struct
{
    uint16_t remaining_seconds;  /* Countdown from 300 to 0 */
} BoostViewData_t;

typedef struct BoostView BoostView_t;

BoostView_t* BoostView_Init(void);
void BoostView_Deinit(BoostView_t *view);
void BoostView_Render(BoostView_t *view, const BoostViewData_t *model);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_BOOST_VIEW_H */
