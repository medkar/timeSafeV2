#pragma once
#include <lvgl.h>
#include <Theme.h>

namespace tsafe {

// Police LVGL pour une taille logique (14 / 28 / 40).
// NB : la *famille* du thème (mono/serif/ronde) est approximée par Montserrat
// pour l'instant — une police par famille viendra en polish.
const lv_font_t* themeFont(int size);

// Applique le fond du thème à un écran (ou conteneur plein).
void themeApplyBg(lv_obj_t* obj, const Theme& t);

// Réduit un UTF-8 français en ASCII (é->e, ç->c, …->...) pour les polices
// Montserrat sans accents. Provisoire : à retirer quand une police accentuée
// sera intégrée. `out` doit faire au moins `cap` octets.
void asciiFold(const char* in, char* out, size_t cap);

// Crée un label thématisé (texte dé-accentué, couleur, police, letter-spacing).
lv_obj_t* themeLabel(lv_obj_t* parent, const char* text, uint32_t color,
                     int size, int letterSpace = 0);

} // namespace tsafe
