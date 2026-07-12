#pragma once
#include <cstdint>
namespace tsafe {
// Temps monotone (ne recule jamais, non réglable) — pour la fenêtre anti-brute-force
// (revue de sécurité §16.7). Unité : secondes depuis le boot.
class IMonotonicClock {
public:
    virtual ~IMonotonicClock() {}
    virtual int64_t nowSeconds() = 0;
};
} // namespace tsafe
