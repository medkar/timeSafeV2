#pragma once
#include <IClockSource.h>

namespace tsafe {

// Récupère l'heure via l'en-tête HTTP `Date` d'une requête HTTPS **épinglée**
// (validation de la chaîne jusqu'à GTS Root R1) → heure non-falsifiable depuis
// le LAN. Le paradoxe temps↔certificat est résolu par un plancher = date de build
// (voir setClockFloor dans main). present=false si la validation échoue (fail-closed).
class HttpsTimeClient : public IClockSource {
public:
    explicit HttpsTimeClient(const char* host);
    TimeSample read() override;   // present=false si échec
private:
    const char* host_;
};

} // namespace tsafe
