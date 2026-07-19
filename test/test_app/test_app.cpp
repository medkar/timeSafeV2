#include <unity.h>
#include <AppStateMachine.h>
#include <vector>

using namespace tsafe;

// --- Fakes ---
class FakeClock : public IClockSource {
public:
    TimeSample sample;
    TimeSample read() override { return sample; }
};
class FakeLock : public ILock {
public:
    int lockCalls = 0, unlockCalls = 0;
    void forceLock() override { ++lockCalls; }
    void unlock() override { ++unlockCalls; }
};
class FakeMono : public IMonotonicClock {
public:
    int64_t t = 0;
    int64_t nowSeconds() override { return t; }
};
class FakeHasher : public IPasswordHasher {
public:
    std::string derive(const std::string& pw, const std::string&, uint32_t) override {
        return "H:" + pw; // hash factice déterministe
    }
    std::string randomSalt(size_t n) override { return std::string(n, 'S'); }
};
class FakeStore : public IStore {
public:
    bool present = false; StoredConfig data;
    bool load(StoredConfig& o) override { if (!present) return false; o = data; return true; }
    bool save(const StoredConfig& c) override { data = c; present = true; return true; }
    bool clear() override { present = false; data = StoredConfig{}; return true; }
};
class FakeUi : public IUiView {
public:
    PolicyState lastShown = PolicyState::Setup;
    int64_t lastRemaining = -1; bool lastLockedOut = false; bool lastPin = false;
    std::vector<UiEvent> queue; size_t idx = 0;
    void showSetup() override { lastShown = PolicyState::Setup; }
    void showWaitingSync() override { lastShown = PolicyState::WaitingSync; }
    void showCountdown(int64_t r, int64_t) override { lastShown = PolicyState::Countdown; lastRemaining = r; }
    void showAskPassword(bool lo, int64_t, bool pin) override { lastShown = PolicyState::AskPassword; lastLockedOut = lo; lastPin = pin; }
    void showUnlocked() override { lastShown = PolicyState::Unlock; }
    void showAlert() override { lastShown = PolicyState::Alert; }
    UiEvent pollEvent() override {
        if (idx < queue.size()) return queue[idx++];
        return UiEvent{};
    }
};

// Fixture
struct Fixture {
    FakeClock https, rtc;
    FakeLock lock;
    FakeMono mono;
    FakeHasher hasher;
    FakeStore store;
    FakeUi ui;
    AppStateMachine* app = nullptr;
    void build() {
        app = new AppStateMachine(store, https, rtc, lock, mono, hasher, ui);
    }
    ~Fixture() { delete app; }
};

void setUp() {}
void tearDown() {}

static StoredConfig armedDate(int64_t openDate) {
    StoredConfig c; c.box.armed = true; c.box.hasDate = true; c.box.openDate = openDate;
    return c;
}
static StoredConfig armedPassword(const std::string& hash) {
    StoredConfig c; c.box.armed = true; c.box.hasPassword = true; c.hash = hash; c.pwType = PasswordType::Alnum;
    return c;
}

void test_begin_forces_lock() {
    Fixture f; f.build();
    f.app->begin();
    TEST_ASSERT_EQUAL_INT(1, f.lock.lockCalls);
    TEST_ASSERT_EQUAL_INT(0, f.lock.unlockCalls);
}

void test_date_not_reached_shows_countdown_stays_locked() {
    Fixture f;
    f.store.present = true; f.store.data = armedDate(5000);
    f.https.sample = {true, 1000};
    f.build(); f.app->begin();
    f.app->tick();
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Countdown, (int)f.ui.lastShown);
    TEST_ASSERT_EQUAL_INT64(4000, f.ui.lastRemaining);
    TEST_ASSERT_EQUAL_INT(0, f.lock.unlockCalls);
}

