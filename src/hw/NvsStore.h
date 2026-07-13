#pragma once
#include <IStore.h>
#include <ConfigCodec.h>
#include <Preferences.h>
#include <string>

namespace tsafe {

// Stockage persistant réel : sérialise StoredConfig via le codec versionné (testé)
// et l'écrit dans la NVS de l'ESP32 (partition flash, survit à la coupure secteur).
// C'est ce qui permet à la boîte — sans batterie — de retrouver sa capsule armée
// au rebranchement. Le déchiffrement au repos viendra avec Flash Encryption (Plan 4).
class NvsStore : public IStore {
public:
    // À appeler une fois dans setup() avant toute utilisation. false si la NVS
    // n'a pas pu être ouverte.
    bool begin() { return prefs_.begin(kNamespace, /*readOnly=*/false); }

    bool load(StoredConfig& out) override {
        if (!prefs_.isKey(kKey)) return false;    // rien de stocké (sans log d'erreur)
        size_t n = prefs_.getBytesLength(kKey);
        if (n == 0) return false;
        std::string blob;
        blob.resize(n);
        size_t got = prefs_.getBytes(kKey, &blob[0], n);
        if (got != n) return false;               // lecture partielle -> ignore
        return ConfigCodec::decode(blob, out);    // false si blob corrompu/incompatible
    }

    bool save(const StoredConfig& cfg) override {
        const std::string blob = ConfigCodec::encode(cfg);
        return prefs_.putBytes(kKey, blob.data(), blob.size()) == blob.size();
    }

    bool clear() override { return prefs_.remove(kKey); }

private:
    static constexpr const char* kNamespace = "timesafe"; // <= 15 car.
    static constexpr const char* kKey       = "cfg";      // <= 15 car.
    Preferences prefs_;
};

} // namespace tsafe
