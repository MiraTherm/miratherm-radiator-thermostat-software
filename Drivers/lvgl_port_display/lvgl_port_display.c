/**
 ******************************************************************************
 * @file lvgl_port_display.c
 * @brief Implementation of LVGL port for use on STM32WB55 with SH1106 display 
 * driver.
 *
 * Currently a SH1106 monochrome display is being used.
 * To replace the display driver, modify this file accordingly.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 MiraTherm.
 * This file is licensed under GPL-3.0 License.
 * For details, see the LICENSE file in the project root directory.
 *
 ******************************************************************************
 */
#include "lvgl_port_display.h"
#include "ssd1306.h"
#include "task_debug.h"

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "task.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Display buffer configuration: 128x8 pixels = 1024 bytes */
#define PARTIAL_BUF_SIZE (SSD1306_WIDTH * 8)

/* Display command and control constants for SSD1306/SH1106 */
#define SSD1306_PAGE_START_ADDR 0xB0   /* Page address base (0xB0-0xB7) */
#define SSD1306_LOWER_COL_ADDR 0x00    /* Lower column address nibble */
#define SSD1306_LOWER_COL_MASK 0x0F    /* Mask for lower column bits */
#define SSD1306_UPPER_COL_ADDR 0x10    /* Upper column address nibble */
#define SSD1306_UPPER_COL_MASK 0x0F    /* Mask for upper column bits */

/* SH1106 maps RAM columns 2-129 to physical columns 0-127 */
#define SH1106_COL_OFFSET 2

/* Bit manipulation macros - optimized for speed */
#define BIT_SET(a, b) ((a) |= (1U << (b)))
#define BIT_CLEAR(a, b) ((a) &= ~(1U << (b)))

/**
 * @brief Write a bit value to a specific location in a buffer.
 *
 * @param buf  Buffer array containing the target byte.
 * @param idx  Index of the byte in the buffer.
 * @param bit  Bit position within the byte (0-7).
 * @param val  Bit value (non-zero = set, zero = clear).
 */
#define WRITE_BIT(buf, idx, bit, val)                                          \
  do {                                                                         \
    if (val)                                                                   \
      BIT_SET(buf[idx], bit);                                                  \
    else                                                                       \
      BIT_CLEAR(buf[idx], bit);                                                \
  } while (0)

/** @brief Number of bits per byte. */
#define BYTE_BITS 8

/** @brief Mask to extract bit position within a byte (y & 0x07). */
#define BIT_MASK 0x07

/** @brief Logarithm base 2 of BYTE_BITS; used for fast division/multiplication.
 */
#define ROW_BITS 3

/** @brief Bit shift for converting column address to upper/lower nibbles. */
#define COL_SHIFT 4

/**
 * @brief LVGL rendering mutex handle.
 *
 * Protects concurrent access to LVGL objects from multiple tasks
 * (display task, input task, etc.). Uses recursive mutex to allow
 * nested locking from the same task.
 */
static osMutexId_t s_lvgl_mutex;

/* Acquire LVGL rendering mutex for exclusive access */
bool lv_port_lock(void) {
  if (s_lvgl_mutex == NULL) {
    return false;
  }
  return osMutexAcquire(s_lvgl_mutex, osWaitForever) == osOK;
}

/* Release LVGL rendering mutex */
void lv_port_unlock(void) {
  if (s_lvgl_mutex == NULL) {
    return;
  }
  osMutexRelease(s_lvgl_mutex);
}

/* LVGL rendering task - handles timer callbacks and display updates */
void StartLVGLTask(void *argument) {
  /* Infinite loop - dedicated LVGL rendering task */
  (void)argument;
#if OS_TASKS_DEBUG
  printf("LVGLTask running (heap=%lu)\n",
         (unsigned long)xPortGetFreeHeapSize());
  osDelay(10);
#endif
  for (;;) {
    /* Acquire lock for LVGL rendering */
    if (lv_port_lock()) {
      /* Handle LVGL timers and rendering */
      lv_timer_handler();
      lv_port_unlock();
    }

    /* Yield to other tasks - adjust delay based on display refresh rate */
    osDelay(pdMS_TO_TICKS(1U));
  }
}

