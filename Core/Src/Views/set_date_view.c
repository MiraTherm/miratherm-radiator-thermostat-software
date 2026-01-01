#include "set_date_view.h"
#include "lvgl_port_display.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct SetDateView
{
    lv_obj_t *screen;
    
    lv_obj_t *label_title;
    
    lv_obj_t *roller_day;
    lv_obj_t *roller_month;
    lv_obj_t *roller_year;
    
    lv_obj_t *label_hint_left;
    lv_obj_t *label_hint_center;
    
    char day_options[256];
    char month_options[256];
    char year_options[512];
    
    uint8_t last_day;
    uint8_t last_month;
    uint16_t last_year;
    uint8_t last_active_field;
    
    bool show_back_hint_on_first_field;
} SetDateView_t;

static const char DAY_OPTIONS[] = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n";
static const char MONTH_OPTIONS[] = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n";

static void create_year_options(char *buf, size_t size)
{
    size_t pos = 0;
    for (int i = 2020; i <= 2049; i++)
    {
        if (pos + 6 < size)
        {
            pos += snprintf(buf + pos, size - pos, "%d\n", i);
        }
    }
}

SetDateView_t* SetDateView_Init(const char *title, bool show_back_hint_on_first_field)
{
    SetDateView_t *view = (SetDateView_t *)malloc(sizeof(SetDateView_t));
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
    lv_label_set_text(view->label_title, title ? title : "Set date:");
    lv_obj_align(view->label_title, LV_ALIGN_TOP_MID, 2, 0);
    lv_obj_set_size(view->label_title, 128, 14);
    lv_obj_set_style_text_color(view->label_title, lv_color_white(), 0);
    lv_obj_set_style_text_align(view->label_title, LV_TEXT_ALIGN_CENTER, 0);

    view->last_day = 0xFF;
    view->last_month = 0xFF;
    view->last_year = 0xFFFF;
    view->last_active_field = 0xFF;

    memcpy(view->day_options, DAY_OPTIONS, sizeof(DAY_OPTIONS));
    memcpy(view->month_options, MONTH_OPTIONS, sizeof(MONTH_OPTIONS));
    create_year_options(view->year_options, sizeof(view->year_options));

    view->roller_year = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_year, view->year_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(view->roller_year, 5, LV_ANIM_OFF);
    lv_obj_set_pos(view->roller_year, 2, 16);
    lv_obj_set_size(view->roller_year, 42, 31);
    lv_obj_set_style_text_color(view->roller_year, lv_color_black(), LV_PART_SELECTED);

    view->roller_month = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_month, view->month_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(view->roller_month, 0, LV_ANIM_OFF);
    lv_obj_set_pos(view->roller_month, 45, 16);
    lv_obj_set_size(view->roller_month, 42, 31);
    lv_obj_set_style_bg_color(view->roller_month, lv_color_white(), 0);
    lv_obj_set_style_text_color(view->roller_month, lv_color_black(), LV_PART_SELECTED);

    view->roller_day = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_day, view->day_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(view->roller_day, 0, LV_ANIM_OFF);
    lv_obj_set_pos(view->roller_day, 88, 16);
    lv_obj_set_size(view->roller_day, 42, 31);
    lv_obj_set_style_bg_color(view->roller_day, lv_color_white(), 0);
    lv_obj_set_style_text_color(view->roller_day, lv_color_black(), LV_PART_SELECTED);

    view->label_hint_left = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_left, "<");
    lv_obj_align(view->label_hint_left, LV_ALIGN_BOTTOM_LEFT, 2, 0);
    lv_obj_set_style_text_color(view->label_hint_left, lv_color_white(), 0);

    view->label_hint_center = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_center, "O");
    lv_obj_align(view->label_hint_center, LV_ALIGN_BOTTOM_MID, 2, 0);
    lv_obj_set_style_text_color(view->label_hint_center, lv_color_white(), 0);

    SetDateView_Render(view, &(SetDate_ViewModelData_t){.day=1, .month=1, .year=2025, .active_field=0});

    lv_port_unlock();

    return view;
}

void SetDateView_Deinit(SetDateView_t *view)
{
    if (view)
    {
        if (view->screen)
            lv_obj_del(view->screen);
        free(view);
    }
}

static void update_date_roller_borders(SetDateView_t *view, uint8_t active_field)
{
    if (view->last_active_field != active_field)
    {
        view->last_active_field = active_field;
        
        lv_obj_set_style_border_width(view->roller_year, 0, 0);
        lv_obj_set_style_border_width(view->roller_month, 0, 0);
        lv_obj_set_style_border_width(view->roller_day, 0, 0);
        
        lv_obj_t *active_roller = (active_field == 0) ? view->roller_year : 
                                   (active_field == 1) ? view->roller_month : view->roller_day;
        lv_obj_set_style_border_color(active_roller, lv_color_black(), 0);
        lv_obj_set_style_border_width(active_roller, 2, 0);
    }
}

void SetDateView_Render(SetDateView_t *view, const SetDate_ViewModelData_t *data)
{
    if (!view || !data)
        return;

    if (!lv_port_lock())
        return;

    if (view->last_day != data->day)
    {
        view->last_day = data->day;
        lv_roller_set_selected(view->roller_day, data->day - 1, LV_ANIM_OFF);
    }
    if (view->last_month != data->month)
    {
        view->last_month = data->month;
        lv_roller_set_selected(view->roller_month, data->month - 1, LV_ANIM_OFF);
    }
    if (view->last_year != data->year)
    {
        view->last_year = data->year;
        lv_roller_set_selected(view->roller_year, data->year - 2020, LV_ANIM_OFF);
    }
    
    update_date_roller_borders(view, data->active_field);

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

void SetDateView_Show(SetDateView_t *view)
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

void SetDateView_Hide(SetDateView_t *view)
{
    /* Nothing to do, loading another screen hides this one */
    (void)view;
}
