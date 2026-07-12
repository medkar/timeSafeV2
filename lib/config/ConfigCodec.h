#pragma once
#include <IStore.h>
#include <string>

namespace tsafe {

// Sérialise StoredConfig en un blob binaire versionné (longueur-préfixée pour
// les chaînes, donc sûr même avec des octets nuls dans salt/hash).
class ConfigCodec {
public:
    static std::string encode(const StoredConfig& c);
    static bool decode(const std::string& blob, StoredConfig& out); // false si invalide
};

} // namespace tsafe
