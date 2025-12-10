/* lvgl_port_display.h
 * Porting layer for LVGL + SSD1306 display
 */
#ifndef LVGL_PORT_DISPLAY_H
#define LVGL_PORT_DISPLAY_H

#include "lvgl.h"

#define LVGL_TASK_STACK_SIZE (512U * 4U)

void StartLVGLTask(void *argument);

/* Initialize the display subsystem: initialize SSD1306, LVGL and register the
 * LVGL display driver. This replaces separate calls to `ssd1306_Init()`,
 * `lv_init()` and the port-specific `lv_port_disp_init()`.
 */
void display_system_init(void);

void start_rendering(void);
void stop_rendering(void);

#endif /* LVGL_PORT_DISPLAY_H */
