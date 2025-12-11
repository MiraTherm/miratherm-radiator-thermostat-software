#include "date_time_view.h"
#include "lvgl_port_display.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/**
 * @brief Internal view structure optimized for 128x64 display
 */
typedef struct DateTimeView
{
    DateTimePresenter_t *presenter;
    
    /* LVGL objects for the wizard */
    lv_obj_t *screen;
    
    /* Page captions (upper line) */
    lv_obj_t *label_step_caption;
    
    /* Page 0: Date selection - 3 rollers */
    lv_obj_t *roller_day;
    lv_obj_t *roller_month;
    lv_obj_t *roller_year;
    
    /* Page 1: Time selection - 2 rollers */
    lv_obj_t *roller_hour;
    lv_obj_t *roller_minute;
    
    /* Page 2: Summer time toggle */
    lv_obj_t *label_dst;
    lv_obj_t *checkbox_dst;
    
    /* Button hints (lower line) */
    lv_obj_t *label_hint_left;
    lv_obj_t *label_hint_center;
    lv_obj_t *label_hint_right;
    
    /* Pre-allocated strings for rollers */
    char day_options[256];
    char month_options[256];
    char year_options[512];
    char hour_options[256];
    char minute_options[512];
    
    /* Cached values to avoid unnecessary updates */
    uint8_t last_day;
    uint8_t last_month;
    uint16_t last_year;
    uint8_t last_hour;
    uint8_t last_minute;
    uint8_t last_page;
    uint8_t last_active_field;
    bool last_dst_state;
} DateTimeView_t;

/* Static pre-built option strings for better performance */
static const char DAY_OPTIONS[] = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n";
static const char MONTH_OPTIONS[] = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n";
static const char HOUR_OPTIONS[] = "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n";
static const char MINUTE_OPTIONS[] = "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59\n";

/* Helper function to create year options string */
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

/* Forward declaration */
void DateTimeView_Render(DateTimeView_t *view);


/**
 * @brief Initialize the date/time view (optimized for 128x64 display)
 */
