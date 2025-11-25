/**
 * @file lv_conf.h
 * Configuration file for LVGL with SH1106 128x64 OLED Display
 */

#if 1 /* Set this to "1" to enable content */

#ifndef LV_CONF_H
#define LV_CONF_H

/*====================
   COLOR SETTINGS
 *====================*/

/** Color depth: 1 (I1), 8 (L8), 16 (RGB565), 24 (RGB888), 32 (XRGB8888) */
#define LV_COLOR_DEPTH 1

/*=========================
   STDLIB WRAPPER SETTINGS
 *=========================*/

#define LV_USE_STDLIB_MALLOC    LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_STRING    LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_SPRINTF   LV_STDLIB_BUILTIN

#define LV_STDINT_INCLUDE       <stdint.h>
#define LV_STDDEF_INCLUDE       <stddef.h>
#define LV_STDBOOL_INCLUDE      <stdbool.h>
#define LV_INTTYPES_INCLUDE     <inttypes.h>
#define LV_LIMITS_INCLUDE       <limits.h>
#define LV_STDARG_INCLUDE       <stdarg.h>

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN
    /** Size of memory available for `lv_malloc()` in bytes (>= 2kB) */
    #define LV_MEM_SIZE (16 * 1024U)          /**< [bytes] */

    /** Size of the memory expand for `lv_malloc()` in bytes */
    #define LV_MEM_POOL_EXPAND_SIZE 0

    /** Set an address for the memory pool instead of allocating it as a normal array. Can be in external SRAM too. */
    #define LV_MEM_ADR 0     /**< 0: unused*/
#endif  /*LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN*/

/*====================
   HAL SETTINGS
 *====================*/

/** Default display refresh, input device read and animation step period. */
#define LV_DEF_REFR_PERIOD  33      /**< [ms] */

/** Default Dots Per Inch. Used to initialize default sizes such as widgets sized, style paddings.
 * (Not so important, you can adjust it to modify default sizes and spaces.) */
#define LV_DPI_DEF 130              /**< [px/inch] */

/*=================
 * OPERATING SYSTEM
 *=================*/
#define LV_OS_NONE 1
#define LV_USE_OS LV_OS_NONE

/*====================
   FEATURE USAGE
 *====================*/

/** Tiny memory mode - disable unnecessary features */
#define LV_USE_FLOAT 0
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1
#define LV_USE_ASSERT_STYLE 1
#define LV_USE_ASSERT_MSG 1
#define LV_USE_ASSERT_OBJ 1

/*=================
 * DISPLAY SETTINGS
 *=================*/

/** Horizontal resolution in pixels. Set to 128 for SH1106 */
#define LV_HOR_RES_MAX 128

/** Vertical resolution in pixels. Set to 64 for SH1106 */
#define LV_VER_RES_MAX 64

/** Default display background color when images are not set (for display color-mapped display) */
#define LV_COLOR_SCREEN_TRANSP 0

/** Swap the 2 bytes of RGB565 color. Useful if the display has a 8 bit interface (I.e. not SPI) */
#define LV_COLOR_16_SWAP 0

/** Enable drawing on transparent background */
#define LV_COLOR_MIX_ROUND_OFS 0

/*========================
 * LAYOUT SETTINGS
 *========================*/

/** Enable support for Anti-aliasing */
#define LV_ANTIALIAS 0

/** Default image cache size. Image caching is disabled if LV_IMG_CACHE_DEF_SIZE = 0 */
#define LV_IMG_CACHE_DEF_SIZE 0

/** Number of images to be cached. The more the better. */
#define LV_IMG_CACHE_ENTRY_CNT 2

/*======================
 * COMPILER SETTINGS
 *======================*/

/** Define the most common data types used by the library
 * int/uint instead of int32_t for code size optimization */
#define LV_USE_LARGE_COORD 0

/** Optimize draw performance */
#define LV_DRAW_SW_ASM 0

/*==================
 * FONT SETTINGS
 *==================*/

/** Enable built-in fonts */
#define LV_FONT_MONTSERRAT_8  1
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 0

/** Set a default font */
#define LV_FONT_DEFAULT &lv_font_montserrat_8

/*===================
 * OBJECT SETTINGS
 *===================*/

/** Enable the rectangle object */
#define LV_USE_OBJX_TYPES 1

/** Base objects */
#define LV_USE_OBJ 1

/* Objects */
#define LV_USE_ARC       0
#define LV_USE_BAR       0
#define LV_USE_BTN       1
#define LV_USE_BTNMATRIX 0
#define LV_USE_CALENDAR  0
#define LV_USE_CANVAS    0
#define LV_USE_CHECKBOX  0
#define LV_USE_COLORWHEEL 0
#define LV_USE_CONT      0
#define LV_USE_CPICKER   0
#define LV_USE_CHART     0
#define LV_USE_DROPDOWN  0
#define LV_USE_GAUGE     0
#define LV_USE_IMG       0
#define LV_USE_IMGBTN    0
#define LV_USE_KEYBOARD  0
#define LV_USE_LABEL     1
#define LV_USE_LED       0
#define LV_USE_LINE      0
#define LV_USE_LIST      0
#define LV_USE_MENU      0
#define LV_USE_MSGBOX    0
#define LV_USE_ROLLER    0
#define LV_USE_SLIDER    0
#define LV_USE_SPINBOX   0
#define LV_USE_SPINNER   0
#define LV_USE_SWITCH    0
#define LV_USE_TEXTAREA  0
#define LV_USE_TABLE     0
#define LV_USE_TABVIEW   0
#define LV_USE_TILEVIEW  0
#define LV_USE_WIN       0

/*========================
 * THEME SETTINGS
 *========================*/

#define LV_THEME_DEFAULT_INIT              lv_theme_mono_init
//#define LV_THEME_DEFAULT_COLOR_PRIMARY     lv_palette_main(LV_PALETTE_BLUE)
//#define LV_THEME_DEFAULT_COLOR_SECONDARY   lv_palette_main(LV_PALETTE_CYAN)

/*================
 * ANIMATION
 *================*/

/** Enable animation support (disabled for small systems) */
#define LV_USE_ANIM 0

#endif /*LV_CONF_H*/

#endif /* Enable this file (if 1 above) */

