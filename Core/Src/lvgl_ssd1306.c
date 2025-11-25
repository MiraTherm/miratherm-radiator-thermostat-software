/**
 * @file lvgl_ssd1306.c
 * @brief LVGL integration with SSD1306/SH1106 OLED display over I2C1
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

    /* LVGL I1 format is already in pages when rendering full width areas
     * The buffer stride for I1 format is always (width + 7) / 8 bytes per row
     * Each row represents one logical row of pixels (not a page!)
     * We need to reorganize from row-major to SSD1306 page-based format
     */
    
    uint16_t stride = (w + 7) / 8;  /* bytes per row in LVGL buffer */
    
    /* For full screen updates, just copy directly - LVGL I1 format matches SSD1306 page format */
    if (x == 0 && y == 0 && w == DISPLAY_WIDTH && h == DISPLAY_HEIGHT) {
        memcpy(display_buffer, px_map, sizeof(display_buffer));
    } else {
        /* Partial update - convert pixel by pixel */
        for (uint16_t py = 0; py < h; py++) {
            for (uint16_t px = 0; px < w; px++) {
                uint16_t src_idx = py * stride + (px >> 3);      /* Byte index in LVGL buffer */
                uint8_t src_bit = px & 7;                        /* Bit position (LSB = left) */
                uint8_t pixel = (px_map[src_idx] >> src_bit) & 1;
                
                /* Calculate display position */
                uint16_t dst_x = x + px;
                uint16_t dst_y = y + py;
                
                /* Convert to SSD1306 page addressing */
                uint16_t page = dst_y / 8;
                uint8_t bit = dst_y % 8;
                uint16_t dst_idx = page * DISPLAY_WIDTH + dst_x;
                
                if (dst_idx < sizeof(display_buffer)) {
                    if (pixel) {
                        display_buffer[dst_idx] |= (1 << bit);
                    } else {
                        display_buffer[dst_idx] &= ~(1 << bit);
                    }
                }
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
 * @brief Initialize LVGL with SSD1306/SH1106 display
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


