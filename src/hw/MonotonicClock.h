#pragma once
#include <IMonotonicClock.h>
#include <esp_timer.h>

namespace tsafe {

// Temps monotone (esp_timer) — pour la fenêtre anti-brute-force (§16.7).
class MonotonicClock : public IMonotonicClock {
public:
    int64_t nowSeconds() override { return esp_timer_get_time() / 1000000LL; }
};

} // namespace tsafe
