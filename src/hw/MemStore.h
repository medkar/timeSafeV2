#pragma once
#include <IStore.h>

namespace tsafe {

// Store en mémoire (le NVS réel viendra ensuite). Config posée par main pour la démo.
class MemStore : public IStore {
public:
    StoredConfig data;
    bool present = false;
    bool load(StoredConfig& o) override { if (!present) return false; o = data; return true; }
    bool save(const StoredConfig& c) override { data = c; present = true; return true; }
    bool clear() override { present = false; data = StoredConfig{}; return true; }
};

} // namespace tsafe
