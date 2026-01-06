#include "set_time_slot_view.h"
#include "lvgl_port_display.h"
#include <src/misc/lv_anim.h>
#include <src/misc/lv_area.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

typedef struct SetTimeSlotView
{
    lv_obj_t *screen;
    
    lv_obj_t *label_title;
    
    lv_obj_t *roller_end_hour;
    lv_obj_t *roller_end_minute;
    
    lv_obj_t *label_start_time;  /* HH:MM format when locked */
    lv_obj_t *label_end_time;    /* HH:MM format when locked */
    
    lv_obj_t *label_dash;
    
    lv_obj_t *label_hint_left;
    lv_obj_t *label_hint_center;
    
    uint8_t last_start_hour;
    uint8_t last_start_minute;
    uint8_t last_end_hour;
    uint8_t last_end_minute;
    uint8_t last_active_field;
    bool last_start_time_locked;
    bool last_end_time_locked;
    int16_t last_start_label_x;  /* Cache alignment position */
} SetTimeSlotView_t;

static const char HOUR_OPTIONS[] = "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n";
static const char MINUTE_OPTIONS[] = "00\n05\n10\n15\n20\n25\n30\n35\n40\n45\n50\n55\n";

SetTimeSlotView_t* SetTimeSlotView_Init(const char *title)
{
    SetTimeSlotView_t *view = (SetTimeSlotView_t *)malloc(sizeof(SetTimeSlotView_t));
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
        lv_port_unlock();
        free(view);
        return NULL;
    }

    lv_obj_set_style_bg_color(view->screen, lv_color_black(), 0);
    lv_obj_set_size(view->screen, LV_HOR_RES, LV_VER_RES);

    view->label_title = lv_label_create(view->screen);
    lv_label_set_text(view->label_title, title ? title : "Set time slot:");
    lv_obj_align(view->label_title, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_size(view->label_title, LV_HOR_RES, 14);
    lv_obj_set_style_text_color(view->label_title, lv_color_white(), 0);
    lv_obj_set_style_text_align(view->label_title, LV_TEXT_ALIGN_CENTER, 0);

    view->last_start_hour = 0xFF;
    view->last_start_minute = 0xFF;
    view->last_end_hour = 0xFF;
    view->last_end_minute = 0xFF;
    view->last_active_field = 0xFF;
    view->last_start_time_locked = false;
    view->last_end_time_locked = false;
    view->last_start_label_x = INT16_MIN;  /* Initialize with sentinel value */

    view->label_start_time = lv_label_create(view->screen);
    lv_label_set_text(view->label_start_time, "00:00");
    lv_obj_align(view->label_start_time, LV_ALIGN_LEFT_MID, 5, 0);
    lv_obj_set_style_text_color(view->label_start_time, lv_color_white(), 0);

    view->label_dash = lv_label_create(view->screen);
    lv_label_set_text(view->label_dash, "-");
    lv_obj_align(view->label_dash, LV_ALIGN_CENTER, -10, 0);
    lv_obj_set_style_text_color(view->label_dash, lv_color_white(), 0);

    view->roller_end_hour = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_end_hour, HOUR_OPTIONS, LV_ROLLER_MODE_NORMAL);
    lv_obj_align(view->roller_end_hour, LV_ALIGN_CENTER, 14, 0);
    lv_obj_set_size(view->roller_end_hour, 32, 31);
    lv_obj_set_style_text_color(view->roller_end_hour, lv_color_black(), LV_PART_SELECTED);

    view->label_end_time = lv_label_create(view->screen);
    lv_label_set_text(view->label_end_time, "00:00");
    lv_obj_align(view->label_end_time, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_set_style_text_color(view->label_end_time, lv_color_white(), 0);
    lv_obj_add_flag(view->label_end_time, LV_OBJ_FLAG_HIDDEN);

    view->roller_end_minute = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_end_minute, MINUTE_OPTIONS, LV_ROLLER_MODE_NORMAL);
    lv_obj_align(view->roller_end_minute, LV_ALIGN_CENTER, 48, 0);
    lv_obj_set_size(view->roller_end_minute, 32, 31);
    lv_obj_set_style_text_color(view->roller_end_minute, lv_color_black(), LV_PART_SELECTED);

    view->label_hint_left = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_left, "<");
    lv_obj_align(view->label_hint_left, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_text_color(view->label_hint_left, lv_color_white(), 0);

    view->label_hint_center = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_center, "O");
    lv_obj_align(view->label_hint_center, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_text_color(view->label_hint_center, lv_color_white(), 0);

    lv_port_unlock();

    return view;
}

