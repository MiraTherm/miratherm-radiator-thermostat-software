/**
 * Configuration file for SSD1306 driver
 * Configured for STM32WB with I2C1
 */

#ifndef __SSD1306_CONF_H__
#define __SSD1306_CONF_H__

/* Microcontroller family */
#define STM32WB

/* Bus configuration - using I2C1 */
#define SSD1306_USE_I2C

/* I2C Configuration */
#define SSD1306_I2C_PORT        hi2c1
#define SSD1306_I2C_ADDR        (0x3C << 1)

/* Display dimensions */
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64

/* Include fonts for potential use */
#define SSD1306_INCLUDE_FONT_6x8
#define SSD1306_INCLUDE_FONT_7x10
#define SSD1306_INCLUDE_FONT_11x18
#define SSD1306_INCLUDE_FONT_16x26

#endif /* __SSD1306_CONF_H__ */
