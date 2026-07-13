#pragma once
#include <lvgl.h>
#include <IUiView.h>

namespace tsafe {

// Implémentation LVGL de IUiView : relie la machine à états aux vues thématisées.
// Ne reconstruit l'écran que lorsque l'état change (anti-scintillement) ; l'accueil
// (Setup) est interactif (changer de thème, armer).
class LvglUiView : public IUiView {
public:
    explicit LvglUiView(uint8_t themeId);

    void showSetup() override;
    void showWaitingSync() override;
    void showCountdown(int64_t remainingSeconds) override;
    void showAskPassword(bool lockedOut, int64_t retryInSeconds) override;
    void showUnlocked() override;
    void showAlert() override;
    UiEvent pollEvent() override;

private:
    void buildCountdown();
    void buildSetup();            // accueil interactif
    void cycleTheme();
    void requestArm();
    static void onThemeCb(lv_event_t* e);
    static void onArmCb(lv_event_t* e);

    uint8_t themeId_;
    int cur_ = -1;                // PolicyState affiché (-1 = aucun)
    UiEvent pending_;
    bool hasPending_ = false;
    lv_obj_t* nD_ = nullptr;
    lv_obj_t* nH_ = nullptr;
    lv_obj_t* nM_ = nullptr;
    lv_obj_t* nS_ = nullptr;
};

} // namespace tsafe
