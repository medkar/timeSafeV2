#pragma once
#include <IClockSource.h>
#include <RTClib.h>

namespace tsafe {

// Source de temps RTC (DS3231 / module HW-111) sur un bus I²C dédié (Wire1).
// Durcissement (spec §16.6) : present=false si l'oscillateur a décroché (OSF)
// ou si l'horodatage est hors-plage → on ne fait jamais confiance à un RTC
// non initialisé/déréglé.
class RtcClock : public IClockSource {
public:
    bool begin(int sda, int scl);   // false si non détecté
    TimeSample read() override;
    void setUtc(int64_t epoch);     // cale le RTC depuis une source de confiance
    bool lostPower();
    bool ready() const { return ready_; }

private:
    RTC_DS3231 rtc_;
    bool ready_ = false;
};

} // namespace tsafe
