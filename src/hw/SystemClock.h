#pragma once
#include <IClockSource.h>
#include <time.h>

namespace tsafe {

// Source de temps = horloge système, calée sur l'heure HTTPS épinglée au boot.
// present=false tant qu'une synchro HTTPS réussie n'a pas eu lieu : ainsi, hors
// ligne, c'est le RTC (pile) qui fait foi — pas le simple plancher de build.
class SystemClock : public IClockSource {
public:
    bool synced = false;   // passe à true après une synchro HTTPS réussie

    TimeSample read() override {
        TimeSample s;
        if (!synced) return s;
        time_t now = time(nullptr);
        if (now > 1600000000) {
            s.present = true;
            s.epoch = (int64_t)now;
        }
        return s;
    }
};

} // namespace tsafe
