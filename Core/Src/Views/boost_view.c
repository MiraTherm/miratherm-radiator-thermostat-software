#include "boost_view.h"
#include "lvgl_port_display.h"
#include <src/misc/lv_area.h>
#include <src/misc/lv_txt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct BoostView
{
    lv_obj_t *screen;
    
    lv_obj_t *label_title;
    lv_obj_t *label_countdown;
    lv_obj_t *label_hint_center;

    /* Cache to avoid redrawing if not changed */
    BoostViewModel_t last_model;
    bool first_render;
} BoostView_t;

BoostView_t* BoostView_Init(void)
{
    BoostView_t *view = (BoostView_t *)malloc(sizeof(BoostView_t));
    if (!view)
        return NULL;

    if (!lv_port_lock())
    {
        free(view);
        return NULL;
    }

    view->screen = lv_obj_create(NULL);
    if (!view->screen)
    {
        free(view);
        return NULL;
    }

    lv_obj_set_style_bg_color(view->screen, lv_color_black(), 0);
    lv_obj_set_size(view->screen, LV_HOR_RES, LV_VER_RES);

    /* Title: Top */
    view->label_title = lv_label_create(view->screen);
    lv_obj_align(view->label_title, LV_ALIGN_TOP_MID, 0, 2);
    lv_obj_set_style_text_color(view->label_title, lv_color_white(), 0);
    lv_obj_set_style_text_font(view->label_title, &lv_font_montserrat_12, 0);
    lv_label_set_text(view->label_title, "Boost Mode");

    /* Countdown: Big Middle */
    view->label_countdown = lv_label_create(view->screen);
    lv_obj_align(view->label_countdown, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(view->label_countdown, lv_color_white(), 0);
    lv_obj_set_style_text_font(view->label_countdown, &lv_font_montserrat_28, 0);
    lv_label_set_text(view->label_countdown, "300");

    /* Button Hint: Center Button */
    view->label_hint_center = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_center, LV_SYMBOL_CLOSE);
    lv_obj_align(view->label_hint_center, LV_ALIGN_BOTTOM_MID, -1, 0);
    lv_obj_set_style_text_color(view->label_hint_center, lv_color_white(), 0);
    lv_obj_set_style_text_font(view->label_hint_center, &lv_font_montserrat_16, 0);

    view->first_render = true;

    lv_scr_load(view->screen);
    lv_port_unlock();

    return view;
}

void BoostView_Deinit(BoostView_t *view)
{
    if (view)
    {
        if (lv_port_lock())
        {
            if (view->screen)
                lv_obj_del(view->screen);
            lv_port_unlock();
        }
        free(view);
    }
}

void BoostView_Render(BoostView_t *view, const BoostViewModel_t *model)
{
    if (!view || !model)
        return;

    if (!lv_port_lock())
        return;

    char buf[32];

    /* Countdown Timer */
    if (view->first_render || view->last_model.remaining_seconds != model->remaining_seconds)
    {
        snprintf(buf, sizeof(buf), "%u", model->remaining_seconds);
        lv_label_set_text(view->label_countdown, buf);
    }

    view->last_model = *model;
    view->first_render = false;

    lv_port_unlock();
}
