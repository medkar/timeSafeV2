#include "AppStateMachine.h"

namespace tsafe {

AppStateMachine::AppStateMachine(IStore& store, IClockSource& https, IClockSource& rtc,
                                 ILock& lock, IMonotonicClock& mono, IPasswordHasher& hasher,
                                 IUiView& ui)
    : store_(store), https_(https), rtc_(rtc), lock_(lock), mono_(mono),
      hasher_(hasher), ui_(ui) {}

void AppStateMachine::begin() {
    lock_.forceLock();               // fail-closed : verrouillé par défaut
    lockOpen_ = false;
    const LoadStatus st = store_.load(cfg_);          // sinon -> cfg_ par défaut (non armée)
    loaded_ = (st == LoadStatus::Ok);
    configCorrupted_ = (st == LoadStatus::Corrupted);
    // Un décodage partiel a pu écrire n'importe quoi dans cfg_ : on n'en garde
    // rien, sinon on risquerait une capsule fantôme issue d'octets corrompus.
    if (configCorrupted_) cfg_ = StoredConfig{};
    passwordSatisfied_ = false;
}

void AppStateMachine::applyLock(bool open) {
    if (lockOpen_ == open) return;   // déjà dans l'état voulu : ne rien commander
    if (open) lock_.unlock(); else lock_.forceLock();
    lockOpen_ = open;
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
        c.pwType = ev.isPin ? PasswordType::Pin : PasswordType::Alnum;
    }
    c.attempts = resetAttempts();
    store_.save(c);
    cfg_ = c;
    passwordSatisfied_ = false;
    lock_.forceLock();                  // verrouille immédiatement à l'armement
    lockOpen_ = false;
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
    // Pas de forceLock : la boîte vient d'être ouverte, on la laisse ouverte pour
    // reconfiguration ; le verrouillage se refera à l'armement.
}

void AppStateMachine::applyResult(const PolicyResult& r) {
    // Config persistante illisible : la capsule est irrécupérable. On ouvre la
    // boîte (plutôt que de la condamner) en affichant un message acquittable.
    if (configCorrupted_) {
        applyLock(true);
        ui_.showConfigError();
        return;
    }

    switch (r.state) {
        // Non armée : rien à protéger -> ouverte (sinon impossible d'y déposer
        // quoi que ce soit après un redémarrage).
        case PolicyState::Setup:        applyLock(true);  ui_.showSetup(); break;
        case PolicyState::WaitingSync:  applyLock(false); ui_.showWaitingSync(); break;
        case PolicyState::Alert:        applyLock(false); ui_.showAlert(); break;
        case PolicyState::Countdown:    applyLock(false); ui_.showCountdown(r.remainingSeconds, cfg_.box.openDate); break;
        case PolicyState::AskPassword:  applyLock(false); ui_.showAskPassword(r.lockedOut, r.retryAt - mono_.nowSeconds(),
                                                            cfg_.pwType == PasswordType::Pin); break;
        case PolicyState::Unlock:       applyLock(true);  ui_.showUnlocked(); break;
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
    } else if (ev.type == UiEventType::ThemeChanged) {
        cfg_.themeId = ev.themeId;   // le thème choisi survit au reboot
        store_.save(cfg_);
    } else if (ev.type == UiEventType::ErrorAcknowledged) {
        configCorrupted_ = false;    // « Compris » -> retour à l'accueil normal
    }

    // 2. Décider et agir.
    PolicyResult r = decide(buildInput(passwordSatisfied_));
    applyResult(r);
}

} // namespace tsafe
