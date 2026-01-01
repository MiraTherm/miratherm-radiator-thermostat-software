#include "menu_view.h"
#include "lvgl_port_display.h"
#include <src/font/lv_symbol_def.h>
#include <stdlib.h>
#include <string.h>

struct MenuView
{
    lv_obj_t *screen;
    lv_obj_t *list;
    lv_obj_t *btn_offset;
    lv_obj_t *btn_schedule;
    lv_obj_t *btn_factory_rst;
    
    lv_obj_t *label_hint_left;
    lv_obj_t *label_hint_center;

    uint16_t last_selected_index;
};

MenuView_t* MenuView_Init(const char *options)
{
    MenuView_t *view = (MenuView_t *)malloc(sizeof(MenuView_t));
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

    /* Create List */
    view->list = lv_list_create(view->screen);
    lv_obj_set_size(view->list, LV_HOR_RES - 3, LV_VER_RES - 17); /* Leave space for hints */
    lv_obj_align(view->list, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(view->list, lv_color_black(), 0);
    lv_obj_set_style_border_width(view->list, 0, 0);
    lv_obj_set_style_pad_all(view->list, 0, 0);
    lv_obj_set_scrollbar_mode(view->list, LV_SCROLLBAR_MODE_OFF);

    /* Add buttons */
    /* Note: We are hardcoding buttons here because lv_list works by adding buttons, 
       and parsing the newline separated string every time is inefficient. 
       Ideally, the presenter should pass an array or we just know what the menu is. 
       Given the context, we know the items. */

    view->btn_schedule = lv_list_add_btn(view->list, NULL, "Schedule");
    lv_obj_set_style_bg_color(view->btn_schedule, lv_color_black(), 0);
    lv_obj_set_style_text_color(view->btn_schedule, lv_color_white(), 0);
    lv_obj_set_style_bg_color(view->btn_schedule, lv_color_white(), LV_STATE_FOCUS_KEY);
    lv_obj_set_style_text_color(view->btn_schedule, lv_color_black(), LV_STATE_FOCUS_KEY);

    view->btn_offset = lv_list_add_btn(view->list, NULL, "Temp offset");
    lv_obj_set_style_bg_color(view->btn_offset, lv_color_black(), 0);
    lv_obj_set_style_text_color(view->btn_offset, lv_color_white(), 0);
    lv_obj_set_style_bg_color(view->btn_offset, lv_color_white(), LV_STATE_FOCUS_KEY);
    lv_obj_set_style_text_color(view->btn_offset, lv_color_black(), LV_STATE_FOCUS_KEY);

    view->btn_factory_rst = lv_list_add_btn(view->list, NULL, "Factory reset");
    lv_obj_set_style_bg_color(view->btn_factory_rst, lv_color_black(), 0);
    lv_obj_set_style_text_color(view->btn_factory_rst, lv_color_white(), 0);
    lv_obj_set_style_bg_color(view->btn_factory_rst, lv_color_white(), LV_STATE_FOCUS_KEY);
    lv_obj_set_style_text_color(view->btn_factory_rst, lv_color_black(), LV_STATE_FOCUS_KEY);

    /* Hints */
    view->label_hint_left = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_left, "<" LV_SYMBOL_HOME);
    lv_obj_align(view->label_hint_left, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_text_color(view->label_hint_left, lv_color_white(), 0);

    view->label_hint_center = lv_label_create(view->screen);
    lv_label_set_text(view->label_hint_center, "O");
    lv_obj_align(view->label_hint_center, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_text_color(view->label_hint_center, lv_color_white(), 0);

    view->last_selected_index = 0xFFFF;

    /* Create a group for navigation if using keypad/encoder directly with LVGL, 
       but here we are manually handling events in presenter and updating view. 
       So we will manually focus buttons based on index. */

    lv_scr_load(view->screen);
    lv_port_unlock();
    return view;
}

void MenuView_Deinit(MenuView_t *view)
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

void MenuView_Render(MenuView_t *view, const MenuViewModel_t *model)
{
    if (!view || !model) return;
    if (!lv_port_lock()) return;

    /* We ignore options_str update for now as we hardcoded buttons for list */

    if (view->last_selected_index != model->selected_index)
    {
        /* Manually manage focus state to simulate selection */
        lv_obj_clear_state(view->btn_schedule, LV_STATE_FOCUS_KEY);
        lv_obj_clear_state(view->btn_offset, LV_STATE_FOCUS_KEY);
        lv_obj_clear_state(view->btn_factory_rst, LV_STATE_FOCUS_KEY);

        if (model->selected_index == 0)
        {
            lv_obj_add_state(view->btn_schedule, LV_STATE_FOCUS_KEY);
            lv_obj_scroll_to_view(view->btn_schedule, LV_ANIM_OFF);
        }
        else if (model->selected_index == 1)
        {
            lv_obj_add_state(view->btn_offset, LV_STATE_FOCUS_KEY);
            lv_obj_scroll_to_view(view->btn_offset, LV_ANIM_OFF);
        }
        else if (model->selected_index == 2)
        {
            lv_obj_add_state(view->btn_factory_rst, LV_STATE_FOCUS_KEY);
            lv_obj_scroll_to_view(view->btn_factory_rst, LV_ANIM_OFF);
        }
        
        view->last_selected_index = model->selected_index;
    }

    lv_port_unlock();
}

void MenuView_Show(MenuView_t *view)
{
    if (!view) return;
    if (lv_port_lock())
    {
        lv_scr_load(view->screen);
        lv_port_unlock();
    }
}

void MenuView_Hide(MenuView_t *view)
{
    (void)view;
}
