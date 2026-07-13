#include "Views.h"
#include "ThemeStyle.h"
#include <cstdio>

namespace tsafe {

// Conteneur transparent, sans style ni scroll, taille = contenu.
static lv_obj_t* box(lv_obj_t* parent) {
    lv_obj_t* o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(o, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    return o;
}

// --- Compte à rebours ---
static void cell(lv_obj_t* row, const Theme& t, const char* num, const char* unit) {
    lv_obj_t* c = box(row);
    lv_obj_set_flex_flow(c, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(c, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(c, 3, 0);
    themeLabel(c, num, t.palette.accent, 40);
    themeLabel(c, unit, t.palette.muted, 14);
}

void viewCountdown(lv_obj_t* scr, const Theme& t, int d, int h, int m, int s) {
    themeApplyBg(scr, t);

    lv_obj_t* lbl = themeLabel(scr, t.wording.countdownLabel, t.palette.muted, 14, 3);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, -85);

    lv_obj_t* row = box(scr);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 22, 0);
    lv_obj_align(row, LV_ALIGN_CENTER, 0, -5);

    char b[12];
    snprintf(b, sizeof(b), "%d", d);  cell(row, t, b, "JOURS");
    snprintf(b, sizeof(b), "%02d", h); cell(row, t, b, "H");
    snprintf(b, sizeof(b), "%02d", m); cell(row, t, b, "MIN");
    snprintf(b, sizeof(b), "%02d", s); cell(row, t, b, "SEC");

    lv_obj_t* foot = themeLabel(scr, t.wording.countdownFoot, t.palette.muted, 14);
    lv_obj_align(foot, LV_ALIGN_BOTTOM_MID, 0, -14);
}

// --- Configuration ---
static void row2(lv_obj_t* parent, const Theme& t, const char* left, const char* right) {
    lv_obj_t* r = box(parent);
    lv_obj_set_width(r, 420);
    lv_obj_set_flex_flow(r, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(r, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_ver(r, 8, 0);
    lv_obj_set_style_border_side(r, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(r, 1, 0);
    lv_obj_set_style_border_color(r, lv_color_hex(t.palette.line), 0);
    lv_obj_set_style_border_opa(r, LV_OPA_30, 0);
    themeLabel(r, left, t.palette.text, 14);
    themeLabel(r, right, t.palette.muted, 14);
}

void viewSetup(lv_obj_t* scr, const Theme& t) {
    themeApplyBg(scr, t);

    lv_obj_t* col = box(scr);
    lv_obj_set_width(col, 440);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(col, 3, 0);
    lv_obj_center(col);

    themeLabel(col, "CONFIGURATION", t.palette.muted, 14, 3);
    row2(col, t, "WiFi", "MaBox");
    row2(col, t, "Date d'ouverture", "25.12.2026 18:00");
    row2(col, t, "Mot de passe", "defini");

    lv_obj_t* btn = box(col);
    lv_obj_set_style_bg_color(btn, lv_color_hex(t.palette.accent), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_hor(btn, 22, 0);
    lv_obj_set_style_pad_ver(btn, 10, 0);
    lv_obj_set_style_radius(btn, 6, 0);
    lv_obj_set_style_margin_top(btn, 12, 0);
    themeLabel(btn, t.wording.armButton, t.palette.bg, 14, 1);
}

// --- Ouverture réussie ---
void viewUnlocked(lv_obj_t* scr, const Theme& t) {
    themeApplyBg(scr, t);

    lv_obj_t* ico = lv_label_create(scr);
    lv_label_set_text(ico, LV_SYMBOL_OK);
    lv_obj_set_style_text_font(ico, themeFont(40), 0);
    lv_obj_set_style_text_color(ico, lv_color_hex(t.palette.accent), 0);
    lv_obj_align(ico, LV_ALIGN_CENTER, 0, -55);

    lv_obj_t* title = themeLabel(scr, t.wording.unlockTitle, t.palette.accent, 28, 2);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 5);

    lv_obj_t* sub = themeLabel(scr, t.wording.unlockSub, t.palette.muted, 14);
    lv_obj_align(sub, LV_ALIGN_CENTER, 0, 45);
}

// --- Alerte / anomalie ---
void viewAlert(lv_obj_t* scr, const Theme& t) {
    themeApplyBg(scr, t);

    lv_obj_t* ico = lv_label_create(scr);
    lv_label_set_text(ico, LV_SYMBOL_WARNING);
    lv_obj_set_style_text_font(ico, themeFont(40), 0);
    lv_obj_set_style_text_color(ico, lv_color_hex(t.palette.warn), 0);
    lv_obj_align(ico, LV_ALIGN_CENTER, 0, -55);

    lv_obj_t* title = themeLabel(scr, t.wording.alertTitle, t.palette.warn, 28, 1);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 5);

    lv_obj_t* sub = themeLabel(scr, "Sources de temps incoherentes - verrouillee",
                               t.palette.muted, 14);
    lv_obj_align(sub, LV_ALIGN_CENTER, 0, 45);
}

// --- Attente synchro ---
void viewSync(lv_obj_t* scr, const Theme& t) {
    themeApplyBg(scr, t);

    lv_obj_t* sp = lv_spinner_create(scr);
    lv_obj_set_size(sp, 50, 50);
    lv_obj_align(sp, LV_ALIGN_CENTER, 0, -40);
    lv_obj_set_style_arc_color(sp, lv_color_hex(t.palette.line), LV_PART_MAIN);
    lv_obj_set_style_arc_color(sp, lv_color_hex(t.palette.accent), LV_PART_INDICATOR);
    lv_spinner_set_anim_params(sp, 1000, 60);

    lv_obj_t* title = themeLabel(scr, t.wording.syncTitle, t.palette.muted, 28, 1);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 35);

    lv_obj_t* sub = themeLabel(scr, "Obtention de l'heure de confiance", t.palette.muted, 14);
    lv_obj_align(sub, LV_ALIGN_CENTER, 0, 70);
}

} // namespace tsafe
