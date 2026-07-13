#include "ThemeStyle.h"
#include <cstring>

namespace tsafe {

const lv_font_t* themeFont(int size) {
    if (size >= 40) return &lv_font_montserrat_40;
    if (size >= 28) return &lv_font_montserrat_28;
    return &lv_font_montserrat_14;
}

void themeApplyBg(lv_obj_t* obj, const Theme& t) {
    lv_obj_set_style_bg_color(obj, lv_color_hex(t.palette.bg), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
}

void asciiFold(const char* in, char* out, size_t cap) {
    size_t o = 0;
    const unsigned char* p = (const unsigned char*)in;
    while (*p && o + 1 < cap) {
        unsigned char c = *p;
        if (c < 0x80) {                       // ASCII direct
            out[o++] = (char)c;
            ++p;
        } else if (c == 0xC3 && p[1]) {       // Latin-1 (à, é, ç, …)
            unsigned char d = p[1];
            char r = '?';
            switch (d) {
                case 0xA0: case 0xA2: r = 'a'; break;       // à â
                case 0xA7: r = 'c'; break;                   // ç
                case 0xA8: case 0xA9: case 0xAA: case 0xAB: r = 'e'; break; // è é ê ë
                case 0xAE: case 0xAF: r = 'i'; break;       // î ï
                case 0xB4: r = 'o'; break;                   // ô
                case 0xB9: case 0xBB: r = 'u'; break;       // ù û
                case 0x80: case 0x82: r = 'A'; break;       // À Â
                case 0x87: r = 'C'; break;                   // Ç
                case 0x88: case 0x89: case 0x8A: r = 'E'; break; // È É Ê
                default: r = '?'; break;
            }
            out[o++] = r;
            p += 2;
        } else if (c == 0xE2 && p[1] == 0x80 && p[2]) { // ponctuation typographique
            unsigned char d = p[2];
            if (d == 0xA6) {                  // … -> ...
                for (int k = 0; k < 3 && o + 1 < cap; ++k) out[o++] = '.';
            } else if (d == 0x99 || d == 0x98) {
                out[o++] = '\'';              // ’ ‘ -> '
            }
            p += 3;
        } else {                              // multibyte inconnu : on saute
            ++p;
            while ((*p & 0xC0) == 0x80) ++p;  // sauter les octets de continuation
        }
    }
    out[o] = '\0';
}

lv_obj_t* themeLabel(lv_obj_t* parent, const char* text, uint32_t color,
                     int size, int letterSpace) {
    char buf[128];
    asciiFold(text, buf, sizeof(buf));
    lv_obj_t* l = lv_label_create(parent);
    lv_label_set_text(l, buf);
    lv_obj_set_style_text_color(l, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(l, themeFont(size), 0);
    if (letterSpace) lv_obj_set_style_text_letter_space(l, letterSpace, 0);
    return l;
}

} // namespace tsafe