void SetTimeSlotView_Deinit(SetTimeSlotView_t *view)
{
    if (!view) return;
    if (lv_port_lock())
    {
        if (view->screen)
            lv_obj_del(view->screen);
        lv_port_unlock();
    }
    free(view);
}

void SetTimeSlotView_Render(SetTimeSlotView_t *view, const SetTimeSlot_ViewModelData_t *data)
{
    if (!view || !data) return;
    if (!lv_port_lock()) return;

    /* Handle start time lock state change and position */
    if (view->last_start_time_locked != data->start_time_locked)
    {
        if (data->start_time_locked)
            lv_obj_clear_flag(view->label_start_time, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(view->label_start_time, LV_OBJ_FLAG_HIDDEN);
        view->last_start_time_locked = data->start_time_locked;
    }

    /* Only update alignment if state actually changed */
    int16_t new_x = (data->start_time_locked && data->end_time_locked) ? -25 : 5;
    if (view->last_start_label_x != new_x)
    {
        int16_t align = (data->start_time_locked && data->end_time_locked) ? LV_ALIGN_CENTER : LV_ALIGN_LEFT_MID;
        lv_obj_align(view->label_start_time, align, new_x, 0);
        view->last_start_label_x = new_x;
    }

    /* Handle end time lock state change */
    if (view->last_end_time_locked != data->end_time_locked)
    {
        if (data->end_time_locked)
        {
            lv_obj_add_flag(view->roller_end_hour, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(view->roller_end_minute, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(view->label_end_time, LV_OBJ_FLAG_HIDDEN);
            /* Move dash label some pixels to the right when end time is locked */
            lv_obj_align(view->label_dash, LV_ALIGN_CENTER, -1, 0);
        }
        else
        {
            lv_obj_clear_flag(view->roller_end_hour, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(view->roller_end_minute, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(view->label_end_time, LV_OBJ_FLAG_HIDDEN);
            /* Reset dash label position when end time is unlocked */
            lv_obj_align(view->label_dash, LV_ALIGN_CENTER, -10, 0);
        }

        if(data->start_time_locked)
        {
            lv_obj_align(view->label_end_time, LV_ALIGN_CENTER, 24, 0);
        }
        else
        {
            lv_obj_align(view->label_end_time, LV_ALIGN_LEFT_MID, -5, 0);
        }

        view->last_end_time_locked = data->end_time_locked;
    }

    if (view->last_start_hour != data->start_hour || view->last_start_minute != data->start_minute)
    {
        lv_label_set_text_fmt(view->label_start_time, "%02d:%02d", (int)data->start_hour, (int)data->start_minute);
        view->last_start_hour = data->start_hour;
        view->last_start_minute = data->start_minute;
    }
    if (view->last_end_hour != data->end_hour || view->last_end_minute != data->end_minute)
    {
        lv_label_set_text_fmt(view->label_end_time, "%02d:%02d", (int)data->end_hour, (int)data->end_minute);
        lv_roller_set_selected(view->roller_end_hour, data->end_hour, LV_ANIM_OFF);
        lv_roller_set_selected(view->roller_end_minute, data->end_minute / 5, LV_ANIM_OFF);
        view->last_end_hour = data->end_hour;
        view->last_end_minute = data->end_minute;
    }

    /* Highlight active field with border - only update if active field changed */
    if (view->last_active_field != data->active_field)
    {
        /* Clear borders from both rollers */
        lv_obj_set_style_border_width(view->roller_end_hour, 0, 0);
        lv_obj_set_style_border_width(view->roller_end_minute, 0, 0);
        
        /* Set border on active roller */
        lv_obj_t *active_roller = NULL;
        if (data->active_field == 2)
            active_roller = view->roller_end_hour;
        else if (data->active_field == 3)
            active_roller = view->roller_end_minute;
        
        if (active_roller)
        {
            lv_obj_set_style_border_color(active_roller, lv_color_black(), 0);
            lv_obj_set_style_border_width(active_roller, 2, 0);
        }
        
        view->last_active_field = data->active_field;
    }

    lv_port_unlock();
}

void SetTimeSlotView_Show(SetTimeSlotView_t *view)
{
    if (!view) return;
    if (lv_port_lock())
    {
        lv_scr_load(view->screen);
        lv_port_unlock();
    }
}

void SetTimeSlotView_Hide(SetTimeSlotView_t *view)
{
    (void)view;
}

void SetTimeSlotView_SetTitle(SetTimeSlotView_t *view, const char *title)
{
    if (!view || !title) return;
    if (lv_port_lock())
    {
        lv_label_set_text(view->label_title, title);
        lv_port_unlock();
    }
}
