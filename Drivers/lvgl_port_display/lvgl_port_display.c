/* lvgl_port_display.c
 * Implementation of display callbacks and combined initialization for LVGL
 * using an SSD1306 monochrome display.
 */

#include "lvgl_port_display.h"
#include "ssd1306.h"
#include <stdint.h>

/* Display buffer configuration */
#define PARTIAL_BUF_SIZE (SSD1306_WIDTH * 8)  /* 128 * 8 = 1024 bytes */

/* SSD1306 command constants */
#define SSD1306_PAGE_START_ADDR 0xB0
#define SSD1306_LOWER_COL_ADDR 0x00
#define SSD1306_LOWER_COL_MASK 0x0F
#define SSD1306_UPPER_COL_ADDR 0x10
#define SSD1306_UPPER_COL_MASK 0x0F

/* Bit manipulation macros */
#define BIT_SET(a, b)   ((a) |= (1U << (b)))
#define BIT_CLEAR(a, b) ((a) &= ~(1U << (b)))

/* Bit operations for monochrome display */
#define BYTE_BITS 8
#define BIT_MASK 0x07
#define ROW_BITS 3  /* log2(BYTE_BITS) */

static void flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area,
		lv_color_t *color_p) {

	uint8_t row_start = area->y1 >> ROW_BITS;
	uint8_t row_end = area->y2 >> ROW_BITS;
	uint8_t *buf = (uint8_t*) color_p;

	for (uint8_t row = row_start; row <= row_end; row++) {
		/* Set the page start address */
		ssd1306_WriteCommand(SSD1306_PAGE_START_ADDR | row);
		/* Set the lower column address */
		ssd1306_WriteCommand(SSD1306_LOWER_COL_ADDR | (area->x1 & SSD1306_LOWER_COL_MASK));
		/* Set the upper column address */
		ssd1306_WriteCommand(SSD1306_UPPER_COL_ADDR | ((area->x1 >> 4) & SSD1306_UPPER_COL_MASK));

		for (uint16_t x = area->x1; x <= area->x2; x++) {
			ssd1306_WriteData(buf, 1);
			buf++;
		}
	}

	lv_disp_flush_ready(disp_drv);
}

static void set_pixel_cb(struct _lv_disp_drv_t *disp_drv, uint8_t *buf,
		lv_coord_t buf_w, lv_coord_t x, lv_coord_t y, lv_color_t color,
		lv_opa_t opa) {
	(void) disp_drv;
	(void) opa;

	uint16_t byte_index = x + ((y >> ROW_BITS) * buf_w);
	uint8_t bit_index = y & BIT_MASK;
	
	if (color.full == 0) {
		BIT_SET(buf[byte_index], bit_index);
	} else {
		BIT_CLEAR(buf[byte_index], bit_index);
	}
}

static void rounder_cb(struct _lv_disp_drv_t *disp_drv, lv_area_t *area) {
	(void) disp_drv;
	
	area->y1 = (area->y1 & (~BIT_MASK));
	area->y2 = (area->y2 & (~BIT_MASK)) + (BYTE_BITS - 1);
}

static void lv_port_disp_init(void) {
	static lv_disp_draw_buf_t draw_buf;
	static lv_color_t screenBuffer1[PARTIAL_BUF_SIZE];
	static lv_color_t screenBuffer2[PARTIAL_BUF_SIZE];

	/* Initialize the display buffer */
	lv_disp_draw_buf_init(&draw_buf, screenBuffer1, screenBuffer2, PARTIAL_BUF_SIZE);

	/* Initialize display driver */
	static lv_disp_drv_t disp_drv_ssd1306;
	lv_disp_drv_init(&disp_drv_ssd1306);

	/* Configure display resolution and refresh mode */
	disp_drv_ssd1306.hor_res = SSD1306_WIDTH;
	disp_drv_ssd1306.ver_res = SSD1306_HEIGHT;
	disp_drv_ssd1306.full_refresh = 1;
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
	ssd1306_Init();
	lv_init();
	lv_port_disp_init();
}

