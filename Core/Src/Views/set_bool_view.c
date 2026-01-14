#include "set_bool_view.h"
#include "lvgl_port_display.h"
#include <src/misc/lv_area.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct SetBoolView {
  lv_obj_t *screen;

  lv_obj_t *label_title;

  lv_obj_t *checkbox_false;
  lv_obj_t *checkbox_true;

  lv_obj_t *label_hint_left;
  lv_obj_t *label_hint_center;

  uint8_t last_value;
  bool show_back_hint;
} SetBoolView_t;

SetBoolView_t *SetBoolView_Init(const char *title, const char *option_true,
                                const char *option_false, bool show_back_hint) {
  SetBoolView_t *view = (SetBoolView_t *)malloc(sizeof(SetBoolView_t));
  if (!view)
    return NULL;

  if (!lv_port_lock())
    return NULL;

  view->show_back_hint = show_back_hint;

  view->screen = lv_obj_create(NULL);
  if (!view->screen) {
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

  view->last_value = 0xFF;

  /* Option 1 (False) */
  view->checkbox_false = lv_checkbox_create(view->screen);
  lv_checkbox_set_text(view->checkbox_false,
                       option_false ? option_false : "Off");
  lv_obj_align(view->checkbox_false, LV_ALIGN_LEFT_MID, 8, -7);
  lv_obj_set_size(view->checkbox_false, LV_HOR_RES - 8, 20);
  lv_obj_set_style_text_color(view->checkbox_false, lv_color_white(), 0);

  /* Radio button style */
  lv_obj_set_style_radius(view->checkbox_false, LV_RADIUS_CIRCLE,
                          LV_PART_INDICATOR);
  lv_obj_set_style_width(view->checkbox_false, 11, LV_PART_INDICATOR);
  lv_obj_set_style_height(view->checkbox_false, 11, LV_PART_INDICATOR);
  lv_obj_set_style_pad_all(view->checkbox_false, 0, LV_PART_INDICATOR);

  /* Unchecked: White border, Black background */
  lv_obj_set_style_border_color(view->checkbox_false, lv_color_white(),
                                LV_PART_INDICATOR);
  lv_obj_set_style_border_width(view->checkbox_false, 1, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(view->checkbox_false, lv_color_black(),
                            LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(view->checkbox_false, LV_OPA_COVER,
                          LV_PART_INDICATOR);

  /* Checked: White background, Black checkmark */
  lv_obj_set_style_bg_color(view->checkbox_false, lv_color_white(),
                            LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_set_style_text_color(view->checkbox_false, lv_color_black(),
                              LV_PART_INDICATOR | LV_STATE_CHECKED);

  /* Option 2 (True) */
  view->checkbox_true = lv_checkbox_create(view->screen);
  lv_checkbox_set_text(view->checkbox_true, option_true ? option_true : "On");
  lv_obj_align(view->checkbox_true, LV_ALIGN_LEFT_MID, 8, 10);
  lv_obj_set_size(view->checkbox_true, LV_HOR_RES - 8, 20);
  lv_obj_set_style_text_color(view->checkbox_true, lv_color_white(), 0);

  /* Radio button style */
  lv_obj_set_style_radius(view->checkbox_true, LV_RADIUS_CIRCLE,
                          LV_PART_INDICATOR);
  lv_obj_set_style_width(view->checkbox_true, 11, LV_PART_INDICATOR);
  lv_obj_set_style_height(view->checkbox_true, 11, LV_PART_INDICATOR);
  lv_obj_set_style_pad_all(view->checkbox_true, 0, LV_PART_INDICATOR);

  /* Unchecked: White border, Black background */
  lv_obj_set_style_border_color(view->checkbox_true, lv_color_white(),
                                LV_PART_INDICATOR);
  lv_obj_set_style_border_width(view->checkbox_true, 1, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(view->checkbox_true, lv_color_black(),
                            LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(view->checkbox_true, LV_OPA_COVER, LV_PART_INDICATOR);

  /* Checked: White background, Black checkmark */
  lv_obj_set_style_bg_color(view->checkbox_true, lv_color_white(),
                            LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_set_style_text_color(view->checkbox_true, lv_color_black(),
                              LV_PART_INDICATOR | LV_STATE_CHECKED);

  view->label_hint_left = lv_label_create(view->screen);
  lv_label_set_text(view->label_hint_left, "<");
  lv_obj_align(view->label_hint_left, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_text_color(view->label_hint_left, lv_color_white(), 0);

  if (!show_back_hint) {
    lv_obj_add_flag(view->label_hint_left, LV_OBJ_FLAG_HIDDEN);
  }

  view->label_hint_center = lv_label_create(view->screen);
  lv_label_set_text(view->label_hint_center, "O");
  lv_obj_align(view->label_hint_center, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_text_color(view->label_hint_center, lv_color_white(), 0);

  SetBoolView_Render(view, &(SetBool_ViewModelData_t){.value = false});

  lv_port_unlock();

  return view;
}

void SetBoolView_Deinit(SetBoolView_t *view) {
  if (view) {
    if (view->screen)
      lv_obj_del(view->screen);
    free(view);
  }
}

void SetBoolView_Render(SetBoolView_t *view,
                        const SetBool_ViewModelData_t *data) {
  if (!view || !data)
    return;

  if (!lv_port_lock())
    return;

  uint8_t target_index = data->value ? 1 : 0;
  if (view->last_value != target_index) {
    view->last_value = target_index;
    if (data->value) {
      lv_obj_clear_state(view->checkbox_false, LV_STATE_CHECKED);
      lv_obj_add_state(view->checkbox_true, LV_STATE_CHECKED);
    } else {
      lv_obj_add_state(view->checkbox_false, LV_STATE_CHECKED);
      lv_obj_clear_state(view->checkbox_true, LV_STATE_CHECKED);
    }
  }

  lv_port_unlock();
}

void SetBoolView_Show(SetBoolView_t *view) {
  if (view && view->screen) {
    if (lv_port_lock()) {
      lv_scr_load(view->screen);
      lv_port_unlock();
    }
  }
}

void SetBoolView_Hide(SetBoolView_t *view) { (void)view; }
