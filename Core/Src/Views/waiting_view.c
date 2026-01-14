#include "waiting_view.h"
#include "lvgl_port_display.h"
#include <src/misc/lv_txt.h>
#include <stdlib.h>

typedef struct WaitingView {
  /* LVGL screen */
  lv_obj_t *screen;

  /* Message label */
  lv_obj_t *label_message;

  /* Button hints (lower line) */
  lv_obj_t *label_hint_center;

} WaitingView_t;

WaitingView_t *WaitingView_Init(const char *message, int16_t y_ofs) {
  WaitingView_t *view = (WaitingView_t *)malloc(sizeof(WaitingView_t));
  if (!view)
    return NULL;

  if (!lv_port_lock()) {
    free(view);
    return NULL;
  }

  view->screen = lv_obj_create(NULL);
  if (!view->screen) {
    free(view);
    lv_port_unlock();
    return NULL;
  }
  lv_obj_set_style_bg_color(view->screen, lv_color_black(), 0);

  view->label_message = lv_label_create(view->screen);
  lv_label_set_text(view->label_message, message ? message : "");
  lv_obj_align(view->label_message, LV_ALIGN_CENTER, 0, y_ofs);
  lv_obj_set_style_text_align(view->label_message, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(view->label_message, lv_color_white(), 0);
  lv_obj_set_style_text_align(view->label_message, LV_TEXT_ALIGN_CENTER, 0);

  view->label_hint_center = lv_label_create(view->screen);
  lv_label_set_text(view->label_hint_center, "O");
  lv_obj_align(view->label_hint_center, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_text_color(view->label_hint_center, lv_color_white(), 0);

  lv_scr_load(view->screen);
  lv_port_unlock();

  return view;
}

void WaitingView_Deinit(WaitingView_t *view) {
  if (view) {
    if (view->screen)
      lv_obj_del(view->screen);
    free(view);
  }
}

void WaitingView_Render(WaitingView_t *view,
                        const Waiting_ViewModelData_t *data) {
  (void)view;
  (void)data;
  /* Static view, no updates needed */
}

void WaitingView_SetMessage(WaitingView_t *view, const char *message) {
  if (view && view->label_message && message) {
    if (lv_port_lock()) {
      lv_label_set_text(view->label_message, message);
      lv_port_unlock();
    }
  }
}
