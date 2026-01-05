#include "home_view.h"
#include "lvgl_port_display.h"
#include <src/misc/lv_area.h>
#include <src/misc/lv_txt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct HomeView
{
    lv_obj_t *screen;
    
    lv_obj_t *label_time;
    lv_obj_t *label_battery;
    lv_obj_t *label_target_temp;
    lv_obj_t *label_current_temp;
    lv_obj_t *label_time_slot;
    
    lv_obj_t *label_hint_left; 
    lv_obj_t *label_hint_center;
    lv_obj_t *label_hint_right;

    /* Cache to avoid redrawing if not changed */
    HomeViewModel_t last_model;
    bool first_render;
} HomeView_t;

HomeView_t* HomeView_Init(void)
{
    HomeView_t *view = (HomeView_t *)malloc(sizeof(HomeView_t));
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

    /* Time: Top Left */
    view->label_time = lv_label_create(view->screen);
    lv_obj_align(view->label_time, LV_ALIGN_TOP_LEFT, 0, 2);
    lv_obj_set_style_text_color(view->label_time, lv_color_white(), 0);
    lv_obj_set_style_text_font(view->label_time, &lv_font_montserrat_12, 0);
    lv_label_set_text(view->label_time, "--:--");

    /* Battery: Top Right */
    view->label_battery = lv_label_create(view->screen);
    lv_obj_align(view->label_battery, LV_ALIGN_TOP_RIGHT, 0, 2);
    lv_obj_set_style_text_color(view->label_battery, lv_color_white(), 0);
    lv_obj_set_style_text_font(view->label_battery, &lv_font_montserrat_12, 0);
    lv_label_set_text(view->label_battery, "Bat: --%");

    /* Target Temp: Big Middle Left */
    view->label_target_temp = lv_label_create(view->screen);
    lv_obj_align(view->label_target_temp, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_text_color(view->label_target_temp, lv_color_white(), 0);
    lv_obj_set_style_text_font(view->label_target_temp, &lv_font_montserrat_28, 0);
    lv_label_set_text(view->label_target_temp, "--.-°");

    /* Current Temp: Small Middle Right */
    view->label_current_temp = lv_label_create(view->screen);
    lv_obj_align(view->label_current_temp, LV_ALIGN_LEFT_MID, 73, -7);
    lv_obj_set_style_text_color(view->label_current_temp, lv_color_white(), 0);
    lv_label_set_text(view->label_current_temp, "<- --.-°");

    /* Time Slot: Small Middle Right (below current temp) */
    view->label_time_slot = lv_label_create(view->screen);
    lv_obj_align(view->label_time_slot, LV_ALIGN_LEFT_MID, 73, 5);
    lv_obj_set_style_text_color(view->label_time_slot, lv_color_white(), 0);
    lv_label_set_text(view->label_time_slot, "-> --:--");

    /* Button Hints */
    view->label_hint_left = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_left, "Auto");
    lv_obj_align(view->label_hint_left, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_text_color(view->label_hint_left, lv_color_white(), 0);
    lv_obj_set_style_text_font(view->label_hint_left, &lv_font_montserrat_14, 0);

    view->label_hint_center = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_center, "O");
    lv_obj_align(view->label_hint_center, LV_ALIGN_BOTTOM_MID, -1, 0);
    lv_obj_set_style_text_color(view->label_hint_center, lv_color_white(), 0);
    lv_obj_set_style_text_font(view->label_hint_center, &lv_font_montserrat_16, 0);

    view->label_hint_right = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_right, LV_SYMBOL_BARS);
    lv_obj_align(view->label_hint_right, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_text_color(view->label_hint_right, lv_color_white(), 0);
    lv_obj_set_style_text_font(view->label_hint_right, &lv_font_montserrat_16, 0);

    view->first_render = true;

    lv_scr_load(view->screen);
    lv_port_unlock();

    return view;
}

void HomeView_Deinit(HomeView_t *view)
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

void HomeView_Render(HomeView_t *view, const HomeViewModel_t *model)
{
    if (!view || !model)
        return;

    if (!lv_port_lock())
        return;

    char buf[32];

    /* Time */
    if (view->first_render || view->last_model.hour != model->hour || view->last_model.minute != model->minute)
    {
        snprintf(buf, sizeof(buf), "%02d:%02d", model->hour, model->minute);
        lv_label_set_text(view->label_time, buf);
    }

    /* Battery */
    if (view->first_render || view->last_model.battery_percentage != model->battery_percentage)
    {
        snprintf(buf, sizeof(buf), "Bat: %d%%", model->battery_percentage);
        lv_label_set_text(view->label_battery, buf);
    }

    /* Target Temp */
    if (view->first_render || view->last_model.target_temp != model->target_temp ||
        view->last_model.is_off_mode != model->is_off_mode ||
        view->last_model.is_on_mode != model->is_on_mode)
    {
        if (model->is_off_mode)
        {
            lv_label_set_text(view->label_target_temp, "OFF");
        }
        else if (model->is_on_mode)
        {
            lv_label_set_text(view->label_target_temp, "ON");
        }
        else
        {
            /* Assuming 1 decimal place */
            int temp_int = (int)model->target_temp;
            int temp_dec = (int)((model->target_temp - temp_int) * 10);
            snprintf(buf, sizeof(buf), "%d.%d°", temp_int, temp_dec);
            lv_label_set_text(view->label_target_temp, buf);
        }
    }

    /* Current Temp */
    if (view->first_render || view->last_model.current_temp != model->current_temp)
    {
        int temp_int = (int)model->current_temp;
        int temp_dec = (int)((model->current_temp - temp_int) * 10);
        snprintf(buf, sizeof(buf), "<- %d.%d°", temp_int, temp_dec);
        lv_label_set_text(view->label_current_temp, buf);
    }

    /* Time Slot */
    if (view->first_render || view->last_model.slot_end_hour != model->slot_end_hour || view->last_model.slot_end_minute != model->slot_end_minute)
    {
        snprintf(buf, sizeof(buf), "-> %02d:%02d", model->slot_end_hour, model->slot_end_minute);
        lv_label_set_text(view->label_time_slot, buf);
    }

    view->last_model = *model;
    view->first_render = false;

    lv_port_unlock();
}
