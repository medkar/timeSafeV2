#pragma once
#include <IStore.h>

namespace tsafe {

// Store en mémoire (le NVS réel viendra ensuite). Config posée par main pour la démo.
class MemStore : public IStore {
public:
    StoredConfig data;
    bool present = false;
    LoadStatus load(StoredConfig& o) override {
        if (!present) return LoadStatus::Empty;
        o = data;
        return LoadStatus::Ok;
    }
    bool save(const StoredConfig& c) override { data = c; present = true; return true; }
    bool clear() override { present = false; data = StoredConfig{}; return true; }
};

} // namespace tsafe