/**
 * @brief Display flush callback for rendering partial display updates.
 *
 * This callback is invoked by LVGL after rendering a portion of the display.
 * It transfers the rendered pixel data from the color buffer to the
 * SSD1306/SH1106 display via I2C.
 *
 * The function:
 * 1. Calculates the page and column ranges for the update area
 * 2. Adjusts column addressing for SH1106 offset (columns 2-129)
 * 3. Writes each page of pixel data to the display
 * 4. Notifies LVGL that the flush is complete
 *
 * @param[in] disp_drv    Pointer to LVGL display driver.
 * @param[in] area        Pointer to the display area to flush (coordinates).
 * @param[in] color_p     Pointer to the pixel buffer in monochrome format.
 *                        Each byte represents 8 vertical pixels.
 *
 * @note This function is called automatically by LVGL during rendering.
 * @note Optimized for minimal I2C overhead with batch command writes.
 *
 * @see rounder_cb()
 * @see set_pixel_cb()
 */
static inline void flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                            lv_color_t *color_p) {

  uint8_t row_start = area->y1 >> ROW_BITS;
  uint8_t row_end = area->y2 >> ROW_BITS;
  uint8_t *buf = (uint8_t *)color_p;

  /* Calculate column addresses for the area width */
  uint16_t col_width = area->x2 - area->x1 + 1;

  /* Add SH1106 column offset to starting column */
  uint16_t col_start = area->x1 + SH1106_COL_OFFSET;
  uint8_t lower_col =
      SSD1306_LOWER_COL_ADDR | (col_start & SSD1306_LOWER_COL_MASK);
  uint8_t upper_col = SSD1306_UPPER_COL_ADDR |
                      ((col_start >> COL_SHIFT) & SSD1306_UPPER_COL_MASK);

  /* Batch command writes to reduce I2C overhead */
  for (uint8_t row = row_start; row <= row_end; row++) {
    /* Set page address */
    ssd1306_WriteCommand(SSD1306_PAGE_START_ADDR | row);
    /* Set column address in one operation */
    ssd1306_WriteCommand(lower_col);
    ssd1306_WriteCommand(upper_col);

    /* Write data directly with minimal overhead */
    ssd1306_WriteData(buf, col_width);
    buf += col_width;
  }

  lv_disp_flush_ready(disp_drv);
}

/**
 * @brief Set pixel callback for drawing individual pixels.
 *
 * This callback is invoked by LVGL to set a single pixel in the display buffer.
 * For monochrome displays, each byte contains 8 pixels arranged vertically.
 * This function uses fast bit arithmetic to locate and modify the target pixel.
 *
 * Pixel location calculation:
 * - Row index: y >> 3 (divide by 8)
 * - Bit position: y & 0x7 (modulo 8)
 * - Byte offset: x + (row_offset × buffer_width)
 *
 * @param[in]     disp_drv  Pointer to LVGL display driver (unused).
 * @param[in,out] buf       Pointer to the pixel buffer.
 * @param[in]     buf_w     Width of the buffer in pixels.
 * @param[in]     x         X coordinate of the pixel.
 * @param[in]     y         Y coordinate of the pixel.
 * @param[in]     color     Pixel color (monochrome: any non-zero value sets
 * bit).
 * @param[in]     opa       Opacity value (unused for monochrome display).
 *
 * @note This function is called for advanced drawing operations.
 * @note Optimized with bitwise operations to avoid division/modulo.
 * @note For monochrome, only the color.full field is checked (0 = off, non-zero
 * = on).
 *
 * @see set_px_cb
 * @see flush_cb()
 */
