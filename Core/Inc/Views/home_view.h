#ifndef CORE_INC_VIEWS_HOME_VIEW_H
#define CORE_INC_VIEWS_HOME_VIEW_H

#include <stdint.h>
#include <stdbool.h>
#include "home_viewmodel.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HomeView HomeView_t;

HomeView_t* HomeView_Init(void);
void HomeView_Deinit(HomeView_t *view);
void HomeView_Render(HomeView_t *view, const Home_ViewModelData_t *model);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_HOME_VIEW_H */
