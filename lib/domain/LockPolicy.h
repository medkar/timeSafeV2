#pragma once
#include "Domain.h"

namespace tsafe {

// Fonction pure : décide l'état de la boîte à partir des faits fournis (spec §6).
// N'accède à aucun matériel. Ne modifie rien.
PolicyResult decide(const PolicyInput& in);

} // namespace tsafe
