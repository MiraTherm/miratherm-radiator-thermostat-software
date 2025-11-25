/**
 * @file lvgl_ssd1306.h
 * @brief LVGL integration with SSD1306 OLED display over I2C1
 */

#ifndef __LVGL_SSD1306_H__
#define __LVGL_SSD1306_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

/**
 * @brief Initialize LVGL with SSD1306 display
 * @note This function must be called after both LVGL and SSD1306 are initialized
 * @return lv_display_t pointer to the display object or NULL on error
 */
lv_display_t * lvgl_ssd1306_init(void);

/**
 * @brief Set the display on/off
 * @param on 1 to turn on, 0 to turn off
 */
void lvgl_ssd1306_display_on(uint8_t on);

#ifdef __cplusplus
}
#endif

#endif /* __LVGL_SSD1306_H__ */
