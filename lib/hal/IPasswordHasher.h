#pragma once
#include <string>
#include <cstdint>
namespace tsafe {
class IPasswordHasher {
public:
    virtual ~IPasswordHasher() {}
    // Dérive un hash à partir d'un mot de passe + sel + itérations.
    virtual std::string derive(const std::string& password,
                               const std::string& salt,
                               uint32_t iterations) = 0;
    // Génère un sel aléatoire de n octets.
    virtual std::string randomSalt(size_t n) = 0;
};
} // namespace tsafe
