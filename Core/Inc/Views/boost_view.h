#ifndef CORE_INC_VIEWS_BOOST_VIEW_H
#define CORE_INC_VIEWS_BOOST_VIEW_H

#include <stdint.h>
#include <stdbool.h>
#include "boost_viewmodel.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BoostView BoostView_t;

BoostView_t* BoostView_Init(void);
void BoostView_Deinit(BoostView_t *view);
void BoostView_Render(BoostView_t *view, const Boost_ViewModelData_t *model);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_BOOST_VIEW_H */
