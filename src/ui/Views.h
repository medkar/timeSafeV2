#pragma once
#include <lvgl.h>
#include <Theme.h>

namespace tsafe {

// Chaque vue construit son contenu sur `scr` (supposé déjà nettoyé) en lisant
// uniquement les tokens du thème `t` (couleurs + formulations).
void viewCountdown(lv_obj_t* scr, const Theme& t, int d, int h, int m, int s);
void viewSetup(lv_obj_t* scr, const Theme& t);
void viewUnlocked(lv_obj_t* scr, const Theme& t);
void viewAlert(lv_obj_t* scr, const Theme& t);
void viewSync(lv_obj_t* scr, const Theme& t);

} // namespace tsafe