static inline void set_pixel_cb(struct _lv_disp_drv_t *disp_drv, uint8_t *buf,
                                lv_coord_t buf_w, lv_coord_t x, lv_coord_t y,
                                lv_color_t color, lv_opa_t opa) {
  (void)disp_drv;
  (void)opa;

  /* Fast bit calculation without division/modulo */
  const uint32_t row_offset = (uint32_t)(y >> ROW_BITS);
  const uint32_t stride = (uint32_t)buf_w;
  const uint32_t byte_index = (uint32_t)x + stride * row_offset;
  const uint8_t bit_mask = 1U << (y & BIT_MASK);

  if (color.full) {
    buf[byte_index] |= bit_mask;
  } else {
    buf[byte_index] &= ~bit_mask;
  }
}

/**
 * @brief Display area rounding callback for monochrome display compatibility.
 *
 * For monochrome displays, pixels are organized as 8 bits per byte vertically.
 * LVGL rendering areas must be aligned to these 8-pixel boundaries to ensure
 * efficient rendering and avoid partial byte updates.
 *
 * This callback rounds the provided drawing area to the nearest 8-pixel
 * boundaries:
 * - Y1 (top):    Rounds down to the nearest multiple of 8
 * - Y2 (bottom): Rounds up to the nearest multiple of 8 + 7
 *
 * Example: y1=10, y2=20 → y1=8, y2=23 (page-aligned)
 *
 * @param[in]     disp_drv  Pointer to LVGL display driver (unused).
 * @param[in,out] area      Pointer to the display area coordinates.
 *                          X coordinates remain unchanged.
 *                          Y coordinates are adjusted to page boundaries.
 *
 * @note This function is essential for correct rendering on monochrome
 * displays.
 * @note Called automatically by LVGL before each render operation.
 * @note X coordinate alignment is not required for this display.
 *
 * @see flush_cb()
 */
static inline void rounder_cb(struct _lv_disp_drv_t *disp_drv,
                              lv_area_t *area) {
  (void)disp_drv;

  area->y1 &= ~BIT_MASK;
  area->y2 = (area->y2 & ~BIT_MASK) | BIT_MASK;
}

/* Initialize LVGL display driver, buffers, and callbacks */
static void lv_port_disp_init(void) {
  static lv_disp_draw_buf_t draw_buf;
  static lv_color_t screenBuffer1[PARTIAL_BUF_SIZE];
  static lv_color_t screenBuffer2[PARTIAL_BUF_SIZE];

  /* Clear buffers using fast memset instead of loop */
  memset(screenBuffer1, 0, sizeof(screenBuffer1));
  memset(screenBuffer2, 0, sizeof(screenBuffer2));

  /* Initialize the display buffer */
  lv_disp_draw_buf_init(&draw_buf, screenBuffer1, screenBuffer2,
                        PARTIAL_BUF_SIZE);

  /* Initialize display driver */
  static lv_disp_drv_t disp_drv_ssd1306;
  lv_disp_drv_init(&disp_drv_ssd1306);

  /* Configure display resolution and refresh mode */
  disp_drv_ssd1306.hor_res = SSD1306_WIDTH;
  disp_drv_ssd1306.ver_res = SSD1306_HEIGHT;
  disp_drv_ssd1306.full_refresh =
      0; /* Use partial refresh to avoid artifacts */
  disp_drv_ssd1306.rotated = LV_DISP_ROT_NONE;

  /* Register display callbacks */
  disp_drv_ssd1306.flush_cb = flush_cb;
  disp_drv_ssd1306.rounder_cb = rounder_cb;
  disp_drv_ssd1306.set_px_cb = set_pixel_cb;

  /* Set display buffer and register driver */
  disp_drv_ssd1306.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv_ssd1306);
}

/* Initialize complete display system: mutex, hardware, and LVGL */
void display_system_init(void) {
  const osMutexAttr_t mutex_attr = {
      .name = "LVGL Mutex", .attr_bits = osMutexPrioInherit | osMutexRecursive};
  s_lvgl_mutex = osMutexNew(&mutex_attr);

  ssd1306_Init();
  lv_init();
  lv_port_disp_init();
}
