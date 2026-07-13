#include "ThemeStyle.h"
#include "fonts/fonts_fr.h"

namespace tsafe {

const lv_font_t* themeFont(int size) {
    if (size >= 40) return &lv_font_fr_40;
    if (size >= 28) return &lv_font_fr_28;
    return &lv_font_fr_14;
}

void themeApplyBg(lv_obj_t* obj, const Theme& t) {
    lv_obj_set_style_bg_color(obj, lv_color_hex(t.palette.bg), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
}

lv_obj_t* themeLabel(lv_obj_t* parent, const char* text, uint32_t color,
                     int size, int letterSpace) {
    lv_obj_t* l = lv_label_create(parent);
    lv_label_set_text(l, text);   // texte accentué direct (la police a les accents)
    lv_obj_set_style_text_color(l, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(l, themeFont(size), 0);
    if (letterSpace) lv_obj_set_style_text_letter_space(l, letterSpace, 0);
    return l;
}

} // namespace tsafe