void test_date_reached_unlocks() {
    Fixture f;
    f.store.present = true; f.store.data = armedDate(5000);
    f.https.sample = {true, 6000};
    f.build(); f.app->begin();
    f.app->tick();
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Unlock, (int)f.ui.lastShown);
    TEST_ASSERT_EQUAL_INT(1, f.lock.unlockCalls);
}

void test_or_rule_one_source_reached_opens() {
    // Règle OU : le HTTPS n'a pas atteint la date mais le RTC oui -> la boîte ouvre.
    Fixture f;
    f.store.present = true; f.store.data = armedDate(5000);
    f.https.sample = {true, 1000};   // pas encore atteint
    f.rtc.sample   = {true, 6000};   // atteint -> ouvre (maximum)
    f.build(); f.app->begin();
    f.app->tick();
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Unlock, (int)f.ui.lastShown);
    TEST_ASSERT_EQUAL_INT(1, f.lock.unlockCalls);
}

void test_correct_password_unlocks() {
    Fixture f;
    f.store.present = true; f.store.data = armedPassword("H:secret");
    f.build(); f.app->begin();
    UiEvent e; e.type = UiEventType::PasswordSubmitted; e.password = "secret";
    f.ui.queue.push_back(e);
    f.app->tick();
    TEST_ASSERT_EQUAL_INT(1, f.lock.unlockCalls);
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Unlock, (int)f.ui.lastShown);
}

void test_wrong_password_registers_backoff_and_persists() {
    Fixture f;
    f.store.present = true; f.store.data = armedPassword("H:secret");
    f.mono.t = 100;
    f.build(); f.app->begin();
    UiEvent e; e.type = UiEventType::PasswordSubmitted; e.password = "nope";
    f.ui.queue.push_back(e);
    f.app->tick();
    TEST_ASSERT_EQUAL_INT(0, f.lock.unlockCalls);
    // le compteur d'échec doit être persisté (survit au reboot)
    StoredConfig saved; TEST_ASSERT_TRUE(f.store.load(saved));
    TEST_ASSERT_EQUAL_INT(1, saved.attempts.failedCount);
    TEST_ASSERT_TRUE(saved.attempts.lockedUntil > 100);
}

void test_locked_out_ignores_password_attempt() {
    Fixture f;
    StoredConfig c = armedPassword("H:secret");
    c.attempts.failedCount = 1; c.attempts.lockedUntil = 500;
    f.store.present = true; f.store.data = c;
    f.mono.t = 100; // 100 < 500 -> verrouillé
    f.build(); f.app->begin();
    UiEvent e; e.type = UiEventType::PasswordSubmitted; e.password = "secret";
    f.ui.queue.push_back(e);
    f.app->tick();
    TEST_ASSERT_EQUAL_INT(0, f.lock.unlockCalls); // tentative ignorée pendant le lockout
    TEST_ASSERT_EQUAL_INT((int)PolicyState::AskPassword, (int)f.ui.lastShown);
    TEST_ASSERT_TRUE(f.ui.lastLockedOut);
}

void test_unarmed_box_opens_the_lock() {
    // Une boîte non armée n'a rien à protéger : le verrou doit s'ouvrir
    // (sinon impossible d'y déposer quoi que ce soit après un redémarrage).
    Fixture f; f.build();
    f.app->begin();                      // fail-closed : verrouille au démarrage
    TEST_ASSERT_EQUAL_INT(1, f.lock.lockCalls);
    f.app->tick();
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Setup, (int)f.ui.lastShown);
    TEST_ASSERT_EQUAL_INT(1, f.lock.unlockCalls);
}

void test_lock_is_driven_only_on_state_change() {
    // ServoLock::writeAngle() bloque 500 ms : re-piloter le verrou à chaque tick
    // (2x/s) figerait l'UI. Il ne doit être commandé que sur changement d'état.
    Fixture f; f.build();
    f.app->begin();
    f.app->tick(); f.app->tick(); f.app->tick();
    TEST_ASSERT_EQUAL_INT(1, f.lock.unlockCalls);
}

