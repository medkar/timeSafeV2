#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "board_config.h"

// Config LovyanGFX pour la Waveshare 3.5" Capacitive Touch LCD (320x480) :
// LCD ST7796 en SPI + tactile FT6336U en I2C. Broches = board_config.h.
class DisplayLGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7796  _panel;
    lgfx::Bus_SPI       _bus;
    lgfx::Light_PWM     _light;
    lgfx::Touch_FT5x06  _touch; // compatible FT6336U

public:
    DisplayLGFX() {
        {   // --- Bus SPI ---
            auto c = _bus.config();
            c.spi_host   = VSPI_HOST;
            c.spi_mode   = 0;
            c.freq_write = TS_LCD_FREQ_WRITE;
            c.freq_read  = 16000000;
            c.spi_3wire  = false;
            c.use_lock   = true;
            c.dma_channel = 1;
            c.pin_sclk = TS_LCD_PIN_SCLK;
            c.pin_mosi = TS_LCD_PIN_MOSI;
            c.pin_miso = TS_LCD_PIN_MISO;
            c.pin_dc   = TS_LCD_PIN_DC;
            _bus.config(c);
            _panel.setBus(&_bus);
        }
        {   // --- Panneau ST7796 ---
            auto c = _panel.config();
            c.pin_cs   = TS_LCD_PIN_CS;
            c.pin_rst  = TS_LCD_PIN_RST;
            c.pin_busy = TS_LCD_PIN_BUSY;
            c.panel_width  = TS_LCD_WIDTH;
            c.panel_height = TS_LCD_HEIGHT;
            c.offset_x = 0;
            c.offset_y = 0;
            c.offset_rotation = 0;
            c.readable  = false;
            c.invert    = TS_LCD_INVERT;
            c.rgb_order = TS_LCD_RGB_ORDER;
            c.bus_shared = true;
            _panel.config(c);
        }
        {   // --- Rétroéclairage (LCD_BL) ---
            auto c = _light.config();
            c.pin_bl = TS_LCD_PIN_BL;
            c.invert = false;
            c.freq = 12000;
            c.pwm_channel = 7;
            _light.config(c);
            _panel.setLight(&_light);
        }
        {   // --- Tactile FT6336U (I2C) ---
            auto c = _touch.config();
            c.x_min = 0;   c.x_max = TS_LCD_WIDTH - 1;
            c.y_min = 0;   c.y_max = TS_LCD_HEIGHT - 1;
            c.pin_int = TS_TOUCH_PIN_INT;
            c.pin_rst = TS_TOUCH_PIN_RST;
            c.bus_shared = false;
            c.offset_rotation = 0;
            c.i2c_port = TS_TOUCH_I2C_PORT;
            c.i2c_addr = TS_TOUCH_I2C_ADDR;
            c.pin_sda = TS_TOUCH_PIN_SDA;
            c.pin_scl = TS_TOUCH_PIN_SCL;
            c.freq = TS_TOUCH_FREQ;
            _touch.config(c);
            _panel.setTouch(&_touch);
        }
        setPanel(&_panel);
    }
};
