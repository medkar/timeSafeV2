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

    // Rassembler les sources plausibles, garder le maximum.
    int count = 0;
    int64_t mx = 0;
    const TimeSample samples[2] = { in.https, in.rtc };
    for (int i = 0; i < 2; ++i) {
        if (!isPlausible(samples[i])) continue;
        if (count == 0 || samples[i].epoch > mx) mx = samples[i].epoch;
        ++count;
    }

    // Aucune source plausible : temps inconnu (fail-closed), sans anomalie.
    if (count == 0) {
        out.trusted = false;
        out.anomaly = false;
        return out;
    }

    // Règle « OU » (maximum) : la boîte peut s'ouvrir dès qu'UNE source atteint la
    // date. Pas de verrou d'anomalie sur divergence : dans le modèle (boîte scellée,
    // RTC interne, pins USB data débranchés, HTTPS épinglé) aucune source ne peut
    // être avancée frauduleusement, donc on privilégie l'ouverture fiable au blocage.
    // Contrepartie assumée : une source qui déraille vers le haut peut ouvrir en avance.
    out.trusted = true;
    out.anomaly = false;
    out.effectiveNow = mx;
    return out;
}

} // namespace tsafe
