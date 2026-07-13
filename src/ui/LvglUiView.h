#pragma once
#include <lvgl.h>
#include <IUiView.h>
#include <Domain.h>
#include <string>

namespace tsafe {

// Implémentation LVGL de IUiView : relie la machine à états aux vues thématisées.
// Ne reconstruit un écran que lorsque l'état change (anti-scintillement).
// L'accueil (Setup) est un vrai menu de configuration avec sous-pages internes :
//   Menu -> (Éditeur de date | Clavier mot de passe) -> Menu -> ARMER.
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
    enum class SetupPage { Menu, DateEdit, PwEdit };

    // Construction des écrans
    void buildCountdown();
    void rebuildSetup();
    void buildSetupPage();
    void buildMenu();
    void buildDateEdit();
    void buildPassword(const char* title, bool forUnlock); // clavier + textarea
    void addBackButton();                                  // « ◀ Retour » -> menu

    // Actions déclenchées par le tactile
    void cycleTheme();
    void backToMenu();
    void openDateEdit();
    void openPwEdit();
    void validateDate();
    void requestArm();
    void requestRearm();
    void kbReady();
    void kbCancel();
    void updateAskPassword(bool lockedOut, int64_t retryIn);

    // Callbacks statiques -> instance via user_data
    static void onThemeCb(lv_event_t* e);
    static void onBackCb(lv_event_t* e);
    static void onDateCb(lv_event_t* e);
    static void onPwCb(lv_event_t* e);
    static void onArmCb(lv_event_t* e);
    static void onRearmCb(lv_event_t* e);
    static void onDateOkCb(lv_event_t* e);
    static void onKbReadyCb(lv_event_t* e);
    static void onKbCancelCb(lv_event_t* e);

    uint8_t themeId_;
    int cur_ = -1;                 // PolicyState affiché (-1 = aucun)
    SetupPage setupPage_ = SetupPage::Menu;

    // Brouillon de configuration en cours de saisie
    BoxConfig draft_;
    std::string draftPassword_;

    // Événement en attente pour la machine à états
    UiEvent pending_;
    bool hasPending_ = false;

    // Compte à rebours
    lv_obj_t* nD_ = nullptr; lv_obj_t* nH_ = nullptr;
    lv_obj_t* nM_ = nullptr; lv_obj_t* nS_ = nullptr;

    // Éditeur de date
    lv_obj_t* rDay_ = nullptr;  lv_obj_t* rMon_ = nullptr; lv_obj_t* rYear_ = nullptr;
    lv_obj_t* rHour_ = nullptr; lv_obj_t* rMin_ = nullptr;

    // Saisie mot de passe (setup ou déverrouillage)
    lv_obj_t* kb_ = nullptr; lv_obj_t* ta_ = nullptr; lv_obj_t* pwStatus_ = nullptr;
    bool kbForUnlock_ = false;
};

} // namespace tsafe
