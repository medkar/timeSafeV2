#include "ConfigCodec.h"
#include <cstdint>
#include <cstring>

namespace tsafe {

namespace {
const char kMagic[4] = {'T', 'S', 'F', '1'}; // TimeSafe format v1

void putU8(std::string& s, uint8_t v) { s.push_back((char)v); }
void putU32(std::string& s, uint32_t v) {
    for (int i = 0; i < 4; ++i) s.push_back((char)((v >> (8 * i)) & 0xFF));
}
void putI64(std::string& s, int64_t v) {
    uint64_t u = (uint64_t)v;
    for (int i = 0; i < 8; ++i) s.push_back((char)((u >> (8 * i)) & 0xFF));
}
void putStr(std::string& s, const std::string& v) {
    putU32(s, (uint32_t)v.size());
    s.append(v);
}

struct Reader {
    const std::string& b;
    size_t pos = 0;
    explicit Reader(const std::string& s) : b(s) {}
    bool need(size_t n) const { return pos + n <= b.size(); }
    bool u8(uint8_t& v) { if (!need(1)) return false; v = (uint8_t)b[pos++]; return true; }
    bool u32(uint32_t& v) {
        if (!need(4)) return false;
        v = 0;
        for (int i = 0; i < 4; ++i) v |= (uint32_t)(uint8_t)b[pos++] << (8 * i);
        return true;
    }
    bool i64(int64_t& v) {
        if (!need(8)) return false;
        uint64_t u = 0;
        for (int i = 0; i < 8; ++i) u |= (uint64_t)(uint8_t)b[pos++] << (8 * i);
        v = (int64_t)u;
        return true;
    }
    bool str(std::string& v) {
        uint32_t n;
        if (!u32(n)) return false;
        if (!need(n)) return false;
        v.assign(b, pos, n);
        pos += n;
        return true;
    }
};
} // namespace

std::string ConfigCodec::encode(const StoredConfig& c) {
    std::string s;
    s.append(kMagic, 4);
    putU8(s, c.box.armed ? 1 : 0);
    putU8(s, c.box.hasDate ? 1 : 0);
    putI64(s, c.box.openDate);
    putU8(s, c.box.hasPassword ? 1 : 0);
    putU8(s, (uint8_t)c.pwType);
    putStr(s, c.salt);
    putStr(s, c.hash);
    putU32(s, c.pbkdf2Iters);
    putU32(s, (uint32_t)c.attempts.failedCount);
    putI64(s, c.attempts.lockedUntil);
    putU8(s, c.hasLastKnown ? 1 : 0);
    putI64(s, c.lastKnownGood);
    putU8(s, c.rtcValid ? 1 : 0);
    putStr(s, c.wifiSsid);
    putStr(s, c.wifiPass);
    putU8(s, c.themeId);
    return s;
}

bool ConfigCodec::decode(const std::string& blob, StoredConfig& out) {
    if (blob.size() < 4 || std::memcmp(blob.data(), kMagic, 4) != 0) return false;
    Reader r(blob);
    r.pos = 4;

    uint8_t b8;
    uint32_t u32;
    StoredConfig c;

    if (!r.u8(b8)) return false; c.box.armed = b8;
    if (!r.u8(b8)) return false; c.box.hasDate = b8;
    if (!r.i64(c.box.openDate)) return false;
    if (!r.u8(b8)) return false; c.box.hasPassword = b8;
    if (!r.u8(b8)) return false; c.pwType = (PasswordType)b8;
    if (!r.str(c.salt)) return false;
    if (!r.str(c.hash)) return false;
    if (!r.u32(c.pbkdf2Iters)) return false;
    if (!r.u32(u32)) return false; c.attempts.failedCount = (int)u32;
    if (!r.i64(c.attempts.lockedUntil)) return false;
    if (!r.u8(b8)) return false; c.hasLastKnown = b8;
    if (!r.i64(c.lastKnownGood)) return false;
    if (!r.u8(b8)) return false; c.rtcValid = b8;
    if (!r.str(c.wifiSsid)) return false;
    if (!r.str(c.wifiPass)) return false;

    // Champ optionnel en fin de blob (rétro-compat) : identifiant de thème.
    // Un blob plus ancien sans cet octet se décode avec le thème par défaut (0).
    if (r.pos < blob.size()) {
        uint8_t tid;
        if (!r.u8(tid)) return false;
        c.themeId = tid;
    }

    out = c;
    return true;
}

} // namespace tsafe
