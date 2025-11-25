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
 * @param disp_obj LVGL display object
 * @param area Area to update
 * @param px_map Pixel data - with COLOR_DEPTH=1, this is packed 1-bit data (I1 format)
 */
static void lvgl_ssd1306_flush_cb(lv_display_t * disp_obj, const lv_area_t * area, uint8_t * px_map)
{
    /* Get area dimensions */
    uint16_t w = lv_area_get_width(area);
    uint16_t h = lv_area_get_height(area);
    uint16_t x = area->x1;
    uint16_t y = area->y1;

    /* If this is a full screen update, clear the buffer first */
    if (x == 0 && y == 0 && w == DISPLAY_WIDTH && h == DISPLAY_HEIGHT) {
        memset(display_buffer, 0x00, sizeof(display_buffer));
    }

    /* With LV_COLOR_DEPTH=1 (I1 format), px_map contains packed 1-bit pixels */
    /* Each byte contains 8 pixels, MSB-first (horizontal packing) */
    
    for (uint16_t row = 0; row < h; row++) {
        for (uint16_t col = 0; col < w; col++) {
            /* Current screen coordinates */
            uint16_t px = x + col;
            uint16_t py = y + row;

            /* Calculate position in LVGL's I1 packed buffer (row-major, MSB first) */
            uint16_t lvgl_byte_idx = row * ((w + 7) / 8) + col / 8;
            uint8_t lvgl_bit_idx = 7 - (col % 8);  /* MSB first */

            /* Extract pixel bit from LVGL buffer */
            uint8_t pixel_bit = (px_map[lvgl_byte_idx] >> lvgl_bit_idx) & 1;

            /* SSD1306 buffer layout: page-based (8 pixels per byte, vertically stacked) */
            uint16_t page = py / 8;
            uint8_t pixel_in_page = py % 8;
            uint16_t disp_byte_idx = page * DISPLAY_WIDTH + px;

            /* Bounds check */
            if (disp_byte_idx >= sizeof(display_buffer)) {
                continue;
            }

            /* Update display buffer */
            if (pixel_bit) {
                display_buffer[disp_byte_idx] |= (1 << pixel_in_page);
            } else {
                display_buffer[disp_byte_idx] &= ~(1 << pixel_in_page);
            }
        }
    }

    /* Update display */
    ssd1306_FillBuffer(display_buffer, sizeof(display_buffer));
    ssd1306_UpdateScreen();

    /* Notify LVGL that flushing is complete */
    lv_display_flush_ready(disp_obj);
}

/**
 * @brief Initialize LVGL with SSD1306 display
 */
lv_display_t * lvgl_ssd1306_init(void)
{
    /* Initialize SSD1306 display */
    ssd1306_Init();
    
    /* Clear the display buffer completely */
    memset(display_buffer, 0x00, sizeof(display_buffer));
    ssd1306_FillBuffer(display_buffer, sizeof(display_buffer));
    ssd1306_UpdateScreen();

    /* Create LVGL display object */
    disp = lv_display_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if (disp == NULL) {
        return NULL;
    }

    /* Allocate and set draw buffer - use full screen buffer for direct mode */
    static lv_color_t buf[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    lv_display_set_buffers(disp, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_FULL);

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
