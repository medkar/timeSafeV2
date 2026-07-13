#pragma once
#include <cstdint>

namespace tsafe {

// Conversion heure locale Europe/Paris <-> epoch UTC, avec passage
// heure d'été/hiver automatique (règle UE : dernier dimanche de mars/octobre).
// Permet à l'utilisateur de saisir l'heure "murale" de Paris, tandis que toute
// la logique de la boîte (HTTPS Date, RTC) travaille en epoch UTC.

// Décalage horaire de Paris pour une date civile donnée : 1 (CET) ou 2 (CEST).
int parisOffsetHours(int year, int month, int day);

// Date/heure locale Paris -> epoch (secondes UTC).
int64_t parisLocalToEpoch(int year, int month, int day, int hour, int minute);

// Epoch UTC -> date/heure locale Paris (pour préremplir/afficher).
void epochToParisLocal(int64_t epoch, int& year, int& month, int& day,
                       int& hour, int& minute);

} // namespace tsafe
