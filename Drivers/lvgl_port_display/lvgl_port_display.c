/* lvgl_port_display.c
 * Implementation of display callbacks and combined initialization for LVGL
 * using an SSD1306 monochrome display.
 */

#include "lvgl_port_display.h"
#include "ssd1306.h"
#include "task_debug.h"

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Display buffer configuration */
#define PARTIAL_BUF_SIZE (SSD1306_WIDTH * 8)  /* 128 * 8 = 1024 bytes */

/* SSD1306 command constants */
#define SSD1306_PAGE_START_ADDR 0xB0
#define SSD1306_LOWER_COL_ADDR 0x00
#define SSD1306_LOWER_COL_MASK 0x0F
#define SSD1306_UPPER_COL_ADDR 0x10
#define SSD1306_UPPER_COL_MASK 0x0F

/* Bit manipulation macros - optimized for speed */
#define BIT_SET(a, b)   ((a) |= (1U << (b)))
#define BIT_CLEAR(a, b) ((a) &= ~(1U << (b)))
#define WRITE_BIT(buf, idx, bit, val) do { if (val) BIT_SET(buf[idx], bit); else BIT_CLEAR(buf[idx], bit); } while(0)

/* Bit operations for monochrome display */
#define BYTE_BITS 8
#define BIT_MASK 0x07
#define ROW_BITS 3  /* log2(BYTE_BITS) */
#define COL_SHIFT 4 /* bits to shift for upper column address */

/* LVGL rendering mutex for thread safety */
static osMutexId_t s_lvgl_mutex;

bool lv_port_lock(void)
{
	if (s_lvgl_mutex == NULL)
	{
		return false;
	}
	return osMutexAcquire(s_lvgl_mutex, osWaitForever) == osOK;
}

void lv_port_unlock(void)
{
	if (s_lvgl_mutex == NULL)
	{
		return;
	}
	osMutexRelease(s_lvgl_mutex);
}

void StartLVGLTask(void *argument)
{
	/* Infinite loop - dedicated LVGL rendering task */
	(void)argument;
#if OS_TASKS_DEBUG
	printf("LVGLTask running (heap=%lu)\n", (unsigned long)xPortGetFreeHeapSize());
	osDelay(10);
#endif
	for(;;)
  {
    /* Acquire lock for LVGL rendering */
    if (lv_port_lock())
    {
      /* Handle LVGL timers and rendering */
      lv_timer_handler();
      lv_port_unlock();
    }

    /* Yield to other tasks - adjust delay based on display refresh rate */
    osDelay(pdMS_TO_TICKS(1U));
  }
}

static inline void flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area,
		lv_color_t *color_p) {

	uint8_t row_start = area->y1 >> ROW_BITS;
	uint8_t row_end = area->y2 >> ROW_BITS;
	uint8_t *buf = (uint8_t*) color_p;
	
	/* Calculate column addresses for the area width */
	uint16_t col_width = area->x2 - area->x1 + 1;
	uint8_t lower_col = SSD1306_LOWER_COL_ADDR | (area->x1 & SSD1306_LOWER_COL_MASK);
	uint8_t upper_col = SSD1306_UPPER_COL_ADDR | ((area->x1 >> COL_SHIFT) & SSD1306_UPPER_COL_MASK);

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

static inline void set_pixel_cb(struct _lv_disp_drv_t *disp_drv, uint8_t *buf,
	lv_coord_t buf_w, lv_coord_t x, lv_coord_t y, lv_color_t color,
	lv_opa_t opa) {
	(void) disp_drv;
	(void) opa;

	/* Fast bit calculation without division/modulo */
	const uint32_t row_offset = (uint32_t)(y >> ROW_BITS);
	const uint32_t stride = (uint32_t) buf_w;
	const uint32_t byte_index = (uint32_t) x + stride * row_offset;
	const uint8_t bit_mask = 1U << (y & BIT_MASK);

	if (color.full) {
		buf[byte_index] |= bit_mask;
	} else {
		buf[byte_index] &= ~bit_mask;
	}
}

static inline void rounder_cb(struct _lv_disp_drv_t *disp_drv, lv_area_t *area) {
	(void) disp_drv;
	
	area->y1 &= ~BIT_MASK;
	area->y2 = (area->y2 & ~BIT_MASK) | BIT_MASK;
}

static void lv_port_disp_init(void) {
	static lv_disp_draw_buf_t draw_buf;
	static lv_color_t screenBuffer1[PARTIAL_BUF_SIZE];
	static lv_color_t screenBuffer2[PARTIAL_BUF_SIZE];

	/* Clear buffers using fast memset instead of loop */
	memset(screenBuffer1, 0, sizeof(screenBuffer1));
	memset(screenBuffer2, 0, sizeof(screenBuffer2));

	/* Initialize the display buffer */
	lv_disp_draw_buf_init(&draw_buf, screenBuffer1, screenBuffer2, PARTIAL_BUF_SIZE);

	/* Initialize display driver */
	static lv_disp_drv_t disp_drv_ssd1306;
	lv_disp_drv_init(&disp_drv_ssd1306);

	/* Configure display resolution and refresh mode */
	disp_drv_ssd1306.hor_res = SSD1306_WIDTH;
	disp_drv_ssd1306.ver_res = SSD1306_HEIGHT;
	disp_drv_ssd1306.full_refresh = 0;  /* Use partial refresh to avoid artifacts */
	disp_drv_ssd1306.rotated = LV_DISP_ROT_NONE;

	/* Register display callbacks */
	disp_drv_ssd1306.flush_cb = flush_cb;
	disp_drv_ssd1306.rounder_cb = rounder_cb;
	disp_drv_ssd1306.set_px_cb = set_pixel_cb;

	/* Set display buffer and register driver */
	disp_drv_ssd1306.draw_buf = &draw_buf;
	lv_disp_drv_register(&disp_drv_ssd1306);
}

void display_system_init(void) {
	/* Create the LVGL rendering mutex */
	const osMutexAttr_t mutex_attr = {
		.name = "LVGL Mutex",
		.attr_bits = osMutexPrioInherit | osMutexRecursive
	};
	s_lvgl_mutex = osMutexNew(&mutex_attr);
	
	ssd1306_Init();
	lv_init();
	lv_port_disp_init();
}

