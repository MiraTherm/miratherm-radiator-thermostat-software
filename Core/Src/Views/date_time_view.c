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
    lv_obj_t *label_dst_value;
    
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
} DateTimeView_t;

/* Helper function to create day options string */
static void create_day_options(char *buf, size_t size)
{
    size_t pos = 0;
    for (int i = 1; i <= 31; i++)
    {
        if (pos + 4 < size)
        {
            pos += sprintf(buf + pos, "%d\n", i);
        }
    }
}

/* Helper function to create month options string */
static void create_month_options(char *buf, size_t size)
{
    const char *months[] = { "1", "2", "3", "4", "5", "6",
                              "7", "8", "9", "10", "11", "12" };
    size_t pos = 0;
    for (int i = 0; i < 12; i++)
    {
        if (pos + 5 < size)  /* "10\n" is longest for month */
        {
            pos += sprintf(buf + pos, "%s\n", months[i]);
        }
    }
}

/* Helper function to create year options string */
static void create_year_options(char *buf, size_t size)
{
    size_t pos = 0;
    for (int i = 2020; i <= 2049; i++)
    {
        int digits = (i >= 2020) ? 4 : 3;  /* All years are 4 digits */
        if (pos + digits + 2 < size)  /* year + newline + null terminator */
        {
            pos += sprintf(buf + pos, "%d\n", i);
        }
    }
}

/* Helper function to create hour options string */
static void create_hour_options(char *buf, size_t size)
{
    buf[0] = '\0';
    for (int i = 0; i <= 23; i++)
    {
        if (strlen(buf) + 4 < size)
        {
            int len = strlen(buf);
            snprintf(buf + len, size - len, "%02d\n", i);
        }
    }
}

