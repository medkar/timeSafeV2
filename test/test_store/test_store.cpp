#include <unity.h>
#include <IStore.h>

using namespace tsafe;

// Implémentation de test en mémoire du contrat de stockage.
class FakeStore : public IStore {
public:
    bool present = false;
    StoredConfig data;

    LoadStatus load(StoredConfig& out) override {
        if (!present) return LoadStatus::Empty;
        out = data;
        return LoadStatus::Ok;
    }
    bool save(const StoredConfig& cfg) override {
        data = cfg;
        present = true;
        return true;
    }
    bool clear() override {
        present = false;
        data = StoredConfig{};
        return true;
    }
};

void setUp() {}
void tearDown() {}

void test_load_empty_returns_empty_status() {
    FakeStore s;
    StoredConfig out;
    TEST_ASSERT_EQUAL_INT((int)LoadStatus::Empty, (int)s.load(out));
}

void test_save_then_load_roundtrip() {
    FakeStore s;
    StoredConfig cfg;
    cfg.box.armed = true;
    cfg.box.hasDate = true;
    cfg.box.openDate = 123456789;
    cfg.box.hasPassword = true;
    cfg.pwType = PasswordType::Alnum;
    cfg.salt = "SEL";
    cfg.hash = "HASH";
    cfg.pbkdf2Iters = 100000;
    cfg.attempts.failedCount = 2;
    cfg.attempts.lockedUntil = 555;
    cfg.hasLastKnown = true;
    cfg.lastKnownGood = 999;
    cfg.rtcValid = true;
    cfg.wifiSsid = "MaBox";
    cfg.wifiPass = "secret";

    TEST_ASSERT_TRUE(s.save(cfg));

    StoredConfig out;
    TEST_ASSERT_EQUAL_INT((int)LoadStatus::Ok, (int)s.load(out));
    TEST_ASSERT_TRUE(out.box.armed);
    TEST_ASSERT_EQUAL_INT64(123456789, out.box.openDate);
    TEST_ASSERT_EQUAL_INT((int)PasswordType::Alnum, (int)out.pwType);
    TEST_ASSERT_EQUAL_STRING("SEL", out.salt.c_str());
    TEST_ASSERT_EQUAL_STRING("HASH", out.hash.c_str());
    TEST_ASSERT_EQUAL_UINT32(100000, out.pbkdf2Iters);
    TEST_ASSERT_EQUAL_INT(2, out.attempts.failedCount);
    TEST_ASSERT_EQUAL_INT64(999, out.lastKnownGood);
    TEST_ASSERT_TRUE(out.rtcValid);
    TEST_ASSERT_EQUAL_STRING("MaBox", out.wifiSsid.c_str());
}

void test_clear_removes_data() {
    FakeStore s;
    StoredConfig cfg;
    cfg.box.armed = true;
    s.save(cfg);
    TEST_ASSERT_TRUE(s.clear());
    StoredConfig out;
    TEST_ASSERT_EQUAL_INT((int)LoadStatus::Empty, (int)s.load(out));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_load_empty_returns_empty_status);
    RUN_TEST(test_save_then_load_roundtrip);
    RUN_TEST(test_clear_removes_data);
    return UNITY_END();
}
