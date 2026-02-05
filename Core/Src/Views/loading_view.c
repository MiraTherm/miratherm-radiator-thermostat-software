#include "loading_view.h"

#include "lvgl_port_display.h"
#include <src/misc/lv_area.h>
#include <src/misc/lv_txt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MESSAGE_LEN 64

/**
 * @brief Internal view structure
 */
typedef struct LoadingView {
  /* LVGL objects */
  lv_obj_t *screen;
  lv_obj_t *label_dots;

  /* Configuration */
  char base_message[MAX_MESSAGE_LEN];
  lv_align_t alignment;
  int16_t x_ofs;

  /* Animation state */
  uint32_t last_animation_frame; /* Last rendered frame */
} LoadingView_t;

/**
 * @brief Initialize the loading view with custom parameters
 */
LoadingView_t *LoadingView_Init(const char *message, lv_align_t alignment,
                                int16_t x_ofs) {
  if (!message)
    return NULL;

  LoadingView_t *view = (LoadingView_t *)malloc(sizeof(LoadingView_t));
  if (!view)
    return NULL;

  /* Store configuration */
  strncpy(view->base_message, message, MAX_MESSAGE_LEN - 1);
  view->base_message[MAX_MESSAGE_LEN - 1] = '\0';
  view->alignment = alignment;
  view->x_ofs = x_ofs;
  view->last_animation_frame = 0;

  if (!lv_port_lock()) {
    free(view);
    return NULL;
  }

  /* Create main screen */
  view->screen = lv_obj_create(NULL);
  if (!view->screen) {
    free(view);
    return NULL;
  }

  lv_obj_set_style_bg_color(view->screen, lv_color_black(), 0);
  lv_obj_set_size(view->screen, LV_HOR_RES, LV_VER_RES);

  /* Create animated dots label */
  view->label_dots = lv_label_create(view->screen);
  lv_label_set_text(view->label_dots, "");
  lv_obj_align(view->label_dots, alignment, x_ofs, 0);
  lv_obj_set_style_text_align(view->label_dots, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_set_style_text_color(view->label_dots, lv_color_white(), 0);

  /* Load the screen to make it visible */
  lv_scr_load(view->screen);

  lv_port_unlock();

  return view;
}

/**
 * @brief Update the message displayed
 */
void LoadingView_SetMessage(LoadingView_t *view, const char *message) {
  if (!view || !message)
    return;

  strncpy(view->base_message, message, MAX_MESSAGE_LEN - 1);
  view->base_message[MAX_MESSAGE_LEN - 1] = '\0';

  /* Force re-render on next cycle or immediately if needed,
     but Render() handles the text construction so it will pick it up. */
}

/**
 * @brief Deinitialize the loading view
 */
void LoadingView_Deinit(LoadingView_t *view) {
  if (view) {
    if (view->screen)
      lv_obj_del(view->screen);
    free(view);
  }
}

/**
 * @brief Render/update the view with animated dots
 */
void LoadingView_Render(LoadingView_t *view,
                        const LoadingViewData_t *data) {
  if (!view || !data)
    return;

  /* The mutex has to be recursive! */
  if (!lv_port_lock())
    return;

  /* Ensure screen is active */
  lv_scr_load(view->screen);

  /* Update animation only if frame changed */
  if (view->last_animation_frame != data->animation_frame) {
    view->last_animation_frame = data->animation_frame;

    /* Build the text with animated dots */
    char animated_text[MAX_MESSAGE_LEN + 4];
    uint32_t frame_index = (data->animation_frame) % 3;

    strncpy(animated_text, view->base_message, MAX_MESSAGE_LEN - 1);
    for (uint32_t i = 0; i <= frame_index; i++) {
      strncat(animated_text, ".", MAX_MESSAGE_LEN - strlen(animated_text) - 1);
    }

    lv_label_set_text(view->label_dots, animated_text);
  }

  lv_port_unlock();
}
