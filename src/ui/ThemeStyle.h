#pragma once
#include <lvgl.h>
#include <Theme.h>

namespace tsafe {

// Police LVGL (Montserrat accentuée FR) pour une taille logique (14 / 28 / 40).
const lv_font_t* themeFont(int size);

// Applique le fond du thème à un écran (ou conteneur plein).
void themeApplyBg(lv_obj_t* obj, const Theme& t);

// Crée un label thématisé (texte accentué direct, couleur, police, letter-spacing).
lv_obj_t* themeLabel(lv_obj_t* parent, const char* text, uint32_t color,
                     int size, int letterSpace = 0);

} // namespace tsafe
