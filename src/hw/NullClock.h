#pragma once
#include <IClockSource.h>

namespace tsafe {

// Source de temps absente (emplacement du RTC tant qu'il n'est pas câblé).
class NullClock : public IClockSource {
public:
    TimeSample read() override { return TimeSample{}; }
};

} // namespace tsafe
