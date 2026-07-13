#pragma once
#include <IStore.h>
#include <LockPolicy.h>
#include <TimeCore.h>
#include <Backoff.h>
#include <IClockSource.h>
#include <ILock.h>
#include <IMonotonicClock.h>
#include <IPasswordHasher.h>
#include <IUiView.h>

namespace tsafe {

class AppStateMachine {
public:
    AppStateMachine(IStore& store, IClockSource& https, IClockSource& rtc,
                    ILock& lock, IMonotonicClock& mono, IPasswordHasher& hasher,
                    IUiView& ui);

    void begin();  // verrouille + charge la config
    void tick();   // à appeler en boucle : lit le temps, traite les events UI, décide, agit

private:
    PolicyInput buildInput(bool passwordSatisfied);
    void applyResult(const PolicyResult& r);
    void handlePasswordSubmit(const std::string& candidate);
    void handleArm(const UiEvent& ev);
    void handleRearm();   // désarme après ouverture -> retour à l'accueil

    IStore& store_;
    IClockSource& https_;
    IClockSource& rtc_;
    ILock& lock_;
    IMonotonicClock& mono_;
    IPasswordHasher& hasher_;
    IUiView& ui_;

    StoredConfig cfg_;
    bool loaded_ = false;
    bool passwordSatisfied_ = false;
    bool unlockedThisSession_ = false;
};

} // namespace tsafe
