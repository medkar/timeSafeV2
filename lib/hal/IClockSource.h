#pragma once
#include <TimeCore.h>   // TimeSample

namespace tsafe {
// Une source de temps (RTC ou HTTPS). read() renvoie un échantillon,
// present=false si indisponible/non fiable.
class IClockSource {
public:
    virtual ~IClockSource() {}
    virtual TimeSample read() = 0;
};
} // namespace tsafe
