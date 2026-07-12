#include "ThemeRegistry.h"

namespace tsafe {

// Table des thèmes — c'est ici, et NULLE PART dans le cœur logique, que vit
// l'habillage. Ajouter une occasion = ajouter une entrée + son id.
static const Theme kThemes[] = {
    // --- Coffre (défaut) : « A sobre », sombre / cyan / monospace ---
    {
        ThemeId::Safe, "Coffre",
        { 0x0b0f16, 0xcbd5e1, 0x64748b, 0x38bdf8, 0x38bdf8, 0xf59e0b, 0x94a3b8 },
        ThemeFont::Mono,
        {
            "Ouverture dans",
            "Cible + statut des sources",
            "Armer la boîte",
            "Saisie du code",
            "Déverrouillée",
            "Tu peux ouvrir le couvercle.",
            "Horloge incohérente",
            "Synchronisation…"
        },
        false
    },
    // --- Cadeau : prune & or, typo ronde, paillettes ---
    {
        ThemeId::Gift, "Cadeau",
        { 0x2a0c33, 0xf6e7ef, 0xc69bc4, 0xf6c85a, 0xff9cc0, 0xf6b24a, 0xf6c85a },
        ThemeFont::Round,
        {
            "Ton cadeau s'ouvre dans",
            "Préparé rien que pour toi",
            "Emballer",
            "Code secret",
            "Surprise !",
            "C'est le moment — ouvre vite.",
            "Petit souci d'horloge",
            "On prépare la surprise…"
        },
        true
    },
    // --- Souvenir : bleu nuit, or, serif, ciel étoilé ---
    {
        ThemeId::Memory, "Souvenir",
        { 0x142038, 0xe7dcc8, 0xab9464, 0xf0d79a, 0xcbb072, 0xf0a83a, 0xf0d79a },
        ThemeFont::Serif,
        {
            "Rendez-vous dans",
            "Ouverture prévue",
            "Sceller",
            "Code d'accès",
            "Ouverte",
            "Le moment est venu.",
            "Horloge incohérente",
            "Synchronisation…"
        },
        true
    },
    // --- Fête : noir & or festif, feux d'artifice ---
    {
        ThemeId::NewYear, "Fête",
        { 0x0a0a12, 0xf4ecd0, 0x9a8b62, 0xffd76a, 0xff8fb0, 0xf5a623, 0xffd76a },
        ThemeFont::Sans,
        {
            "Compte à rebours",
            "À minuit pile",
            "Lancer le décompte",
            "Code",
            "C'est l'heure !",
            "Que la fête commence.",
            "Horloge incohérente",
            "Synchronisation…"
        },
        true
    },
    // --- Enfant : couleurs vives, typo ronde, étoiles ---
    {
        ThemeId::Kids, "Enfant",
        { 0x5b2ba0, 0xfdf5ff, 0xcbb7f0, 0xffe24d, 0x57eccf, 0xffab3d, 0xffffff },
        ThemeFont::Round,
        {
            "Plus que…",
            "Tiens bon, c'est bientôt !",
            "C'est parti !",
            "Code secret",
            "Youpi !",
            "Tu peux ouvrir !",
            "Oups, petit souci",
            "Ça arrive…"
        },
        true
    }
};

static const size_t kThemeCount = sizeof(kThemes) / sizeof(kThemes[0]);

const Theme& themeById(uint8_t id) {
    if (id < kThemeCount) return kThemes[id]; // id == index (Safe=0 … Kids=4)
    return kThemes[0];                        // inconnu -> défaut (Coffre)
}

uint8_t defaultThemeId() { return (uint8_t)ThemeId::Safe; }

size_t themeCount() { return kThemeCount; }

const Theme& themeByIndex(size_t i) {
    if (i < kThemeCount) return kThemes[i];
    return kThemes[0];
}

} // namespace tsafe
