#include "Backoff.h"

namespace tsafe {

AttemptState registerFailure(const AttemptState& prev, int64_t now, const BackoffParams& p) {
    AttemptState s;
    s.failedCount = prev.failedCount + 1;

    int shift = s.failedCount - 1;
    if (shift > p.maxShift) shift = p.maxShift;
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
