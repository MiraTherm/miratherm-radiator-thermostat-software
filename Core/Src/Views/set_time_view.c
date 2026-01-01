#include "set_time_view.h"
#include "lvgl_port_display.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct SetTimeView
{
    lv_obj_t *screen;
    
    lv_obj_t *label_title;
    
    lv_obj_t *roller_hour;
    lv_obj_t *roller_minute;
    
    lv_obj_t *label_hint_left;
    lv_obj_t *label_hint_center;
    
    char hour_options[256];
    char minute_options[512];
    
    uint8_t last_hour;
    uint8_t last_minute;
    uint8_t last_active_field;

    bool show_back_hint_on_first_field;
} SetTimeView_t;

static const char HOUR_OPTIONS[] = "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n";
static const char MINUTE_OPTIONS[] = "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59\n";

SetTimeView_t* SetTimeView_Init(const char *title, bool show_back_hint_on_first_field)
{
    SetTimeView_t *view = (SetTimeView_t *)malloc(sizeof(SetTimeView_t));
    if (!view)
        return NULL;

    if (!lv_port_lock())
        return NULL;

    view->show_back_hint_on_first_field = show_back_hint_on_first_field;

    view->screen = lv_obj_create(NULL);
    if (!view->screen)
    {
        free(view);
        return NULL;
    }

    lv_obj_set_style_bg_color(view->screen, lv_color_black(), 0);
    lv_obj_set_size(view->screen, LV_HOR_RES, LV_VER_RES);

    view->label_title = lv_label_create(view->screen);
    lv_label_set_text(view->label_title, title ? title : "Set time:");
    lv_obj_align(view->label_title, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_size(view->label_title, 128, 14);
    lv_obj_set_style_text_color(view->label_title, lv_color_white(), 0);
    lv_obj_set_style_text_align(view->label_title, LV_TEXT_ALIGN_CENTER, 0);

    view->last_hour = 0xFF;
    view->last_minute = 0xFF;
    view->last_active_field = 0xFF;

    memcpy(view->hour_options, HOUR_OPTIONS, sizeof(HOUR_OPTIONS));
    memcpy(view->minute_options, MINUTE_OPTIONS, sizeof(MINUTE_OPTIONS));

    view->roller_hour = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_hour, view->hour_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(view->roller_hour, 12, LV_ANIM_OFF);
    lv_obj_set_pos(view->roller_hour, 30, 16);
    lv_obj_set_size(view->roller_hour, 32, 31);
    lv_obj_set_style_text_color(view->roller_hour, lv_color_black(), LV_PART_SELECTED);

    view->roller_minute = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_minute, view->minute_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(view->roller_minute, 0, LV_ANIM_OFF);
    lv_obj_set_pos(view->roller_minute, 64, 16);
    lv_obj_set_size(view->roller_minute, 32, 31);
    lv_obj_set_style_text_color(view->roller_minute, lv_color_black(), LV_PART_SELECTED);

    view->label_hint_left = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_left, "<");
    lv_obj_align(view->label_hint_left, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_text_color(view->label_hint_left, lv_color_white(), 0);

    view->label_hint_center = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_center, "O");
    lv_obj_align(view->label_hint_center, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_text_color(view->label_hint_center, lv_color_white(), 0);

    SetTimeView_Render(view, &(SetTime_ViewModelData_t){.hour=12, .minute=0, .active_field=0});

    lv_port_unlock();

    return view;
}

void SetTimeView_Deinit(SetTimeView_t *view)
{
    if (view)
    {
        if (view->screen)
            lv_obj_del(view->screen);
        free(view);
    }
}

static void update_time_roller_borders(SetTimeView_t *view, uint8_t active_field)
{
    if (view->last_active_field != active_field)
    {
        view->last_active_field = active_field;
        
        lv_obj_set_style_border_width(view->roller_hour, 0, 0);
        lv_obj_set_style_border_width(view->roller_minute, 0, 0);
        
        lv_obj_t *active_roller = (active_field == 0) ? view->roller_hour : view->roller_minute;
        lv_obj_set_style_border_color(active_roller, lv_color_black(), 0);
        lv_obj_set_style_border_width(active_roller, 2, 0);
    }
}

void SetTimeView_Render(SetTimeView_t *view, const SetTime_ViewModelData_t *data)
{
    if (!view || !data)
        return;

    if (!lv_port_lock())
        return;

    if (view->last_hour != data->hour)
    {
        view->last_hour = data->hour;
        lv_roller_set_selected(view->roller_hour, data->hour, LV_ANIM_OFF);
    }
    if (view->last_minute != data->minute)
    {
        view->last_minute = data->minute;
        lv_roller_set_selected(view->roller_minute, data->minute, LV_ANIM_OFF);
    }
    
    update_time_roller_borders(view, data->active_field);

    if (data->active_field > 0 || view->show_back_hint_on_first_field)
    {
        lv_obj_clear_flag(view->label_hint_left, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(view->label_hint_left, LV_OBJ_FLAG_HIDDEN);
    }

    lv_port_unlock();
}

void SetTimeView_Show(SetTimeView_t *view)
{
    if (view && view->screen)
    {
        if (lv_port_lock())
        {
            lv_scr_load(view->screen);
            lv_port_unlock();
        }
    }
}

void SetTimeView_Hide(SetTimeView_t *view)
{
    (void)view;
}
