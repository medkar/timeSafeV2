#include <Arduino.h>
#include <lvgl.h>
#include "hw/lv_port.h"

// Écran « A sobre » — démo du compte à rebours, tique en direct.
static lv_obj_t* numDays;
static lv_obj_t* numHours;
static lv_obj_t* numMins;
static lv_obj_t* numSecs;
static int64_t remainingSec;

static const uint32_t COL_BG    = 0x0b0f16;
static const uint32_t COL_CYAN  = 0x38bdf8;
static const uint32_t COL_MUTED = 0x64748b;
static const uint32_t COL_FOOT  = 0x475569;

static void make_cell(lv_obj_t* parent, const char* unit, lv_obj_t** numOut) {
    lv_obj_t* cell = lv_obj_create(parent);
    lv_obj_remove_style_all(cell);
    lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(cell, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cell, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cell, 4, 0);

    lv_obj_t* num = lv_label_create(cell);
    lv_label_set_text(num, "00");
    lv_obj_set_style_text_font(num, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(num, lv_color_hex(COL_CYAN), 0);

    lv_obj_t* u = lv_label_create(cell);
    lv_label_set_text(u, unit);
    lv_obj_set_style_text_font(u, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(u, lv_color_hex(COL_MUTED), 0);

    *numOut = num;
}

static void tick_countdown(lv_timer_t* t) {
    (void)t;
    if (remainingSec > 0) remainingSec--;
    int64_t r = remainingSec;
    int d = (int)(r / 86400); r %= 86400;
    int h = (int)(r / 3600);  r %= 3600;
    int m = (int)(r / 60);
    int s = (int)(r % 60);
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", d);  lv_label_set_text(numDays, buf);
    snprintf(buf, sizeof(buf), "%02d", h); lv_label_set_text(numHours, buf);
    snprintf(buf, sizeof(buf), "%02d", m); lv_label_set_text(numMins, buf);
    snprintf(buf, sizeof(buf), "%02d", s); lv_label_set_text(numSecs, buf);
}

static void build_ui() {
    lv_obj_t* scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(COL_BG), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t* lbl = lv_label_create(scr);
    lv_label_set_text(lbl, "OUVERTURE DANS");
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(COL_MUTED), 0);
    lv_obj_set_style_text_letter_space(lbl, 4, 0);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, -85);

    lv_obj_t* row = lv_obj_create(scr);
    lv_obj_remove_style_all(row);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 22, 0);
    lv_obj_align(row, LV_ALIGN_CENTER, 0, -5);

    make_cell(row, "JOURS", &numDays);
    make_cell(row, "H", &numHours);
    make_cell(row, "MIN", &numMins);
    make_cell(row, "SEC", &numSecs);

    lv_obj_t* foot = lv_label_create(scr);
    lv_label_set_text(foot, "CIBLE 2026-12-25  18:00   |   demo LVGL");
    lv_obj_set_style_text_font(foot, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(foot, lv_color_hex(COL_FOOT), 0);
    lv_obj_align(foot, LV_ALIGN_BOTTOM_MID, 0, -12);

    remainingSec = (int64_t)142 * 86400 + 6 * 3600 + 23 * 60 + 45;
    tick_countdown(nullptr);
    lv_timer_create(tick_countdown, 1000, nullptr);
}

void setup() {
    Serial.begin(115200);
    lvport_init();
    build_ui();
    Serial.println("LVGL countdown UI ready");
}

void loop() {
    lvport_loop();
    delay(5);
}
