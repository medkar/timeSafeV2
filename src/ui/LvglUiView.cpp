#include "LvglUiView.h"
#include "Views.h"
#include "ThemeStyle.h"
#include <ThemeRegistry.h>
#include <Domain.h>
#include <LocalTime.h>
#include <cstdio>
#include <string>
#include <time.h>

namespace tsafe {

static const int kMinYear = 2026;
static const int kMaxYear = 2035;

static int daysInMonth(int y, int m) {
    static const int dm[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2) {
        bool leap = (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
        return leap ? 29 : 28;
    }
    return dm[(m - 1) % 12];
}

// ---------- Widgets utilitaires ----------
static lv_obj_t* container(lv_obj_t* parent) {
    lv_obj_t* o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(o, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    return o;
}

static lv_obj_t* themedButton(lv_obj_t* parent, const Theme& t, const char* text,
                              bool primary, lv_event_cb_t cb, void* user, int width = 300) {
    lv_obj_t* btn = lv_button_create(parent);
    lv_obj_set_width(btn, width);
    lv_obj_set_height(btn, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(btn, 7, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(primary ? t.palette.accent : t.palette.bg), 0);
    lv_obj_set_style_bg_opa(btn, primary ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    if (!primary) {
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(t.palette.line), 0);
    }
    lv_obj_t* l = lv_label_create(btn);
    lv_label_set_text(l, text);
    lv_obj_set_style_text_font(l, themeFont(14), 0);
    lv_obj_set_style_text_color(l, lv_color_hex(primary ? t.palette.bg : t.palette.text), 0);
    lv_obj_center(l);
    if (cb) lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user);
    return btn;
}

// ---------- Accueil : menu de configuration ----------
void LvglUiView::buildMenu() {
    const Theme& t = themeById(themeId_);
    lv_obj_t* scr = lv_screen_active();
    themeApplyBg(scr, t);

    lv_obj_t* col = container(scr);
    lv_obj_set_size(col, 340, LV_SIZE_CONTENT);
    lv_obj_center(col);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(col, 9, 0);

    themeLabel(col, "CONFIGURATION", t.palette.muted, 14, 3);

    char buf[48];
    snprintf(buf, sizeof(buf), "Theme : %s  (changer)", t.name);
    themedButton(col, t, buf, false, onThemeCb, this);

    if (draft_.hasDate) {
        int y, mo, d, h, mi;
        epochToParisLocal(draft_.openDate, y, mo, d, h, mi);
        snprintf(buf, sizeof(buf), "Date : %02d.%02d.%04d  %02d:%02d", d, mo, y, h, mi);
    } else {
        snprintf(buf, sizeof(buf), "Date d'ouverture : —");
    }
    themedButton(col, t, buf, false, onDateCb, this);

    snprintf(buf, sizeof(buf), "Mot de passe : %s", draft_.hasPassword ? "defini" : "—");
    themedButton(col, t, buf, false, onPwCb, this);

    bool ready = draft_.hasDate || draft_.hasPassword;
    if (ready) {
        themedButton(col, t, t.wording.armButton, true, onArmCb, this);
    } else {
        themedButton(col, t, t.wording.armButton, false, nullptr, this);
        themeLabel(col, "Choisis au moins une date ou un mot de passe",
                   t.palette.muted, 14);
    }
}

// ---------- Éditeur de date (rollers) ----------
static lv_obj_t* makeRoller(lv_obj_t* parent, const Theme& t,
                            const std::string& opts, int sel) {
    lv_obj_t* r = lv_roller_create(parent);
    lv_roller_set_options(r, opts.c_str(), LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(r, 3);
    lv_obj_set_style_bg_color(r, lv_color_hex(t.palette.bg), 0);
    lv_obj_set_style_bg_opa(r, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(r, 1, 0);
    lv_obj_set_style_border_color(r, lv_color_hex(t.palette.line), 0);
    lv_obj_set_style_text_color(r, lv_color_hex(t.palette.muted), LV_PART_MAIN);
    lv_obj_set_style_text_font(r, themeFont(14), LV_PART_MAIN);
    lv_obj_set_style_bg_color(r, lv_color_hex(t.palette.accent), LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(r, LV_OPA_20, LV_PART_SELECTED);
    lv_obj_set_style_text_color(r, lv_color_hex(t.palette.accent), LV_PART_SELECTED);
    lv_obj_set_style_text_font(r, themeFont(28), LV_PART_SELECTED);
    if (sel >= 0) lv_roller_set_selected(r, (uint32_t)sel, LV_ANIM_OFF);
    return r;
}

void LvglUiView::buildDateEdit() {
    const Theme& t = themeById(themeId_);
    lv_obj_t* scr = lv_screen_active();
    themeApplyBg(scr, t);

    lv_obj_t* title = themeLabel(scr, "DATE D'OUVERTURE", t.palette.muted, 14, 3);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

    // Valeur de départ : brouillon existant, sinon heure système (Paris).
    int y, mo, d, h, mi;
    if (draft_.hasDate) {
        epochToParisLocal(draft_.openDate, y, mo, d, h, mi);
    } else {
        epochToParisLocal((int64_t)time(nullptr), y, mo, d, h, mi);
    }
    if (y < kMinYear) y = kMinYear;
    if (y > kMaxYear) y = kMaxYear;

    std::string oDay, oMon, oYear, oHour, oMin;
    char b[8];
    for (int v = 1; v <= 31; ++v) { snprintf(b, sizeof(b), "%02d", v); if (v > 1) oDay += "\n"; oDay += b; }
    for (int v = 1; v <= 12; ++v) { snprintf(b, sizeof(b), "%02d", v); if (v > 1) oMon += "\n"; oMon += b; }
    for (int v = kMinYear; v <= kMaxYear; ++v) { snprintf(b, sizeof(b), "%04d", v); if (v > kMinYear) oYear += "\n"; oYear += b; }
    for (int v = 0; v <= 23; ++v) { snprintf(b, sizeof(b), "%02d", v); if (v > 0) oHour += "\n"; oHour += b; }
    for (int v = 0; v <= 59; ++v) { snprintf(b, sizeof(b), "%02d", v); if (v > 0) oMin += "\n"; oMin += b; }

    lv_obj_t* row = container(scr);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 6, 0);
    lv_obj_align(row, LV_ALIGN_CENTER, 0, -6);

    rDay_  = makeRoller(row, t, oDay,  d - 1);
    rMon_  = makeRoller(row, t, oMon,  mo - 1);
    rYear_ = makeRoller(row, t, oYear, y - kMinYear);
    // petit séparateur visuel entre date et heure
    themeLabel(row, "  ", t.palette.muted, 14);
    rHour_ = makeRoller(row, t, oHour, h);
    rMin_  = makeRoller(row, t, oMin,  mi);

    // Calcule les tailles avant les alignements relatifs (rollers = hauteur variable).
    lv_obj_update_layout(scr);

    lv_obj_t* cap = themeLabel(scr, "jour  .  mois  .  annee        heure  :  min",
                               t.palette.muted, 14);
    lv_obj_align_to(cap, row, LV_ALIGN_OUT_TOP_MID, 0, -8);

    lv_obj_t* note = themeLabel(scr, "heure de Paris", t.palette.muted, 14);
    lv_obj_align_to(note, row, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    lv_obj_t* ok = themedButton(scr, t, "Valider", true, onDateOkCb, this, 200);
    lv_obj_align(ok, LV_ALIGN_BOTTOM_MID, 0, -14);

    addBackButton();
}

// ---------- Clavier mot de passe (setup ou déverrouillage) ----------
void LvglUiView::buildPassword(const char* title, bool forUnlock) {
    kbForUnlock_ = forUnlock;
    const Theme& t = themeById(themeId_);
    lv_obj_t* scr = lv_screen_active();
    themeApplyBg(scr, t);

    lv_obj_t* ttl = themeLabel(scr, title, t.palette.muted, 14, 2);
    lv_obj_align(ttl, LV_ALIGN_TOP_MID, 0, 8);

    ta_ = lv_textarea_create(scr);
    lv_textarea_set_one_line(ta_, true);
    lv_textarea_set_password_mode(ta_, true);
    lv_textarea_set_password_bullet(ta_, "*");   // ASCII : présent dans toute police
    lv_textarea_set_max_length(ta_, 32);
    lv_textarea_set_placeholder_text(ta_, forUnlock ? "code" : "choisis un code");
    lv_obj_set_width(ta_, 300);
    lv_obj_align(ta_, LV_ALIGN_TOP_MID, 0, 34);
    lv_obj_set_style_text_font(ta_, themeFont(28), 0);
    lv_obj_set_style_bg_color(ta_, lv_color_hex(t.palette.bg), 0);
    lv_obj_set_style_text_color(ta_, lv_color_hex(t.palette.accent), 0);
    lv_obj_set_style_border_color(ta_, lv_color_hex(t.palette.accent), 0);
    lv_obj_set_style_border_opa(ta_, LV_OPA_50, 0);

    pwStatus_ = themeLabel(scr, "", t.palette.warn, 14);
    lv_obj_align(pwStatus_, LV_ALIGN_TOP_MID, 0, 84);

    kb_ = lv_keyboard_create(scr);
    lv_keyboard_set_mode(kb_, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_keyboard_set_textarea(kb_, ta_);
    lv_obj_add_event_cb(kb_, onKbReadyCb, LV_EVENT_READY, this);
    lv_obj_add_event_cb(kb_, onKbCancelCb, LV_EVENT_CANCEL, this);

    // Retour possible seulement pendant la configuration (pas au déverrouillage).
    if (!forUnlock) addBackButton();
}

// Bouton « Retour » constant en haut à gauche -> revient au menu de config.
// Texte simple (pas de symbole : la police FR n'a pas la flèche) et sans bordure.
void LvglUiView::addBackButton() {
    const Theme& t = themeById(themeId_);
    lv_obj_t* scr = lv_screen_active();
    lv_obj_t* back = lv_button_create(scr);
    lv_obj_set_height(back, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(back, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(back, 0, 0);
    lv_obj_set_style_shadow_width(back, 0, 0);
    lv_obj_set_style_pad_hor(back, 10, 0);
    lv_obj_set_style_pad_ver(back, 8, 0);
    lv_obj_align(back, LV_ALIGN_TOP_LEFT, 6, 6);
    lv_obj_t* l = lv_label_create(back);
    lv_label_set_text(l, "Retour");
    lv_obj_set_style_text_font(l, themeFont(14), 0);
    lv_obj_set_style_text_color(l, lv_color_hex(t.palette.accent), 0);
    lv_obj_center(l);
    lv_obj_add_event_cb(back, onBackCb, LV_EVENT_CLICKED, this);
}

// ---------- Aiguillage des sous-pages Setup ----------
void LvglUiView::buildSetupPage() {
    switch (setupPage_) {
        case SetupPage::Menu:     buildMenu(); break;
        case SetupPage::DateEdit: buildDateEdit(); break;
        case SetupPage::PwEdit:   buildPassword("MOT DE PASSE", /*forUnlock=*/false); break;
    }
}

void LvglUiView::rebuildSetup() {
    lv_obj_clean(lv_screen_active());
    buildSetupPage();
}

// ---------- Actions ----------
void LvglUiView::cycleTheme() {
    themeId_ = (uint8_t)((themeId_ + 1) % themeCount());
    setupPage_ = SetupPage::Menu;
    rebuildSetup();
}
void LvglUiView::backToMenu()   { setupPage_ = SetupPage::Menu;     rebuildSetup(); }
void LvglUiView::openDateEdit() { setupPage_ = SetupPage::DateEdit; rebuildSetup(); }
void LvglUiView::openPwEdit()   { setupPage_ = SetupPage::PwEdit;   rebuildSetup(); }

void LvglUiView::validateDate() {
    int d  = (int)lv_roller_get_selected(rDay_)  + 1;
    int mo = (int)lv_roller_get_selected(rMon_)  + 1;
    int y  = (int)lv_roller_get_selected(rYear_) + kMinYear;
    int h  = (int)lv_roller_get_selected(rHour_);
    int mi = (int)lv_roller_get_selected(rMin_);
    int dim = daysInMonth(y, mo);
    if (d > dim) d = dim;                         // évite 31 février -> clamp
    draft_.hasDate = true;
    draft_.openDate = parisLocalToEpoch(y, mo, d, h, mi);
    setupPage_ = SetupPage::Menu;
    rebuildSetup();
}

void LvglUiView::requestArm() {
    pending_ = UiEvent{};
    pending_.type = UiEventType::ArmRequested;
    pending_.config = draft_;
    pending_.config.armed = true;
    if (draft_.hasPassword) pending_.newPassword = draftPassword_;
    hasPending_ = true;
}

void LvglUiView::requestRearm() {
    pending_ = UiEvent{};
    pending_.type = UiEventType::RearmRequested;
    hasPending_ = true;
}

void LvglUiView::kbReady() {
    const char* txt = lv_textarea_get_text(ta_);
    std::string s = txt ? txt : "";
    if (kbForUnlock_) {
        pending_ = UiEvent{};
        pending_.type = UiEventType::PasswordSubmitted;
        pending_.password = s;
        hasPending_ = true;
        lv_textarea_set_text(ta_, "");            // prêt pour une nouvelle tentative
    } else {
        draft_.hasPassword = !s.empty();
        draftPassword_ = s;
        setupPage_ = SetupPage::Menu;
        rebuildSetup();
    }
}

void LvglUiView::kbCancel() {
    if (kbForUnlock_) {
        lv_textarea_set_text(ta_, "");            // pas d'échappatoire au déverrouillage
    } else {
        backToMenu();                             // retour menu sans définir de mot de passe
    }
}

void LvglUiView::updateAskPassword(bool lockedOut, int64_t retryIn) {
    if (!pwStatus_) return;
    if (lockedOut) {
        char b[56];
        long long s = retryIn > 0 ? (long long)retryIn : 0;
        snprintf(b, sizeof(b), "Trop d'essais — patiente %llds", s);
        lv_label_set_text(pwStatus_, b);
    } else {
        lv_label_set_text(pwStatus_, "");
    }
}

// ---------- Callbacks statiques ----------
void LvglUiView::onThemeCb(lv_event_t* e)       { static_cast<LvglUiView*>(lv_event_get_user_data(e))->cycleTheme(); }
void LvglUiView::onBackCb(lv_event_t* e)        { static_cast<LvglUiView*>(lv_event_get_user_data(e))->backToMenu(); }
void LvglUiView::onDateCb(lv_event_t* e)        { static_cast<LvglUiView*>(lv_event_get_user_data(e))->openDateEdit(); }
void LvglUiView::onPwCb(lv_event_t* e)          { static_cast<LvglUiView*>(lv_event_get_user_data(e))->openPwEdit(); }
void LvglUiView::onArmCb(lv_event_t* e)         { static_cast<LvglUiView*>(lv_event_get_user_data(e))->requestArm(); }
void LvglUiView::onRearmCb(lv_event_t* e)       { static_cast<LvglUiView*>(lv_event_get_user_data(e))->requestRearm(); }
void LvglUiView::onDateOkCb(lv_event_t* e)      { static_cast<LvglUiView*>(lv_event_get_user_data(e))->validateDate(); }
void LvglUiView::onKbReadyCb(lv_event_t* e)     { static_cast<LvglUiView*>(lv_event_get_user_data(e))->kbReady(); }
void LvglUiView::onKbCancelCb(lv_event_t* e)    { static_cast<LvglUiView*>(lv_event_get_user_data(e))->kbCancel(); }

LvglUiView::LvglUiView(uint8_t themeId) : themeId_(themeId) {}

// ---------- Compte à rebours ----------
void LvglUiView::buildCountdown() {
    const Theme& t = themeById(themeId_);
    lv_obj_t* scr = lv_screen_active();
    themeApplyBg(scr, t);

    lv_obj_t* lbl = themeLabel(scr, t.wording.countdownLabel, t.palette.muted, 14, 3);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, -85);

    lv_obj_t* row = container(scr);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 22, 0);
    lv_obj_align(row, LV_ALIGN_CENTER, 0, -5);

    lv_obj_t** outs[4] = { &nD_, &nH_, &nM_, &nS_ };
    const char* units[4] = { "JOURS", "H", "MIN", "SEC" };
    for (int i = 0; i < 4; ++i) {
        lv_obj_t* c = container(row);
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
    snprintf(b, sizeof(b), "%d", d);   lv_label_set_text(nD_, b);
    snprintf(b, sizeof(b), "%02d", h); lv_label_set_text(nH_, b);
    snprintf(b, sizeof(b), "%02d", m); lv_label_set_text(nM_, b);
    snprintf(b, sizeof(b), "%02d", s); lv_label_set_text(nS_, b);
}

// ---------- États pilotés par la machine à états ----------
void LvglUiView::showSetup() {
    if (cur_ == (int)PolicyState::Setup) return;
    setupPage_ = SetupPage::Menu;
    draft_ = BoxConfig{};                 // nouvelle session de configuration
    draftPassword_.clear();
    lv_obj_clean(lv_screen_active());
    buildSetupPage();
    cur_ = (int)PolicyState::Setup;
}

void LvglUiView::showWaitingSync() {
    if (cur_ == (int)PolicyState::WaitingSync) return;
    lv_obj_clean(lv_screen_active());
    viewSync(lv_screen_active(), themeById(themeId_));
    cur_ = (int)PolicyState::WaitingSync;
}

void LvglUiView::showAskPassword(bool lockedOut, int64_t retryInSeconds) {
    if (cur_ != (int)PolicyState::AskPassword) {
        lv_obj_clean(lv_screen_active());
        buildPassword(themeById(themeId_).wording.passwordTitle, /*forUnlock=*/true);
        cur_ = (int)PolicyState::AskPassword;
    }
    updateAskPassword(lockedOut, retryInSeconds);
}

void LvglUiView::showUnlocked() {
    if (cur_ == (int)PolicyState::Unlock) return;
    lv_obj_t* scr = lv_screen_active();
    lv_obj_clean(scr);
    const Theme& t = themeById(themeId_);
    viewUnlocked(scr, t);

    lv_obj_t* hint = themeLabel(scr, "Touche l'ecran pour reconfigurer", t.palette.muted, 14);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -14);

    // Calque transparent plein écran : un clic n'importe où => désarmement.
    // Enfant de l'écran (donc nettoyé au changement d'état, pas de callback résiduel).
    lv_obj_t* tap = lv_button_create(scr);
    lv_obj_remove_style_all(tap);
    lv_obj_set_size(tap, LV_PCT(100), LV_PCT(100));
    lv_obj_add_flag(tap, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(tap, onRearmCb, LV_EVENT_CLICKED, this);

    cur_ = (int)PolicyState::Unlock;
}

void LvglUiView::showAlert() {
    if (cur_ == (int)PolicyState::Alert) return;
    lv_obj_clean(lv_screen_active());
    viewAlert(lv_screen_active(), themeById(themeId_));
    cur_ = (int)PolicyState::Alert;
}

UiEvent LvglUiView::pollEvent() {
    if (hasPending_) { hasPending_ = false; return pending_; }
    return UiEvent{};
}

} // namespace tsafe
