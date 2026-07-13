#pragma once
#include <IPasswordHasher.h>

namespace tsafe {

// Hacheur factice (le vrai PBKDF2/mbedTLS viendra avec le mode coffre).
class StubHasher : public IPasswordHasher {
public:
    std::string derive(const std::string& pw, const std::string&, uint32_t) override {
        return std::string("H:") + pw;
    }
    std::string randomSalt(size_t n) override { return std::string(n, 'S'); }
};

} // namespace tsafe
