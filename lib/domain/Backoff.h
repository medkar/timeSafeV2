#pragma once
#include "Domain.h"

namespace tsafe {

struct BackoffParams {
    int64_t baseSeconds = 30; // délai de base
    int maxShift = 6;         // plafond : délai max = base * 2^maxShift
};

// Enregistre un échec : incrémente le compteur et fixe lockedUntil = now + délai.
AttemptState registerFailure(const AttemptState& prev, int64_t now, const BackoffParams& p);

// Remet l'état à zéro (après un déverrouillage réussi).
AttemptState resetAttempts();

// Vrai si l'on est encore dans la fenêtre de temporisation.
bool isLockedOut(const AttemptState& s, int64_t now);

} // namespace tsafe
