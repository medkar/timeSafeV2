#pragma once
#include <cstdint>
#include <cstddef>

namespace tsafe {

// Identifiant persistant d'un thème (stocké tel quel dans StoredConfig::themeId).
enum class ThemeId : uint8_t {
    Safe    = 0, // Coffre (défaut)
    Gift    = 1, // Cadeau
    Memory  = 2, // Souvenir
    NewYear = 3, // Fête
    Kids    = 4  // Enfant
};

// Famille de police logique — mappée à une vraie police LVGL au Plan 3.
enum class ThemeFont : uint8_t { Mono, Round, Serif, Sans };

// Palette d'un thème. Couleurs en 0xRRGGBB (les dégradés/alphas sont gérés côté UI).
struct ThemePalette {
    uint32_t bg;
    uint32_t text;
    uint32_t muted;
    uint32_t accent;   // accent principal (chiffres du rebours, etc.)
    uint32_t accent2;  // accent secondaire
    uint32_t warn;     // avertissement sémantique (reste fort sur tous les thèmes)
    uint32_t line;     // filets / bordures
};

// Formulations propres au thème (le ton change avec l'occasion).
struct ThemeWording {
    const char* countdownLabel; // ex. "Ouverture dans" / "Ton cadeau s'ouvre dans"
    const char* countdownFoot;  // pied de l'écran rebours
    const char* armButton;      // ex. "Armer la boîte" / "Emballer"
    const char* passwordTitle;  // ex. "Saisie du code" / "Code secret"
    const char* unlockTitle;    // ex. "Déverrouillée" / "Surprise !"
    const char* unlockSub;      // sous-titre de l'écran ouvert
    const char* alertTitle;     // ex. "Horloge incohérente" / "Petit souci d'horloge"
    const char* syncTitle;      // ex. "Synchronisation…" / "On prépare la surprise…"
};

struct Theme {
    ThemeId id;
    const char* name;   // libellé affiché dans le sélecteur ("Coffre", "Cadeau"…)
    ThemePalette palette;
    ThemeFont font;
    ThemeWording wording;
    bool sparkles;      // motif décoratif (paillettes/étoiles)
};

} // namespace tsafe