void test_arm_request_arms_capsule() {
    Fixture f;
    f.build(); f.app->begin();               // démarre non armée
    UiEvent e; e.type = UiEventType::ArmRequested;
    e.config.armed = true; e.config.hasDate = true; e.config.openDate = 5000;
    e.config.hasPassword = false;
    f.ui.queue.push_back(e);
    f.https.sample = {true, 1000};           // now(1000) < openDate(5000)
    f.app->tick();
    StoredConfig saved;
    TEST_ASSERT_TRUE(f.store.load(saved));    // config persistée
    TEST_ASSERT_TRUE(saved.box.armed);
    TEST_ASSERT_EQUAL_INT64(5000, saved.box.openDate);
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Countdown, (int)f.ui.lastShown);
    TEST_ASSERT_EQUAL_INT(2, f.lock.lockCalls); // begin + armement
}

void test_rearm_after_unlock_returns_to_setup() {
    Fixture f;
    f.store.present = true; f.store.data = armedDate(5000);
    f.https.sample = {true, 6000};          // date atteinte -> ouverture
    f.build(); f.app->begin();
    f.app->tick();
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Unlock, (int)f.ui.lastShown);

    // Un clic sur l'écran d'ouverture -> désarmement.
    UiEvent e; e.type = UiEventType::RearmRequested;
    f.ui.queue.push_back(e);
    f.app->tick();
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Setup, (int)f.ui.lastShown);
    StoredConfig saved; TEST_ASSERT_TRUE(f.store.load(saved));
    TEST_ASSERT_FALSE(saved.box.armed);     // config désarmée et persistée
    TEST_ASSERT_FALSE(saved.box.hasDate);
}

void test_arm_pin_persists_pwtype_pin() {
    Fixture f; f.build(); f.app->begin();
    UiEvent e; e.type = UiEventType::ArmRequested;
    e.config.armed = true; e.config.hasPassword = true;
    e.newPassword = "123456"; e.isPin = true;
    f.ui.queue.push_back(e);
    f.app->tick();
    StoredConfig saved; TEST_ASSERT_TRUE(f.store.load(saved));
    TEST_ASSERT_TRUE(saved.box.hasPassword);
    TEST_ASSERT_EQUAL_INT((int)PasswordType::Pin, (int)saved.pwType);
    // l'écran de déverrouillage doit indiquer le mode PIN
    TEST_ASSERT_EQUAL_INT((int)PolicyState::AskPassword, (int)f.ui.lastShown);
    TEST_ASSERT_TRUE(f.ui.lastPin);
}

void test_theme_change_persists() {
    Fixture f; f.build(); f.app->begin();
    UiEvent e; e.type = UiEventType::ThemeChanged; e.themeId = 3;
    f.ui.queue.push_back(e);
    f.app->tick();
    StoredConfig saved; TEST_ASSERT_TRUE(f.store.load(saved));
    TEST_ASSERT_EQUAL_UINT8(3, saved.themeId);   // thème persisté (survit au reboot)
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_theme_change_persists);
    RUN_TEST(test_arm_pin_persists_pwtype_pin);
    RUN_TEST(test_rearm_after_unlock_returns_to_setup);
    RUN_TEST(test_unarmed_box_opens_the_lock);
    RUN_TEST(test_lock_is_driven_only_on_state_change);
    RUN_TEST(test_arm_request_arms_capsule);
    RUN_TEST(test_begin_forces_lock);
    RUN_TEST(test_date_not_reached_shows_countdown_stays_locked);
    RUN_TEST(test_date_reached_unlocks);
    RUN_TEST(test_or_rule_one_source_reached_opens);
    RUN_TEST(test_correct_password_unlocks);
    RUN_TEST(test_wrong_password_registers_backoff_and_persists);
    RUN_TEST(test_locked_out_ignores_password_attempt);
    return UNITY_END();
}
