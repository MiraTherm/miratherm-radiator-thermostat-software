#ifndef CORE_INC_VIEWS_HOME_VIEW_H
#define CORE_INC_VIEWS_HOME_VIEW_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HomeView HomeView_t;

typedef struct
{
    uint8_t hour;
    uint8_t minute;
    float current_temp;
    float target_temp;
    uint8_t slot_end_hour;
    uint8_t slot_end_minute;
    uint8_t battery_percentage;
    /* Operational mode is static "Auto" for now */
} HomeViewModel_t;

HomeView_t* HomeView_Init(void);
void HomeView_Deinit(HomeView_t *view);
void HomeView_Render(HomeView_t *view, const HomeViewModel_t *model);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_HOME_VIEW_H */
