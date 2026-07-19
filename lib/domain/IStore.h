#pragma once
#include "Domain.h"
#include <string>

namespace tsafe {

enum class PasswordType {
    Pin,   // code numérique
    Alnum  // mot de passe alphanumérique
};

// Ensemble complet des données persistantes (spec §7).
struct StoredConfig {
    BoxConfig box;
    uint8_t themeId = 0;             // thème d'affichage choisi (0 = Coffre) ; interprété côté UI
    PasswordType pwType = PasswordType::Pin;
    std::string salt;                // sel aléatoire (octets)
    std::string hash;                // sortie PBKDF2 (octets)
    uint32_t pbkdf2Iters = 1;      // hash salé instantané (le stretching lourd est superflu : backoff + Flash Encryption font la sécurité). Augmentable si besoin.
    AttemptState attempts;
    bool hasLastKnown = false;
    int64_t lastKnownGood = 0;
    bool rtcValid = false;
    std::string wifiSsid;
    std::string wifiPass;
};

// Issue d'une lecture de configuration. Distinguer « vierge » de « illisible »
// est essentiel : une boîte vierge s'ouvre normalement, tandis qu'une config
// corrompue doit prévenir l'utilisateur (la capsule a été perdue).
enum class LoadStatus {
    Ok,         // config lue et valide
    Empty,      // rien de stocké (boîte vierge)
    Corrupted   // données présentes mais illisibles ou incompatibles
};

// Contrat de stockage. Implémenté en NVS chiffré sur ESP32 (Plan 2),
// et en mémoire (FakeStore) dans les tests.
class IStore {
public:
    virtual ~IStore() {}
    virtual LoadStatus load(StoredConfig& out) = 0;
    virtual bool save(const StoredConfig& cfg) = 0;
    virtual bool clear() = 0;
};

} // namespace tsafe
