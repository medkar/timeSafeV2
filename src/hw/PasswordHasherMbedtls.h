#pragma once
#include <IPasswordHasher.h>
#include <string>
#include <cstring>
#include <cstdint>
#include "mbedtls/md.h"
#include "esp_random.h"

namespace tsafe {

// Hacheur réel : PBKDF2-HMAC-SHA256 (sortie 32 octets), sel via RNG matériel.
// Le mot de passe n'est jamais stocké : seul le hash + sel + itérations le sont.
// Même si la flash est lue, il reste à forcer 100 000 itérations par essai.
// PBKDF2 est implémenté "à la main" au-dessus de mbedtls_md HMAC, API stable
// entre mbedTLS 2.x et 3.x (contrairement aux helpers pkcs5 dépréciés en 3.x).
class PasswordHasherMbedtls : public IPasswordHasher {
public:
    std::string derive(const std::string& password,
                       const std::string& salt,
                       uint32_t iterations) override {
        unsigned char out[32];
        pbkdf2Sha256(reinterpret_cast<const unsigned char*>(password.data()), password.size(),
                     reinterpret_cast<const unsigned char*>(salt.data()), salt.size(),
                     iterations ? iterations : 1, out);
        return std::string(reinterpret_cast<const char*>(out), sizeof(out));
    }

    std::string randomSalt(size_t n) override {
        std::string s;
        s.resize(n);
        for (size_t i = 0; i < n; ++i) s[i] = static_cast<char>(esp_random() & 0xFF);
        return s;
    }

    // Auto-test cryptographique (vecteur connu PBKDF2-HMAC-SHA256).
    // P="password", S="salt", c=1, dkLen=32. true si conforme.
    static bool selfTest() {
        PasswordHasherMbedtls h;
        const std::string d = h.derive("password", "salt", 1);
        static const unsigned char kExpected[32] = {
            0x12,0x0f,0xb6,0xcf,0xfc,0xf8,0xb3,0x2c,0x43,0xe7,0x22,0x52,0x56,0xc4,0xf8,0x37,
            0xa8,0x65,0x48,0xc9,0x2c,0xcc,0x35,0x48,0x08,0x05,0x98,0x7c,0xb7,0x0b,0xe1,0x7b
        };
        return d.size() == 32 && memcmp(d.data(), kExpected, 32) == 0;
    }

private:
    // PBKDF2-HMAC-SHA256 avec dkLen fixé à un bloc SHA-256 (32 octets) : un seul
    // bloc T_1 suffit, pas de concaténation multi-blocs à gérer.
    static void pbkdf2Sha256(const unsigned char* pw, size_t pwLen,
                             const unsigned char* salt, size_t saltLen,
                             uint32_t iters, unsigned char out[32]) {
        mbedtls_md_context_t ctx;
        mbedtls_md_init(&ctx);
        mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1); // 1 = HMAC
        mbedtls_md_hmac_starts(&ctx, pw, pwLen);                                 // pose la clé

        // U1 = HMAC(pw, salt || INT32BE(1))
        const unsigned char idx[4] = {0, 0, 0, 1};
        unsigned char U[32], T[32];
        mbedtls_md_hmac_reset(&ctx);
        mbedtls_md_hmac_update(&ctx, salt, saltLen);
        mbedtls_md_hmac_update(&ctx, idx, 4);
        mbedtls_md_hmac_finish(&ctx, U);
        memcpy(T, U, 32);

        // Ui = HMAC(pw, U(i-1)) ; T ^= Ui
        for (uint32_t j = 1; j < iters; ++j) {
            mbedtls_md_hmac_reset(&ctx);
            mbedtls_md_hmac_update(&ctx, U, 32);
            mbedtls_md_hmac_finish(&ctx, U);
            for (int k = 0; k < 32; ++k) T[k] ^= U[k];
        }
        mbedtls_md_free(&ctx);
        memcpy(out, T, 32);
    }
};

} // namespace tsafe
