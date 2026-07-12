#include "LockPolicy.h"

namespace tsafe {

PolicyResult decide(const PolicyInput& in) {
    PolicyResult r;

    // 1. Non armée -> configuration.
    if (!in.config.armed) {
        r.state = PolicyState::Setup;
        return r;
    }

    // 2. Anomalie de temps -> alerte, verrouillé (fail-closed).
    if (in.time.anomaly) {
        r.state = PolicyState::Alert;
        return r;
    }

    // 3. Gate « date » (uniquement si une date est requise).
    if (in.config.hasDate) {
        if (!in.time.trusted) {
            r.state = PolicyState::WaitingSync;
            return r;
        }
        if (in.time.effectiveNow < in.config.openDate) {
            r.state = PolicyState::Countdown;
            r.remainingSeconds = in.config.openDate - in.time.effectiveNow;
            return r;
        }
    }

    // À ce stade : pas de date, ou date atteinte.

    // 4. Gate « mot de passe » (uniquement si requis et pas encore satisfait).
    if (in.config.hasPassword && !in.passwordSatisfied) {
        r.state = PolicyState::AskPassword;
        r.lockedOut = in.now < in.attempts.lockedUntil;
        r.retryAt = in.attempts.lockedUntil;
        return r;
    }

    // 5. Toutes les conditions remplies -> ouvrir.
    r.state = PolicyState::Unlock;
    return r;
}

} // namespace tsafe
