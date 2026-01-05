#include "set_value_view.h"
#include "lvgl_port_display.h"
#include <stdlib.h>
#include <string.h>

typedef struct SetValueView
{
    lv_obj_t *screen;
    lv_obj_t *label_title;
    lv_obj_t *roller_value;
    lv_obj_t *label_unit;
    lv_obj_t *label_hint_left;
    lv_obj_t *label_hint_center;
    
    uint16_t last_selected_index;
    const char *last_options_str;  /* Cache options string pointer */
} SetValueView_t;

SetValueView_t* SetValueView_Init(const char *title, const char *unit, const char *options)
{
    SetValueView_t *view = (SetValueView_t *)malloc(sizeof(SetValueView_t));
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
    lv_label_set_text(view->label_title, title ? title : "Set value:");
    lv_obj_align(view->label_title, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_size(view->label_title, LV_HOR_RES, 14);
    lv_obj_set_style_text_color(view->label_title, lv_color_white(), 0);
    lv_obj_set_style_text_align(view->label_title, LV_TEXT_ALIGN_CENTER, 0);

    view->roller_value = lv_roller_create(view->screen);
    if (options)
    {
        lv_roller_set_options(view->roller_value, options, LV_ROLLER_MODE_NORMAL);
    }
    lv_roller_set_selected(view->roller_value, 0, LV_ANIM_OFF);
    lv_obj_align(view->roller_value, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(view->roller_value, 48, 31);
    lv_obj_set_style_text_color(view->roller_value, lv_color_black(), LV_PART_SELECTED);

    if (unit)
    {
        view->label_unit = lv_label_create(view->screen);
        lv_label_set_text(view->label_unit, unit);
        lv_obj_align(view->label_unit, LV_ALIGN_CENTER, 42, 0);
        lv_obj_set_style_text_color(view->label_unit, lv_color_white(), 0);
        lv_obj_set_style_text_font(view->label_unit, &lv_font_montserrat_16, 0);
    }
    else
    {
        view->label_unit = NULL;
    }

    view->label_hint_left = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_left, "<");
    lv_obj_align(view->label_hint_left, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_text_color(view->label_hint_left, lv_color_white(), 0);

    view->label_hint_center = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_center, "O");
    lv_obj_align(view->label_hint_center, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_text_color(view->label_hint_center, lv_color_white(), 0);

    view->last_selected_index = 0xFFFF;
    view->last_options_str = NULL;

    lv_port_unlock();
    return view;
}

void SetValueView_Deinit(SetValueView_t *view)
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

void SetValueView_Render(SetValueView_t *view, const SetValue_ViewModelData_t *data)
{
    if (!view || !data) return;
    if (!lv_port_lock()) return;

    /* Only update options if pointer changed (new options provided) */
    if (view->last_options_str != data->options_str)
    {
        if (data->options_str)
        {
            lv_roller_set_options(view->roller_value, data->options_str, LV_ROLLER_MODE_NORMAL);
        }
        view->last_options_str = data->options_str;
    }

    if (view->last_selected_index != data->selected_index)
    {
        lv_roller_set_selected(view->roller_value, data->selected_index, LV_ANIM_OFF);
        view->last_selected_index = data->selected_index;
    }

    lv_port_unlock();
}

void SetValueView_Show(SetValueView_t *view)
{
    if (!view) return;
    if (lv_port_lock())
    {
        lv_scr_load(view->screen);
        lv_port_unlock();
    }
}

void SetValueView_Hide(SetValueView_t *view)
{
    /* Nothing to do, usually handled by showing another screen */
    (void)view;
}

void SetValueView_SetTitle(SetValueView_t *view, const char *title)
{
    if (!view || !title) return;
    if (lv_port_lock())
    {
        lv_label_set_text(view->label_title, title);
        lv_port_unlock();
    }
}

void SetValueView_SetUnit(SetValueView_t *view, const char *unit)
{
    if (!view) return;
    if (lv_port_lock())
    {
        if (unit)
        {
            if (!view->label_unit)
            {
                view->label_unit = lv_label_create(view->screen);
                lv_obj_set_pos(view->label_unit, 92, 25);
                lv_obj_set_size(view->label_unit, 20, 10);
                lv_obj_set_style_text_color(view->label_unit, lv_color_white(), 0);
                lv_obj_set_style_text_font(view->label_unit, &lv_font_montserrat_16, 0);
            }
            lv_label_set_text(view->label_unit, unit);
        }
        else if (view->label_unit)
        {
            lv_obj_del(view->label_unit);
            view->label_unit = NULL;
        }
        lv_port_unlock();
    }
}

void SetValueView_SetOptions(SetValueView_t *view, const char *options)
{
    if (!view || !options) return;
    if (lv_port_lock())
    {
        lv_roller_set_options(view->roller_value, options, LV_ROLLER_MODE_NORMAL);
        lv_port_unlock();
    }
}
