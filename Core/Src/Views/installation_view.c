#include "installation_view.h"

#include "lvgl_port_display.h"
#include <src/misc/lv_area.h>
#include <src/misc/lv_txt.h>
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
    lv_label_set_text(view->label_dots, "Installation...");
    lv_obj_align(view->label_dots, LV_ALIGN_LEFT_MID, 20, 0);
    lv_obj_set_style_text_align(view->label_dots, LV_TEXT_ALIGN_LEFT, 0);
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

        const char *frames[] = {
            "Installation.",
            "Installation..",
            "Installation..."
        };

        uint32_t frame_index = (data->animation_frame) % 3;
        lv_label_set_text(view->label_dots, frames[frame_index]);
    }

    lv_port_unlock();
}
