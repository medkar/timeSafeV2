#include "Backoff.h"

namespace tsafe {

// Plafond dur du décalage, indépendant de BackoffParams::maxShift (revue #3) :
// garantit que 1LL << shift ne touche jamais le bit de signe et borne le délai,
// même si un maxShift aberrant est configuré. 2^40 s ≈ 34 000 ans : largement
// au-delà de tout usage réaliste, donc jamais atteint en pratique.
static const int kHardMaxShift = 40;

AttemptState registerFailure(const AttemptState& prev, int64_t now, const BackoffParams& p) {
    AttemptState s;
    s.failedCount = prev.failedCount + 1;

    int shift = s.failedCount - 1;
    if (shift > p.maxShift) shift = p.maxShift;
    if (shift > kHardMaxShift) shift = kHardMaxShift;
    if (shift < 0) shift = 0;

    const int64_t delay = p.baseSeconds * (static_cast<int64_t>(1) << shift);
    s.lockedUntil = now + delay;
    return s;
}

AttemptState resetAttempts() {
    return AttemptState{};
}

bool isLockedOut(const AttemptState& s, int64_t now) {
    return now < s.lockedUntil;
}

} // namespace tsafe
