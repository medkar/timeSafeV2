#pragma once
#include "Theme.h"

namespace tsafe {

// Retourne le thème d'identifiant `id` ; retombe sur le thème par défaut (Coffre)
// si l'id est inconnu.
const Theme& themeById(uint8_t id);

// Identifiant du thème par défaut (Coffre = 0).
uint8_t defaultThemeId();

// Nombre de thèmes disponibles.
size_t themeCount();

// Accès par index pour construire le sélecteur (i < themeCount()).
const Theme& themeByIndex(size_t i);

} // namespace tsafe
