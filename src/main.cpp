#include <Arduino.h>
#include <lvgl.h>
#include "hw/lv_port.h"
#include "hw/ServoLock.h"
#include <ThemeRegistry.h>
#include "ui/ThemeStyle.h"

using namespace tsafe;

// Test servo : tap gauche = verrouiller, tap droite = déverrouiller.
// Angles par défaut à ajuster selon ta mécanique.
static ServoLock lock(13, 0, 90);   // pin 13, verrou=0°, ouvert=90°
static bool locked = true;

static void render() {
    lv_obj_t* scr = lv_screen_active();
    lv_obj_clean(scr);
    const Theme& t = themeById(0); // Coffre
    themeApplyBg(scr, t);

    lv_obj_t* title = themeLabel(scr, locked ? "VERROUILLÉ" : "DÉVERROUILLÉ",
                                 t.palette.accent, 40, 1);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -40);

    char buf[24];
    snprintf(buf, sizeof(buf), "angle : %d°", lock.currentAngle());
    lv_obj_t* ang = themeLabel(scr, buf, t.palette.muted, 28);
    lv_obj_align(ang, LV_ALIGN_CENTER, 0, 20);

    lv_obj_t* hint = themeLabel(scr, "< verrouiller          déverrouiller >",
                                t.palette.muted, 14, 1);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -16);
}

static void on_click(lv_event_t* e) {
    (void)e;
    lv_point_t p;
    lv_indev_get_point(lv_indev_active(), &p);
    if (p.x >= 240) { lock.unlock();   locked = false; }
    else            { lock.forceLock(); locked = true; }
    Serial.printf("%s angle=%d\n", locked ? "LOCK" : "UNLOCK", lock.currentAngle());
    render();
}

void setup() {
    Serial.begin(115200);
    lvport_init();
    lock.begin();               // attache + force le verrou (0°)
    Serial.println("Servo test pret");

    lv_obj_t* scr = lv_screen_active();
    lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(scr, on_click, LV_EVENT_CLICKED, nullptr);
    render();
}

void loop() {
    lvport_loop();
    delay(5);
}
