#include "TimeCore.h"

namespace tsafe {

// Bornes de plausibilité d'un horodatage courant (revue #2) :
// rejette 0 / non initialisé et les valeurs aberrantes, ce qui ferme aussi
// l'overflow int64 dans les soustractions ci-dessous.
static const int64_t kMinPlausibleEpoch = 1;              // > 0
static const int64_t kMaxPlausibleEpoch = 32503680000LL;  // ~ an 3000

static bool isPlausible(const TimeSample& s) {
    return s.present
        && s.epoch >= kMinPlausibleEpoch
        && s.epoch <= kMaxPlausibleEpoch;
}

TimeStatus resolveTime(const TimeResolveInput& in) {
    TimeStatus out;

    // Rassembler les sources plausibles, calculer min et max.
    int count = 0;
    int64_t mn = 0;
    int64_t mx = 0;
    const TimeSample samples[2] = { in.https, in.rtc };
    for (int i = 0; i < 2; ++i) {
        if (!isPlausible(samples[i])) continue;
        if (count == 0) {
            mn = samples[i].epoch;
            mx = samples[i].epoch;
        } else {
            if (samples[i].epoch < mn) mn = samples[i].epoch;
            if (samples[i].epoch > mx) mx = samples[i].epoch;
        }
        ++count;
    }

    // Aucune source fiable : temps inconnu (fail-closed), sans anomalie.
    if (count == 0) {
        out.trusted = false;
        out.anomaly = false;
        return out;
    }

    // Deux sources qui divergent au-delà de la tolérance : anomalie.
    if (count >= 2 && (mx - mn) > in.toleranceSeconds) {
        out.anomaly = true;
        out.trusted = false;
        return out;
    }

    // Règle du minimum (conservateur : empêche l'ouverture anticipée).
    const int64_t eff = mn;

    // Anti-retour-arrière : un temps nettement inférieur au dernier connu est suspect.
    if (in.hasLastKnown && eff < in.lastKnownGood - in.toleranceSeconds) {
        out.anomaly = true;
        out.trusted = false;
        return out;
    }

    out.trusted = true;
    out.anomaly = false;
    out.effectiveNow = eff;
    return out;
}

} // namespace tsafe
