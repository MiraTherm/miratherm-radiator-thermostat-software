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
    float ambient_temperature;
    float target_temp;
    uint8_t slot_end_hour;
    uint8_t slot_end_minute;
    uint8_t battery_percentage;
    bool is_off_mode;   /* true if target_temp == 4.5 (OFF) */
    bool is_on_mode;    /* true if target_temp == 30.0 (ON) */
    int mode;           /* 0 = MODE_AUTO, 1 = MODE_MANUAL */
} HomeViewModel_t;

HomeView_t* HomeView_Init(void);
void HomeView_Deinit(HomeView_t *view);
void HomeView_Render(HomeView_t *view, const HomeViewModel_t *model);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_VIEWS_HOME_VIEW_H */
