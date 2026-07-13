#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// Config LovyanGFX pour la Waveshare 3.5" Capacitive Touch LCD (320x480) :
// LCD ST7796 en SPI + tactile FT6336U en I2C. Broches = câblage TimeSafe.
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
            c.freq_write = 40000000;
            c.freq_read  = 16000000;
            c.spi_3wire  = false;
            c.use_lock   = true;
            c.dma_channel = 1;
            c.pin_sclk = 18;   // SCLK
            c.pin_mosi = 23;   // MOSI
            c.pin_miso = -1;   // non connecté
            c.pin_dc   = 26;   // LCD_DC
            _bus.config(c);
            _panel.setBus(&_bus);
        }
        {   // --- Panneau ST7796 ---
            auto c = _panel.config();
            c.pin_cs   = 27;   // LCD_CS
            c.pin_rst  = 33;   // LCD_RST
            c.pin_busy = -1;
            c.panel_width  = 320;
            c.panel_height = 480;
            c.offset_x = 0;
            c.offset_y = 0;
            c.offset_rotation = 0;
            c.readable  = false;
            c.invert    = true;   // à basculer si les couleurs sont inversées
            c.rgb_order = false;  // à basculer si rouge/bleu sont permutés
            c.bus_shared = true;
            _panel.config(c);
        }
        {   // --- Rétroéclairage (LCD_BL) ---
            auto c = _light.config();
            c.pin_bl = 14;
            c.invert = false;
            c.freq = 12000;
            c.pwm_channel = 7;
            _light.config(c);
            _panel.setLight(&_light);
        }
        {   // --- Tactile FT6336U (I2C) ---
            auto c = _touch.config();
            c.x_min = 0;   c.x_max = 319;
            c.y_min = 0;   c.y_max = 479;
            c.pin_int = 34;   // INT
            c.pin_rst = 25;   // RST (tactile)
            c.bus_shared = false;
            c.offset_rotation = 0;
            c.i2c_port = 0;
            c.i2c_addr = 0x38;
            c.pin_sda = 21;   // TP_SDA
            c.pin_scl = 22;   // TP_SCL
            c.freq = 400000;
            _touch.config(c);
            _panel.setTouch(&_touch);
        }
        setPanel(&_panel);
    }
};
