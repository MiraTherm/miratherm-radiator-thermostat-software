#include "set_dst_view.h"
#include "lvgl_port_display.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct SetDstView
{
    lv_obj_t *screen;
    
    lv_obj_t *label_step_caption;
    
    lv_obj_t *label_dst;
    lv_obj_t *checkbox_dst;
    
    lv_obj_t *label_hint_left;
    lv_obj_t *label_hint_center;
    
    uint8_t last_dst_state;
} SetDstView_t;

SetDstView_t* SetDstView_Init(void)
{
    SetDstView_t *view = (SetDstView_t *)malloc(sizeof(SetDstView_t));
    if (!view)
        return NULL;

    if (!lv_port_lock())
        return NULL;

    view->screen = lv_obj_create(NULL);
    if (!view->screen)
    {
        free(view);
        return NULL;
    }

    lv_obj_set_style_bg_color(view->screen, lv_color_black(), 0);
    lv_obj_set_size(view->screen, LV_HOR_RES, LV_VER_RES);

    view->label_step_caption = lv_label_create(view->screen);
    lv_label_set_text(view->label_step_caption, "Summer time");
    lv_obj_set_pos(view->label_step_caption, 0, 0);
    lv_obj_set_size(view->label_step_caption, 128, 10);
    lv_obj_set_style_text_color(view->label_step_caption, lv_color_white(), 0);
    lv_obj_set_style_text_align(view->label_step_caption, LV_TEXT_ALIGN_CENTER, 0);

    view->last_dst_state = 0xFF;

    view->label_dst = lv_label_create(view->screen);
    lv_label_set_text(view->label_dst, "ON:");
    lv_obj_set_pos(view->label_dst, 40, 18);
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

    SetDstView_Render(view, &(SetDst_ViewModelData_t){.is_summer_time=true});

    lv_port_unlock();

    return view;
}

void SetDstView_Deinit(SetDstView_t *view)
{
    if (view)
    {
        if (view->screen)
            lv_obj_del(view->screen);
        free(view);
    }
}

void SetDstView_Render(SetDstView_t *view, const SetDst_ViewModelData_t *data)
{
    if (!view || !data)
        return;

    if (!lv_port_lock())
        return;

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

    lv_port_unlock();
}

void SetDstView_Show(SetDstView_t *view)
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

void SetDstView_Hide(SetDstView_t *view)
{
    (void)view;
}