/* Helper function to create minute options string */
static void create_minute_options(char *buf, size_t size)
{
    buf[0] = '\0';
    for (int i = 0; i <= 59; i++)
    {
        if (strlen(buf) + 4 < size)
        {
            int len = strlen(buf);
            snprintf(buf + len, size - len, "%02d\n", i);
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

    /* Prepare roller option strings */
    create_day_options(view->day_options, sizeof(view->day_options));
    create_month_options(view->month_options, sizeof(view->month_options));
    create_year_options(view->year_options, sizeof(view->year_options));
    create_hour_options(view->hour_options, sizeof(view->hour_options));
    create_minute_options(view->minute_options, sizeof(view->minute_options));

    /* Page 0: Date selection - 3 compact rollers (reduced height) */
    view->roller_day = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_day, view->day_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(view->roller_day, 0, LV_ANIM_OFF);
    lv_obj_set_pos(view->roller_day, 2, 16);
    lv_obj_set_size(view->roller_day, 42, 31);
    lv_obj_set_style_text_color(view->roller_day, lv_color_white(), 0);
    lv_obj_set_style_text_color(view->roller_day, lv_color_black(), LV_PART_SELECTED);
    lv_obj_set_style_bg_color(view->roller_day, lv_color_white(), LV_PART_SELECTED);

    view->roller_month = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_month, view->month_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(view->roller_month, 0, LV_ANIM_OFF);
    lv_obj_set_pos(view->roller_month, 45, 16);
    lv_obj_set_size(view->roller_month, 42, 31);
    lv_obj_set_style_text_color(view->roller_month, lv_color_white(), 0);
    lv_obj_set_style_text_color(view->roller_month, lv_color_black(), LV_PART_SELECTED);
    lv_obj_set_style_bg_color(view->roller_month, lv_color_white(), LV_PART_SELECTED);

    view->roller_year = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_year, view->year_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(view->roller_year, 5, LV_ANIM_OFF);  /* Default to 2025 */
    lv_obj_set_pos(view->roller_year, 88, 16);
    lv_obj_set_size(view->roller_year, 42, 31);
    lv_obj_set_style_text_color(view->roller_year, lv_color_white(), 0);
    lv_obj_set_style_text_color(view->roller_year, lv_color_black(), LV_PART_SELECTED);
    lv_obj_set_style_bg_color(view->roller_year, lv_color_white(), LV_PART_SELECTED);

    /* Page 1: Time selection - 2 compact rollers (reduced height) */
    view->roller_hour = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_hour, view->hour_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(view->roller_hour, 12, LV_ANIM_OFF);
    lv_obj_set_pos(view->roller_hour, 22, 16);
    lv_obj_set_size(view->roller_hour, 40, 31);
    lv_obj_set_style_text_color(view->roller_hour, lv_color_white(), 0);
    lv_obj_set_style_text_color(view->roller_hour, lv_color_black(), LV_PART_SELECTED);
    lv_obj_set_style_bg_color(view->roller_hour, lv_color_white(), LV_PART_SELECTED);

    view->roller_minute = lv_roller_create(view->screen);
    lv_roller_set_options(view->roller_minute, view->minute_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(view->roller_minute, 0, LV_ANIM_OFF);
    lv_obj_set_pos(view->roller_minute, 70, 16);
    lv_obj_set_size(view->roller_minute, 40, 31);
    lv_obj_set_style_text_color(view->roller_minute, lv_color_white(), 0);
    lv_obj_set_style_text_color(view->roller_minute, lv_color_black(), LV_PART_SELECTED);
    lv_obj_set_style_bg_color(view->roller_minute, lv_color_white(), LV_PART_SELECTED);

    /* Page 2: Summer time toggle - minimal display */
    view->label_dst = lv_label_create(view->screen);
    lv_label_set_text(view->label_dst, "DST");
    lv_obj_set_pos(view->label_dst, 10, 20);
    lv_obj_set_size(view->label_dst, 50, 20);
    lv_obj_set_style_text_color(view->label_dst, lv_color_white(), 0);

    view->label_dst_value = lv_label_create(view->screen);
    lv_label_set_text(view->label_dst_value, "OFF");
    lv_obj_set_pos(view->label_dst_value, 70, 20);
    lv_obj_set_size(view->label_dst_value, 50, 20);
    lv_obj_set_style_text_color(view->label_dst_value, lv_color_white(), 0);

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

    view->label_hint_right = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_right, ">");
    lv_obj_set_pos(view->label_hint_right, 114, 51);
    lv_obj_set_size(view->label_hint_right, 20, 13);
    lv_obj_set_style_text_color(view->label_hint_right, lv_color_white(), 0);

    /* Initially show page 0 */
    DateTimeView_Render(view);

    /* Load the screen */
    lv_scr_load(view->screen);

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

    /* Hide all pages first */
    lv_obj_add_flag(view->roller_day, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(view->roller_month, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(view->roller_year, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(view->roller_hour, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(view->roller_minute, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(view->label_dst, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(view->label_dst_value, LV_OBJ_FLAG_HIDDEN);

    if (page == 0)
    {
        /* Date selection page - show 3 rollers */
        lv_label_set_text(view->label_step_caption, "Set date:");
        lv_obj_clear_flag(view->roller_day, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(view->roller_month, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(view->roller_year, LV_OBJ_FLAG_HIDDEN);

        lv_roller_set_selected(view->roller_day, data->day - 1, LV_ANIM_OFF);
        lv_roller_set_selected(view->roller_month, data->month - 1, LV_ANIM_OFF);
        lv_roller_set_selected(view->roller_year, data->year - 2020, LV_ANIM_OFF);
    }
    else if (page == 1)
    {
        /* Time selection page - show 2 rollers */
        lv_label_set_text(view->label_step_caption, "Set time:");
        lv_obj_clear_flag(view->roller_hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(view->roller_minute, LV_OBJ_FLAG_HIDDEN);

        lv_roller_set_selected(view->roller_hour, data->hour, LV_ANIM_OFF);
        lv_roller_set_selected(view->roller_minute, data->minute, LV_ANIM_OFF);
    }
    else if (page == 2)
    {
        /* Summer time page - show toggle state */
        lv_label_set_text(view->label_step_caption, "Summer time:");
        lv_obj_clear_flag(view->label_dst, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(view->label_dst_value, LV_OBJ_FLAG_HIDDEN);

        if (data->is_summer_time)
            lv_label_set_text(view->label_dst_value, "ON");
        else
            lv_label_set_text(view->label_dst_value, "OFF");
    }
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
