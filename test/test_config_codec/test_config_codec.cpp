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

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_roundtrip_preserves_all_fields);
    RUN_TEST(test_decode_rejects_wrong_magic);
    RUN_TEST(test_decode_rejects_truncated);
    return UNITY_END();
}
