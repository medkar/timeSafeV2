#pragma once
// =============================================================================
//  board_config.h — TimeSafe hardware map (compile-time)
//
//  EDIT THIS FILE to match your wiring. Every pin and the key panel parameters
//  live here so the rest of the firmware stays hardware-agnostic. (A few panel
//  internals — rotation offset, freq_read… — remain in src/hw/DisplayLGFX.h.)
//
//  As-built target: ESP32-WROOM-32 (PlatformIO board `esp32dev`)
//    - LCD    : Waveshare 3.5" ST7796 (SPI)
//    - Touch  : FT6336U (I2C, shares nothing with the LCD bus)
//    - RTC    : DS3231 on a dedicated I2C bus (Wire1)
//    - Lock   : SG90-type servo
//  See docs/architecture.md ("Porting") to adapt another display or ESP32 variant.
// =============================================================================

// ---- LCD (ST7796, SPI) ------------------------------------------------------
#define TS_LCD_FREQ_WRITE   40000000
#define TS_LCD_PIN_SCLK     18
#define TS_LCD_PIN_MOSI     23
#define TS_LCD_PIN_MISO     -1      // not connected
#define TS_LCD_PIN_DC       26
#define TS_LCD_PIN_CS       27
#define TS_LCD_PIN_RST      33
#define TS_LCD_PIN_BUSY     -1      // not connected
#define TS_LCD_WIDTH        320     // panel native size (portrait)
#define TS_LCD_HEIGHT       480
#define TS_LCD_INVERT       true    // flip if colours look inverted
#define TS_LCD_RGB_ORDER    false   // flip if red/blue are swapped

// ---- Backlight (PWM) --------------------------------------------------------
#define TS_LCD_PIN_BL       14

// ---- Touch (FT6336U, I2C) ---------------------------------------------------
#define TS_TOUCH_I2C_PORT   0
#define TS_TOUCH_I2C_ADDR   0x38
#define TS_TOUCH_FREQ       400000
#define TS_TOUCH_PIN_SDA    21
#define TS_TOUCH_PIN_SCL    22
#define TS_TOUCH_PIN_INT    34
#define TS_TOUCH_PIN_RST    25

// ---- RTC (DS3231, dedicated I2C = Wire1) ------------------------------------
#define TS_RTC_PIN_SDA      32
#define TS_RTC_PIN_SCL      19

// ---- Servo lock -------------------------------------------------------------
#define TS_SERVO_PIN            13
#define TS_SERVO_ANGLE_LOCKED    0
#define TS_SERVO_ANGLE_UNLOCKED  90
