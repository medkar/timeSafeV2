#include <unity.h>
#include <ConfigCodec.h>

using namespace tsafe;

void setUp() {}
void tearDown() {}

static StoredConfig sample() {
    StoredConfig c;
    c.box.armed = true;
    c.box.hasDate = true;
    c.box.openDate = 1893456000; // 2030-01-01
    c.box.hasPassword = true;
    c.pwType = PasswordType::Alnum;
    c.salt = std::string("\x00\x01\x02\x7f", 4); // octets bruts, dont un nul
    c.hash = "deadbeef";
    c.pbkdf2Iters = 120000;
    c.attempts.failedCount = 3;
    c.attempts.lockedUntil = 42;
    c.hasLastKnown = true;
    c.lastKnownGood = 1893450000;
    c.rtcValid = true;
    c.wifiSsid = "MaBox";
    c.wifiPass = "p@ss word;=|";
    c.themeId = 3; // Fête
    return c;
}

void test_roundtrip_preserves_all_fields() {
    StoredConfig in = sample();
    std::string blob = ConfigCodec::encode(in);

    StoredConfig out;
    TEST_ASSERT_TRUE(ConfigCodec::decode(blob, out));

    TEST_ASSERT_TRUE(out.box.armed);
    TEST_ASSERT_TRUE(out.box.hasDate);
    TEST_ASSERT_EQUAL_INT64(1893456000, out.box.openDate);
    TEST_ASSERT_TRUE(out.box.hasPassword);
    TEST_ASSERT_EQUAL_INT((int)PasswordType::Alnum, (int)out.pwType);
    TEST_ASSERT_EQUAL_UINT32(4, (uint32_t)out.salt.size()); // octets bruts + nul conservés
    TEST_ASSERT_EQUAL_INT8(0x7f, out.salt[3]);
    TEST_ASSERT_EQUAL_STRING("deadbeef", out.hash.c_str());
    TEST_ASSERT_EQUAL_UINT32(120000, out.pbkdf2Iters);
    TEST_ASSERT_EQUAL_INT(3, out.attempts.failedCount);
    TEST_ASSERT_EQUAL_INT64(42, out.attempts.lockedUntil);
    TEST_ASSERT_TRUE(out.hasLastKnown);
    TEST_ASSERT_EQUAL_INT64(1893450000, out.lastKnownGood);
    TEST_ASSERT_TRUE(out.rtcValid);
    TEST_ASSERT_EQUAL_STRING("MaBox", out.wifiSsid.c_str());
    TEST_ASSERT_EQUAL_STRING("p@ss word;=|", out.wifiPass.c_str());
    TEST_ASSERT_EQUAL_UINT8(3, out.themeId);
}

void test_decode_rejects_wrong_magic() {
    StoredConfig out;
    TEST_ASSERT_FALSE(ConfigCodec::decode("garbage", out));
}

void test_decode_rejects_truncated() {
    StoredConfig in = sample();
    std::string blob = ConfigCodec::encode(in);
    blob.resize(blob.size() - 3); // tronqué
    StoredConfig out;
    TEST_ASSERT_FALSE(ConfigCodec::decode(blob, out));
}

// Rétro-compat : un blob sans l'octet de thème final se décode avec le thème par défaut.
void test_decode_without_theme_defaults_to_zero() {
    StoredConfig in = sample(); // themeId = 3
    std::string blob = ConfigCodec::encode(in);
    blob.resize(blob.size() - 1); // retire l'octet de thème (champ optionnel en fin)
    StoredConfig out;
    TEST_ASSERT_TRUE(ConfigCodec::decode(blob, out));
    TEST_ASSERT_EQUAL_UINT8(0, out.themeId);
    TEST_ASSERT_EQUAL_STRING("MaBox", out.wifiSsid.c_str()); // le reste intact
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_roundtrip_preserves_all_fields);
    RUN_TEST(test_decode_rejects_wrong_magic);
    RUN_TEST(test_decode_rejects_truncated);
    RUN_TEST(test_decode_without_theme_defaults_to_zero);
    return UNITY_END();
}
