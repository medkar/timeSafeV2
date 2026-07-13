#include <Arduino.h>
#include <lvgl.h>
#include "DisplayLGFX.h"
#include "lv_port.h"

static DisplayLGFX gfx;

static const uint32_t H_RES = 480;
static const uint32_t V_RES = 320;

// Buffer de rendu partiel : 40 lignes en RGB565 (2 octets/pixel).
static uint8_t drawbuf[H_RES * 40 * 2];

static void flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    gfx.startWrite();
    gfx.setAddrWindow(area->x1, area->y1, w, h);
    // Cast en lgfx::rgb565_t* : LovyanGFX gère l'ordre des octets correctement
    // (pattern canonique LVGL9 + LovyanGFX). Corrige l'effet "arc-en-ciel".
    gfx.writePixels(reinterpret_cast<lgfx::rgb565_t*>(px_map), w * h);
    gfx.endWrite();
    lv_display_flush_ready(disp);
}

static void touch_read_cb(lv_indev_t* indev, lv_indev_data_t* data) {
    (void)indev;
    int32_t x, y;
    if (gfx.getTouch(&x, &y)) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = x;
        data->point.y = y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static uint32_t tick_cb(void) { return millis(); }

void lvport_init() {
    gfx.init();
    gfx.setRotation(1);        // paysage 480x320
    gfx.setBrightness(200);

    lv_init();
    lv_tick_set_cb(tick_cb);

    lv_display_t* disp = lv_display_create(H_RES, V_RES);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, drawbuf, nullptr, sizeof(drawbuf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_indev_t* indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read_cb);
}

void lvport_loop() {
    lv_timer_handler();
}
