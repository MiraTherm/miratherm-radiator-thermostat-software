#include "installation_view.h"

#include "lvgl_port_display.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Internal view structure
 */
typedef struct InstallationView
{
    /* LVGL objects */
    lv_obj_t *screen;
    lv_obj_t *label_dots;
    
    /* Animation state */
    uint32_t last_animation_frame;  /* Last rendered frame */
} InstallationView_t;

/**
 * @brief Initialize the installation view (optimized for 128x64)
 */
InstallationView_t* InstallationView_Init(void)
{
    InstallationView_t *view = (InstallationView_t *)malloc(sizeof(InstallationView_t));
    if (!view)
        return NULL;

    view->last_animation_frame = 0;

    if (!lv_port_lock())
    {
        free(view);
        return NULL;
    }

    /* Create main screen */
    view->screen = lv_obj_create(NULL);
    if (!view->screen)
    {
        free(view);
        return NULL;
    }

    lv_obj_set_style_bg_color(view->screen, lv_color_black(), 0);
    lv_obj_set_size(view->screen, LV_HOR_RES, LV_VER_RES);

    /* Create animated dots label - centered, minimal text */
    view->label_dots = lv_label_create(view->screen);
    lv_label_set_text(view->label_dots, ".");
    lv_obj_set_pos(view->label_dots, 60, 28);
    lv_obj_set_size(view->label_dots, 8, 8);
    lv_obj_set_style_text_align(view->label_dots, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(view->label_dots, lv_color_white(), 0);

    /* Load the screen to make it visible */
    lv_scr_load(view->screen);

    lv_port_unlock();

    return view;
}

/**
 * @brief Deinitialize the installation view
 */
void InstallationView_Deinit(InstallationView_t *view)
{
    if (view)
    {
        if (view->screen)
            lv_obj_del(view->screen);
        free(view);
    }
}

/**
 * @brief Render/update the view with animated dots
 */
void InstallationView_Render(InstallationView_t *view, const Installation_ViewModelData_t *data)
{
    if (!view || !data)
        return;

    /* The mutex has to be recursive! */
    if (!lv_port_lock())
        return;

    /* Ensure screen is active */
    lv_scr_load(view->screen);

    /* Update animation only if frame changed */
    if (view->last_animation_frame != data->animation_frame)
    {
        view->last_animation_frame = data->animation_frame;

        const char *dots[] = {
            ".",
            "..",
            "...",
            "...."
        };

        uint32_t dot_index = (data->animation_frame) % 4;
        lv_label_set_text(view->label_dots, dots[dot_index]);
    }

    lv_port_unlock();
}
