#pragma once
#include <lvgl.h>
#include <IUiView.h>
#include <Domain.h>
#include <string>
#include <functional>

namespace tsafe {

// Implémentation LVGL de IUiView : relie la machine à états aux vues thématisées.
// Ne reconstruit un écran que lorsque l'état change (anti-scintillement).
// L'accueil (Setup) est un vrai menu de configuration avec sous-pages internes :
//   Menu -> (Éditeur de date | Clavier mot de passe) -> Menu -> ARMER.
class LvglUiView : public IUiView {
public:
    explicit LvglUiView(uint8_t themeId);

    void setTheme(uint8_t id);   // applique un thème (ex. celui relu depuis la NVS au boot)
    void ensureStatusBar();      // crée (une fois) la barre heure+WiFi sur la couche haute
    void updateStatusBar();      // rafraîchit heure + état WiFi

    // Handler appelé quand l'utilisateur valide un WiFi (SSID, mot de passe) :
    // main y persiste les identifiants en NVS et redémarre pour se reconnecter.
    void setWifiConfigHandler(std::function<void(const std::string&, const std::string&)> h);

    void showSetup() override;
    void showWaitingSync() override;
    void showCountdown(int64_t remainingSeconds, int64_t openDate) override;
    void showAskPassword(bool lockedOut, int64_t retryInSeconds, bool pin) override;
    void showUnlocked() override;
    void showConfigError() override;
    UiEvent pollEvent() override;

private:
    enum class SetupPage { Menu, DateEdit, PwChoose, PwEdit };

    // Construction des écrans
    void buildCountdown();
    void rebuildSetup();
    void buildSetupPage();
    void buildMenu();
    void buildDateEdit();
    void buildPwChoose();                                   // choix PIN / mot de passe
    void buildPassword(const char* title, bool forUnlock, bool isPin);
    void buildPinPad(lv_obj_t* scr);                        // pavé numérique 0-9
    void addBackButton(lv_event_cb_t cb = onBackCb);        // « Retour »

    // Configuration WiFi (écran modal accessible pendant le verrouillage)
    void addWifiButton();                                   // bouton « WiFi » sur écrans verrouillés
    void openWifi();
    void closeWifi();
    void buildWifiScan();                                   // scan + liste des réseaux
    void buildWifiPass();                                   // clavier mot de passe WiFi
    void wifiPick(lv_event_t* e);                           // réseau choisi -> saisie mdp
    void wifiPassReady();                                   // valider -> handler (save + reboot)

    // Actions déclenchées par le tactile
    void cycleTheme();
    void backToMenu();
    void openDateEdit();
    void openPwChoose();
    void choosePin(bool pin);                               // choisit le type puis saisie
    void validateDate();
    void requestArm();
    void requestRearm();
    void acknowledgeError();                                // « Compris » sur l'écran d'erreur
    void kbReady();
    void kbCancel();
    void pinKey(lv_event_t* e);                             // touche du pavé numérique
    void updateAskPassword(bool lockedOut, int64_t retryIn);

    // Callbacks statiques -> instance via user_data
    static void onThemeCb(lv_event_t* e);
    static void onBackCb(lv_event_t* e);
    static void onDateCb(lv_event_t* e);
    static void onProtectCb(lv_event_t* e);
    static void onPinChoiceCb(lv_event_t* e);
    static void onTextChoiceCb(lv_event_t* e);
    static void onArmCb(lv_event_t* e);
    static void onRearmCb(lv_event_t* e);
    static void onErrorAckCb(lv_event_t* e);
    static void onDateOkCb(lv_event_t* e);
    static void onKbReadyCb(lv_event_t* e);
    static void onKbCancelCb(lv_event_t* e);
    static void onPinKeyCb(lv_event_t* e);
    static void onWifiBtnCb(lv_event_t* e);
    static void onWifiPickCb(lv_event_t* e);
    static void onWifiPassReadyCb(lv_event_t* e);
    static void onWifiBackCb(lv_event_t* e);
    static void onStatusTimerCb(lv_timer_t* tm);

    uint8_t themeId_;
    int cur_ = -1;                 // PolicyState affiché (-1 = aucun)
    SetupPage setupPage_ = SetupPage::Menu;

    // Brouillon de configuration en cours de saisie
    BoxConfig draft_;
    std::string draftPassword_;

    // Saisie mot de passe en deux temps (définition) : 1re saisie puis confirmation.
    std::string pwFirstEntry_;   // code saisi à la 1re étape
    bool pwConfirming_ = false;  // true = on est à l'étape "retape le code"
    std::string pwError_;        // message à afficher après reconstruction (ex. non-concordance)
    bool pwTypePin_ = false;     // type choisi pour le brouillon : PIN (true) ou texte
    bool curEntryPin_ = false;   // l'écran de saisie courant est un pavé PIN

    // Événement en attente pour la machine à états
    UiEvent pending_;
    bool hasPending_ = false;

    // Compte à rebours
    lv_obj_t* nD_ = nullptr; lv_obj_t* nH_ = nullptr;
    lv_obj_t* nM_ = nullptr; lv_obj_t* nS_ = nullptr;
    lv_obj_t* openLbl_ = nullptr;   // date/heure d'ouverture, sous le rebours

    // Éditeur de date
    lv_obj_t* rDay_ = nullptr;  lv_obj_t* rMon_ = nullptr; lv_obj_t* rYear_ = nullptr;
    lv_obj_t* rHour_ = nullptr; lv_obj_t* rMin_ = nullptr;

    // Saisie mot de passe (setup ou déverrouillage)
    lv_obj_t* kb_ = nullptr; lv_obj_t* ta_ = nullptr; lv_obj_t* pwStatus_ = nullptr;
    bool kbForUnlock_ = false;

    // Configuration WiFi
    bool modalWifi_ = false;     // écran WiFi ouvert : gèle l'affichage piloté par l'état
    std::string wifiSsid_;       // réseau choisi (avant saisie du mot de passe)
    std::function<void(const std::string&, const std::string&)> onWifiConfig_;

    // Barre d'état (heure + WiFi) sur la couche supérieure -> persiste entre écrans.
    lv_obj_t* statusBar_ = nullptr;
    lv_obj_t* statusTime_ = nullptr;
    lv_obj_t* statusWifi_ = nullptr;
    lv_timer_t* statusTimer_ = nullptr;
};

} // namespace tsafe
