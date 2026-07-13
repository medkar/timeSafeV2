#include "AppStateMachine.h"

namespace tsafe {

AppStateMachine::AppStateMachine(IStore& store, IClockSource& https, IClockSource& rtc,
                                 ILock& lock, IMonotonicClock& mono, IPasswordHasher& hasher,
                                 IUiView& ui)
    : store_(store), https_(https), rtc_(rtc), lock_(lock), mono_(mono),
      hasher_(hasher), ui_(ui) {}

void AppStateMachine::begin() {
    lock_.forceLock();               // fail-closed : verrouillé par défaut
    loaded_ = store_.load(cfg_);     // si rien -> cfg_ par défaut (non armée)
    passwordSatisfied_ = false;
    unlockedThisSession_ = false;
}

PolicyInput AppStateMachine::buildInput(bool passwordSatisfied) {
    PolicyInput in;
    in.config = cfg_.box;

    TimeResolveInput tri;
    tri.https = https_.read();
    tri.rtc = rtc_.read();
    tri.hasLastKnown = cfg_.hasLastKnown;
    tri.lastKnownGood = cfg_.lastKnownGood;
    in.time = resolveTime(tri);

    in.passwordSatisfied = passwordSatisfied;
    in.attempts = cfg_.attempts;
    in.now = mono_.nowSeconds();   // temps monotone pour la fenêtre de lockout (§16.7)
    return in;
}

void AppStateMachine::handlePasswordSubmit(const std::string& candidate) {
    if (!cfg_.box.hasPassword) return;
    // Ignorer si dans la fenêtre de temporisation.
    if (isLockedOut(cfg_.attempts, mono_.nowSeconds())) return;

    const std::string h = hasher_.derive(candidate, cfg_.salt, cfg_.pbkdf2Iters);
    if (h == cfg_.hash) {
        passwordSatisfied_ = true;
        cfg_.attempts = resetAttempts();
        store_.save(cfg_);
    } else {
        BackoffParams p; // défauts spec §9
        cfg_.attempts = registerFailure(cfg_.attempts, mono_.nowSeconds(), p);
        store_.save(cfg_);
    }
}

void AppStateMachine::handleArm(const UiEvent& ev) {
    StoredConfig c = cfg_;              // conserve thème / wifi existants
    c.box = ev.config;                  // date / mot de passe voulus
    c.box.armed = true;
    if (ev.config.hasPassword) {
        c.salt = hasher_.randomSalt(16);
        c.hash = hasher_.derive(ev.newPassword, c.salt, c.pbkdf2Iters);
        c.pwType = PasswordType::Alnum;
    }
    c.attempts = resetAttempts();
    store_.save(c);
    cfg_ = c;
    passwordSatisfied_ = false;
    unlockedThisSession_ = false;
    lock_.forceLock();                  // verrouille immédiatement à l'armement
}

void AppStateMachine::handleRearm() {
    StoredConfig c = cfg_;              // conserve thème / wifi / plancher anti-rollback
    c.box = BoxConfig{};                // désarme : plus de date ni de mot de passe
    c.hash.clear();
    c.salt.clear();
    c.attempts = resetAttempts();
    store_.save(c);                     // persiste l'état désarmé (survit au reboot)
    cfg_ = c;
    passwordSatisfied_ = false;
    unlockedThisSession_ = false;
    // Pas de forceLock : la boîte vient d'être ouverte, on la laisse ouverte pour
    // reconfiguration ; le verrouillage se refera à l'armement.
}

void AppStateMachine::applyResult(const PolicyResult& r) {
    switch (r.state) {
        case PolicyState::Setup:       ui_.showSetup(); break;
        case PolicyState::WaitingSync:  ui_.showWaitingSync(); break;
        case PolicyState::Alert:        ui_.showAlert(); break;
        case PolicyState::Countdown:    ui_.showCountdown(r.remainingSeconds); break;
        case PolicyState::AskPassword:  ui_.showAskPassword(r.lockedOut, r.retryAt - mono_.nowSeconds()); break;
        case PolicyState::Unlock:
            if (!unlockedThisSession_) {
                lock_.unlock();
                unlockedThisSession_ = true;
            }
            ui_.showUnlocked();
            break;
    }
}

void AppStateMachine::tick() {
    // 1. Traiter un éventuel event UI.
    UiEvent ev = ui_.pollEvent();
    if (ev.type == UiEventType::PasswordSubmitted) {
        handlePasswordSubmit(ev.password);
    } else if (ev.type == UiEventType::ArmRequested) {
        handleArm(ev);
    } else if (ev.type == UiEventType::RearmRequested) {
        handleRearm();
    }

    // 2. Décider et agir.
    PolicyResult r = decide(buildInput(passwordSatisfied_));
    applyResult(r);
}

} // namespace tsafe
