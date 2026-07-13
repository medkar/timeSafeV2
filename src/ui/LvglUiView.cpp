#include "LvglUiView.h"
#include "Views.h"
#include "ThemeStyle.h"
#include <ThemeRegistry.h>
#include <Domain.h>
#include <cstdio>

namespace tsafe {

LvglUiView::LvglUiView(uint8_t themeId) : themeId_(themeId) {}

void LvglUiView::buildCountdown() {
    const Theme& t = themeById(themeId_);
    lv_obj_t* scr = lv_screen_active();
    themeApplyBg(scr, t);

    lv_obj_t* lbl = themeLabel(scr, t.wording.countdownLabel, t.palette.muted, 14, 3);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, -85);

    lv_obj_t* row = lv_obj_create(scr);
    lv_obj_remove_style_all(row);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 22, 0);
    lv_obj_align(row, LV_ALIGN_CENTER, 0, -5);

    lv_obj_t** outs[4] = { &nD_, &nH_, &nM_, &nS_ };
    const char* units[4] = { "JOURS", "H", "MIN", "SEC" };
    for (int i = 0; i < 4; ++i) {
        lv_obj_t* c = lv_obj_create(row);
        lv_obj_remove_style_all(c);
        lv_obj_clear_flag(c, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(c, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(c, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(c, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_row(c, 3, 0);
        *outs[i] = themeLabel(c, "0", t.palette.accent, 40);
        themeLabel(c, units[i], t.palette.muted, 14);
    }

    lv_obj_t* foot = themeLabel(scr, t.wording.countdownFoot, t.palette.muted, 14);
    lv_obj_align(foot, LV_ALIGN_BOTTOM_MID, 0, -14);
}

void LvglUiView::showCountdown(int64_t remainingSeconds) {
    if (cur_ != (int)PolicyState::Countdown) {
        lv_obj_clean(lv_screen_active());
        buildCountdown();
        cur_ = (int)PolicyState::Countdown;
    }
    int64_t r = remainingSeconds < 0 ? 0 : remainingSeconds;
    int d = (int)(r / 86400); r %= 86400;
    int h = (int)(r / 3600);  r %= 3600;
    int m = (int)(r / 60);
    int s = (int)(r % 60);
    char b[12];
    snprintf(b, sizeof(b), "%d", d);  lv_label_set_text(nD_, b);
    snprintf(b, sizeof(b), "%02d", h); lv_label_set_text(nH_, b);
    snprintf(b, sizeof(b), "%02d", m); lv_label_set_text(nM_, b);
    snprintf(b, sizeof(b), "%02d", s); lv_label_set_text(nS_, b);
}

void LvglUiView::showSetup() {
    if (cur_ == (int)PolicyState::Setup) return;
    lv_obj_clean(lv_screen_active());
    viewSetup(lv_screen_active(), themeById(themeId_));
    cur_ = (int)PolicyState::Setup;
}

void LvglUiView::showWaitingSync() {
    if (cur_ == (int)PolicyState::WaitingSync) return;
    lv_obj_clean(lv_screen_active());
    viewSync(lv_screen_active(), themeById(themeId_));
    cur_ = (int)PolicyState::WaitingSync;
}

void LvglUiView::showAskPassword(bool, int64_t) {
    if (cur_ == (int)PolicyState::AskPassword) return;
    lv_obj_clean(lv_screen_active());
    viewPin(lv_screen_active(), themeById(themeId_));
    cur_ = (int)PolicyState::AskPassword;
}

void LvglUiView::showUnlocked() {
    if (cur_ == (int)PolicyState::Unlock) return;
    lv_obj_clean(lv_screen_active());
    viewUnlocked(lv_screen_active(), themeById(themeId_));
    cur_ = (int)PolicyState::Unlock;
}

void LvglUiView::showAlert() {
    if (cur_ == (int)PolicyState::Alert) return;
    lv_obj_clean(lv_screen_active());
    viewAlert(lv_screen_active(), themeById(themeId_));
    cur_ = (int)PolicyState::Alert;
}

UiEvent LvglUiView::pollEvent() {
    return UiEvent{};   // pas d'entrée dans cette tranche (capsule sans mdp)
}

} // namespace tsafe
