/**
 * @file lvgl_ssd1306.c
 * @brief LVGL integration with SSD1306 OLED display over I2C1
 */

#include "lvgl_ssd1306.h"
#include "ssd1306.h"
#include <string.h>

/* Display dimensions */
#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 64

/* Buffer for pixel data (1 bit per pixel for monochrome) */
static uint8_t display_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];

/* LVGL display object */
static lv_display_t *disp = NULL;

/**
 * @brief Flush callback for LVGL to update display
 * @param disp LVGL display object
 * @param area Area to update
 * @param px_map Pixel data in ARGB8888 format
 */
static void lvgl_ssd1306_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    /* Get buffer size for this area */
    uint16_t w = lv_area_get_width(area);
    uint16_t h = lv_area_get_height(area);
    uint16_t x = area->x1;
    uint16_t y = area->y1;

    /* Convert 32-bit color data to 1-bit monochrome and copy to SSD1306 buffer */
    for (uint16_t row = 0; row < h; row++) {
        for (uint16_t col = 0; col < w; col++) {
            /* Get pixel from LVGL buffer (32-bit ARGB) */
            uint32_t pixel = *((uint32_t *)px_map + row * w + col);

            /* Convert to luminance (simple average of RGB) */
            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = pixel & 0xFF;
            uint8_t gray = (r + g + b) / 3;

            /* Determine if pixel should be on or off (threshold at 128) */
            uint8_t pixel_on = (gray >= 128) ? 1 : 0;

            /* Calculate byte and bit position in SSD1306 buffer */
            uint16_t px = x + col;
            uint16_t py = y + row;
            uint16_t byte_idx = (py / 8) * DISPLAY_WIDTH + px;
            uint8_t bit_idx = py % 8;

            /* Set or clear the bit */
            if (pixel_on) {
                display_buffer[byte_idx] |= (1 << bit_idx);
            } else {
                display_buffer[byte_idx] &= ~(1 << bit_idx);
            }
        }
    }

    /* Update the display with the modified buffer */
    ssd1306_FillBuffer(display_buffer, sizeof(display_buffer));
    ssd1306_UpdateScreen();

    /* Tell LVGL that flushing is done */
    lv_display_flush_ready(disp);
}

/**
 * @brief Initialize LVGL with SSD1306 display
 */
lv_display_t * lvgl_ssd1306_init(void)
{
    /* Initialize SSD1306 display */
    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();

    /* Create LVGL display object */
    disp = lv_display_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if (disp == NULL) {
        return NULL;
    }

    /* Allocate and set draw buffer */
    static lv_color_t buf[DISPLAY_WIDTH * DISPLAY_HEIGHT / 10];
    lv_display_set_buffers(disp, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    /* Set flush callback */
    lv_display_set_flush_cb(disp, lvgl_ssd1306_flush_cb);

    return disp;
}

/**
 * @brief Set the display on/off
 */
void lvgl_ssd1306_display_on(uint8_t on)
{
    ssd1306_SetDisplayOn(on);
}
