#include <Arduino.h>
#include <lvgl.h>
#include "hw/lv_port.h"
#include <ThemeRegistry.h>
#include "ui/Views.h"

using namespace tsafe;

// Galerie de démo : parcourir les écrans (moitié droite) et les thèmes
// (moitié gauche) au toucher. Servira de base à l'UI réelle (Plan 3).
static int      scrIdx = 0;
static uint8_t  themeIdx = 0;
static const int SCR_COUNT = 5;
static int64_t  demoSec;

static void build(lv_obj_t* scr) {
    const Theme& t = themeById(themeIdx);
    switch (scrIdx) {
        case 0: {
            int64_t r = demoSec;
            int d = (int)(r / 86400); r %= 86400;
            int h = (int)(r / 3600);  r %= 3600;
            int m = (int)(r / 60);
            int s = (int)(r % 60);
            viewCountdown(scr, t, d, h, m, s);
        } break;
        case 1: viewSetup(scr, t);    break;
        case 2: viewUnlocked(scr, t); break;
        case 3: viewAlert(scr, t);    break;
        case 4: viewSync(scr, t);     break;
    }
    Serial.printf("screen=%d theme=%s\n", scrIdx, t.name);
}

static void rebuild() {
    lv_obj_t* scr = lv_screen_active();
    lv_obj_clean(scr);
    build(scr);
}

static void on_click(lv_event_t* e) {
    (void)e;
    lv_point_t p;
    lv_indev_get_point(lv_indev_active(), &p);
    if (p.x >= 240) scrIdx = (scrIdx + 1) % SCR_COUNT;             // droite : écran suivant
    else            themeIdx = (uint8_t)((themeIdx + 1) % themeCount()); // gauche : thème suivant
    rebuild();
}

static void tick_cb(lv_timer_t* tm) {
    (void)tm;
    if (demoSec > 0) demoSec--;
    if (scrIdx == 0) rebuild();  // rafraîchir le rebours chaque seconde
}

void setup() {
    Serial.begin(115200);
    lvport_init();
    demoSec = (int64_t)142 * 86400 + 6 * 3600 + 23 * 60 + 45;

    lv_obj_t* scr = lv_screen_active();
    lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(scr, on_click, LV_EVENT_CLICKED, nullptr);

    rebuild();
    lv_timer_create(tick_cb, 1000, nullptr);
    Serial.println("Galerie UI prete : gauche=theme, droite=ecran");
}

void loop() {
    lvport_loop();
    delay(5);
}
