#pragma once
#include "Domain.h"

namespace tsafe {

// Une lecture de temps issue d'une source (HTTPS épinglé ou RTC).
struct TimeSample {
    bool present = false;
    int64_t epoch = 0; // epoch secondes UTC
};

struct TimeResolveInput {
    TimeSample https;
    TimeSample rtc;
    bool hasLastKnown = false;
    int64_t lastKnownGood = 0;
    int64_t toleranceSeconds = 900; // 15 min par défaut (spec §5)
};

// Croise les sources et applique la règle du minimum + détection d'anomalie.
TimeStatus resolveTime(const TimeResolveInput& in);

} // namespace tsafe
