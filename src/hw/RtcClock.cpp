#include "RtcClock.h"
#include <Wire.h>

namespace tsafe {

static const int64_t kMinEpoch = 1600000000LL;   // ~2020-09
static const int64_t kMaxEpoch = 32503680000LL;  // ~an 3000

bool RtcClock::begin(int sda, int scl) {
    // setPins AVANT begin : les pins persistent même si RTClib rappelle
    // Wire1.begin() sans arguments (sinon il repasserait sur les pins par défaut).
    Wire1.setPins(sda, scl);
    Wire1.begin();
    ready_ = rtc_.begin(&Wire1);
    return ready_;
}

bool RtcClock::lostPower() {
    return ready_ ? rtc_.lostPower() : true;   // OSF : le RTC a perdu l'heure
}

TimeSample RtcClock::read() {
    TimeSample s;
    if (!ready_ || rtc_.lostPower()) return s;          // non initialisé / OSF -> absent
    int64_t epoch = (int64_t)rtc_.now().unixtime();
    if (epoch < kMinEpoch || epoch > kMaxEpoch) return s;  // aberrant -> absent
    s.present = true;
    s.epoch = epoch;
    return s;
}

void RtcClock::setUtc(int64_t epoch) {
    if (ready_) rtc_.adjust(DateTime((uint32_t)epoch));  // efface aussi le flag OSF
}

} // namespace tsafe
