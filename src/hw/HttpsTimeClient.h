#pragma once
#include <IClockSource.h>

namespace tsafe {

// Récupère l'heure via l'en-tête HTTP `Date` d'une requête HTTPS.
// ÉTAPE 1 (bring-up) : NON épinglé (setInsecure) — la plomberie d'abord.
// ÉTAPE 2 (à venir)  : épinglage + validation → vraie protection anti-triche.
class HttpsTimeClient : public IClockSource {
public:
    explicit HttpsTimeClient(const char* host);
    TimeSample read() override;   // present=false si échec
private:
    const char* host_;
};

} // namespace tsafe
