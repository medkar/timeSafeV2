#pragma once
#include <Domain.h>
#include <string>
#include <cstdint>

namespace tsafe {

// Événement remonté par l'UI vers la machine à états.
enum class UiEventType { None, PasswordSubmitted, ArmRequested, RearmRequested, ThemeChanged };

struct UiEvent {
    UiEventType type = UiEventType::None;
    std::string password;    // si PasswordSubmitted
    BoxConfig config;        // si ArmRequested (date/mdp voulus)
    std::string newPassword; // si ArmRequested et hasPassword
    bool isPin = false;      // si ArmRequested et hasPassword : PIN (true) ou texte (false)
    uint8_t themeId = 0;     // si ThemeChanged : nouveau thème à persister
};

// Vue abstraite. SerialUiView (Plan 2) puis LvglUiView (Plan 3) l'implémentent.
class IUiView {
public:
    virtual ~IUiView() {}
    virtual void showSetup() = 0;
    virtual void showWaitingSync() = 0;
    virtual void showCountdown(int64_t remainingSeconds, int64_t openDate) = 0;
    virtual void showAskPassword(bool lockedOut, int64_t retryInSeconds, bool pin) = 0;
    virtual void showUnlocked() = 0;
    virtual void showAlert() = 0;
    virtual UiEvent pollEvent() = 0; // non bloquant ; None si rien
};

} // namespace tsafe