DateTimeView_t* DateTimeView_Init(DateTimePresenter_t *presenter)
{
    if (!presenter)
        return NULL;

    DateTimeView_t *view = (DateTimeView_t *)malloc(sizeof(DateTimeView_t));
    if (!view)
        return NULL;

    view->presenter = presenter;

    if (!lv_port_lock())
        return NULL;

    /* Create main screen */
    view->screen = lv_obj_create(NULL);
    if (!view->screen)
    {
        free(view);
        return NULL;
    }

    lv_obj_set_style_bg_color(view->screen, lv_color_black(), 0);
    lv_obj_set_size(view->screen, LV_HOR_RES, LV_VER_RES);

    /* Step caption label (line 0) */
    view->label_step_caption = lv_label_create(view->screen);
    lv_label_set_text(view->label_step_caption, "Set date:");
    lv_obj_set_pos(view->label_step_caption, 0, 0);
    lv_obj_set_size(view->label_step_caption, 128, 10);
    lv_obj_set_style_text_color(view->label_step_caption, lv_color_white(), 0);
    lv_obj_set_style_text_align(view->label_step_caption, LV_TEXT_ALIGN_CENTER, 0);

    /* Initialize cached values */
    view->last_day = 0xFF;
    view->last_month = 0xFF;
    view->last_year = 0xFFFF;
    view->last_hour = 0xFF;
    view->last_minute = 0xFF;
    view->last_page = 0xFF;
    view->last_active_field = 0xFF;
    view->last_dst_state = 0xFF;

    /* Prepare roller option strings - use static strings */
    memcpy(view->day_options, DAY_OPTIONS, sizeof(DAY_OPTIONS));
    memcpy(view->month_options, MONTH_OPTIONS, sizeof(MONTH_OPTIONS));
    memcpy(view->hour_options, HOUR_OPTIONS, sizeof(HOUR_OPTIONS));
    memcpy(view->minute_options, MINUTE_OPTIONS, sizeof(MINUTE_OPTIONS));
    create_year_options(view->year_options, sizeof(view->year_options));

    /* Page 0: Date selection - 3 compact rollers (reduced height) */
    view->roller_day = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_day, view->day_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(view->roller_day, 0, LV_ANIM_OFF);
    lv_obj_set_pos(view->roller_day, 2, 16);
    lv_obj_set_size(view->roller_day, 42, 31);
    lv_obj_set_style_bg_color(view->roller_day, lv_color_white(), 0);
    lv_obj_set_style_text_color(view->roller_day, lv_color_black(), LV_PART_SELECTED);

    view->roller_month = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_month, view->month_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(view->roller_month, 0, LV_ANIM_OFF);
    lv_obj_set_pos(view->roller_month, 45, 16);
    lv_obj_set_size(view->roller_month, 42, 31);
    lv_obj_set_style_bg_color(view->roller_month, lv_color_white(), 0);
    lv_obj_set_style_text_color(view->roller_month, lv_color_black(), LV_PART_SELECTED);

    view->roller_year = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_year, view->year_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(view->roller_year, 5, LV_ANIM_OFF);  /* Default to 2025 */
    lv_obj_set_pos(view->roller_year, 88, 16);
    lv_obj_set_size(view->roller_year, 42, 31);
    lv_obj_set_style_text_color(view->roller_year, lv_color_black(), LV_PART_SELECTED);

    /* Page 1: Time selection - 2 compact rollers (reduced height) */
    view->roller_hour = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_hour, view->hour_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(view->roller_hour, 12, LV_ANIM_OFF);
    lv_obj_set_pos(view->roller_hour, 22, 16);
    lv_obj_set_size(view->roller_hour, 40, 31);
    lv_obj_set_style_text_color(view->roller_hour, lv_color_black(), LV_PART_SELECTED);

    view->roller_minute = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_minute, view->minute_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(view->roller_minute, 0, LV_ANIM_OFF);
    lv_obj_set_pos(view->roller_minute, 70, 16);
    lv_obj_set_size(view->roller_minute, 40, 31);
    lv_obj_set_style_text_color(view->roller_minute, lv_color_black(), LV_PART_SELECTED);

    /* Page 2: Summer time toggle - checkbox control */
    view->label_dst = lv_label_create(view->screen);
    lv_label_set_text(view->label_dst, "On/Off:");
    lv_obj_set_pos(view->label_dst, 20, 20);
    lv_obj_set_size(view->label_dst, 70, 20);
    lv_obj_set_style_text_color(view->label_dst, lv_color_white(), 0);

    view->checkbox_dst = lv_checkbox_create(view->screen);
    lv_checkbox_set_text(view->checkbox_dst, "");
    lv_obj_set_pos(view->checkbox_dst, 80, 17);
    lv_obj_set_size(view->checkbox_dst, 30, 20);
    lv_obj_set_style_bg_color(view->checkbox_dst, lv_color_white(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(view->checkbox_dst, lv_color_white(), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(view->checkbox_dst, lv_color_white(), LV_PART_INDICATOR);
    lv_obj_set_style_text_color(view->checkbox_dst, lv_color_black(), LV_PART_INDICATOR | LV_STATE_CHECKED);

    /* Button hints (line 51-64 at bottom) */
    view->label_hint_left = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_left, "<");
    lv_obj_set_pos(view->label_hint_left, 6, 51);
    lv_obj_set_size(view->label_hint_left, 20, 13);
    lv_obj_set_style_text_color(view->label_hint_left, lv_color_white(), 0);

    view->label_hint_center = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_center, "OK");
    lv_obj_set_pos(view->label_hint_center, 60, 51);
    lv_obj_set_size(view->label_hint_center, 20, 13);
    lv_obj_set_style_text_color(view->label_hint_center, lv_color_white(), 0);

    /* Initially show page 0 */
    DateTimeView_Render(view);

    /* Load the screen */
    lv_scr_load(view->screen);

    lv_port_unlock();

    return view;
}

/**
 * @brief Deinitialize the date/time view
 */
void DateTimeView_Deinit(DateTimeView_t *view)
{
    if (view)
    {
        if (view->screen)
            lv_obj_del(view->screen);
        free(view);
    }
}

/**
 * @brief Helper to update border for date rollers
 */
static void update_date_roller_borders(DateTimeView_t *view, uint8_t active_field)
{
    /* Only update if active field changed to avoid unnecessary style updates */
    if (view->last_active_field != active_field)
    {
        view->last_active_field = active_field;
        
        /* Reset all borders first */
        lv_obj_set_style_border_width(view->roller_day, 0, 0);
        lv_obj_set_style_border_width(view->roller_month, 0, 0);
        lv_obj_set_style_border_width(view->roller_year, 0, 0);
        
        /* Set border only on active field */
        lv_obj_t *active_roller = (active_field == 0) ? view->roller_day : 
                                   (active_field == 1) ? view->roller_month : view->roller_year;
        lv_obj_set_style_border_color(active_roller, lv_color_black(), 0);
        lv_obj_set_style_border_width(active_roller, 2, 0);
    }
}

/**
 * @brief Helper to update border for time rollers
 */
static void update_time_roller_borders(DateTimeView_t *view, uint8_t active_field)
{
    /* Only update if active field changed to avoid unnecessary style updates */
    if (view->last_active_field != active_field)
    {
        view->last_active_field = active_field;
        
        /* Reset all borders first */
        lv_obj_set_style_border_width(view->roller_hour, 0, 0);
        lv_obj_set_style_border_width(view->roller_minute, 0, 0);
        
        /* Set border only on active field */
        lv_obj_t *active_roller = (active_field == 0) ? view->roller_hour : view->roller_minute;
        lv_obj_set_style_border_color(active_roller, lv_color_black(), 0);
        lv_obj_set_style_border_width(active_roller, 2, 0);
    }
}

/**
 * @brief Render/update the current page
 */
void DateTimeView_Render(DateTimeView_t *view)
{
    if (!view || !view->presenter)
        return;

    uint8_t page = DateTimePresenter_GetCurrentPage(view->presenter);
    const DateTime_ViewModelData_t *data = DateTimePresenter_GetData(view->presenter);

    if (!data)
        return;

    if (!lv_port_lock())
        return;

    /* Only update visibility and borders if page changed */
    if (view->last_page != page)
    {
        view->last_page = page;
        view->last_active_field = 0xFF;  /* Force border update on page change */
        
        /* Hide all elements first */
        lv_obj_add_flag(view->roller_day, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(view->roller_month, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(view->roller_year, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(view->roller_hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(view->roller_minute, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(view->label_dst, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(view->checkbox_dst, LV_OBJ_FLAG_HIDDEN);

        /* Reset all borders */
        lv_obj_set_style_border_width(view->roller_day, 0, 0);
        lv_obj_set_style_border_width(view->roller_month, 0, 0);
        lv_obj_set_style_border_width(view->roller_year, 0, 0);
        lv_obj_set_style_border_width(view->roller_hour, 0, 0);
        lv_obj_set_style_border_width(view->roller_minute, 0, 0);

        /* Show page-specific elements */
        if (page == 0)
        {
            lv_label_set_text(view->label_step_caption, "Set date:");
            lv_obj_clear_flag(view->roller_day, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(view->roller_month, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(view->roller_year, LV_OBJ_FLAG_HIDDEN);
        }
        else if (page == 1)
        {
            lv_label_set_text(view->label_step_caption, "Set time:");
            lv_obj_clear_flag(view->roller_hour, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(view->roller_minute, LV_OBJ_FLAG_HIDDEN);
        }
        else if (page == 2)
        {
            lv_label_set_text(view->label_step_caption, "Summer time");
            lv_obj_clear_flag(view->label_dst, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(view->checkbox_dst, LV_OBJ_FLAG_HIDDEN);
        }
    }

    /* Update page-specific data only if changed */
    if (page == 0)
    {
        /* Date selection page */
        uint8_t active_field = DateTimePresenter_GetDateActiveField(view->presenter);
        
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
        
        update_date_roller_borders(view, active_field);
    }
    else if (page == 1)
    {
        /* Time selection page */
        uint8_t active_field = DateTimePresenter_GetTimeActiveField(view->presenter);
        
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
        
        update_time_roller_borders(view, active_field);
    }
    else if (page == 2)
    {
        /* Summer time page - checkbox control */
        if (view->last_dst_state != data->is_summer_time)
        {
            view->last_dst_state = data->is_summer_time;
            if (data->is_summer_time)
            {
                lv_obj_add_state(view->checkbox_dst, LV_STATE_CHECKED);
            }
            else
            {
                lv_obj_clear_state(view->checkbox_dst, LV_STATE_CHECKED);
            }
        }
    }

    lv_port_unlock();
}


/**
 * @brief Handle page transitions (next/previous)
 */
void DateTimeView_NextPage(DateTimeView_t *view)
{
    if (!view)
        return;
    DateTimeView_Render(view);
}

/**
 * @brief Handle page transitions (previous)
 */
void DateTimeView_PreviousPage(DateTimeView_t *view)
{
    if (!view)
        return;
    DateTimeView_Render(view);
}
