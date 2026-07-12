#include <unity.h>
#include <LockPolicy.h>

using namespace tsafe;

void setUp() {}
void tearDown() {}

// Fabrique une entrée « boîte armée, temps fiable » comme base.
static PolicyInput armed(int64_t now) {
    PolicyInput in;
    in.config.armed = true;
    in.time.trusted = true;
    in.time.effectiveNow = now;
    in.now = now;
    return in;
}

void test_not_armed_is_setup() {
    PolicyInput in;
    in.config.armed = false;
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Setup, (int)decide(in).state);
}

void test_anomaly_is_alert() {
    PolicyInput in = armed(1000);
    in.time.anomaly = true;
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Alert, (int)decide(in).state);
}

void test_date_required_but_time_untrusted_is_waiting() {
    PolicyInput in = armed(1000);
    in.time.trusted = false;
    in.config.hasDate = true;
    in.config.openDate = 5000;
    TEST_ASSERT_EQUAL_INT((int)PolicyState::WaitingSync, (int)decide(in).state);
}

void test_date_not_reached_is_countdown_with_remaining() {
    PolicyInput in = armed(1000);
    in.config.hasDate = true;
    in.config.openDate = 5000;
    PolicyResult r = decide(in);
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Countdown, (int)r.state);
    TEST_ASSERT_EQUAL_INT64(4000, r.remainingSeconds);
}

void test_date_reached_no_password_is_unlock() {
    PolicyInput in = armed(6000);
    in.config.hasDate = true;
    in.config.openDate = 5000;
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Unlock, (int)decide(in).state);
}

void test_password_only_not_satisfied_is_askpassword() {
    PolicyInput in = armed(1000);
    in.config.hasPassword = true;
    in.passwordSatisfied = false;
    PolicyResult r = decide(in);
    TEST_ASSERT_EQUAL_INT((int)PolicyState::AskPassword, (int)r.state);
    TEST_ASSERT_FALSE(r.lockedOut);
}

void test_password_lockout_window_sets_lockedout() {
    PolicyInput in = armed(1000);
    in.config.hasPassword = true;
    in.passwordSatisfied = false;
    in.attempts.lockedUntil = 1500; // now(1000) < 1500
    PolicyResult r = decide(in);
    TEST_ASSERT_EQUAL_INT((int)PolicyState::AskPassword, (int)r.state);
    TEST_ASSERT_TRUE(r.lockedOut);
    TEST_ASSERT_EQUAL_INT64(1500, r.retryAt);
}

void test_password_satisfied_is_unlock() {
    PolicyInput in = armed(1000);
    in.config.hasPassword = true;
    in.passwordSatisfied = true;
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Unlock, (int)decide(in).state);
}

void test_combined_date_gate_comes_before_password() {
    // Date pas encore atteinte + mot de passe : on doit voir le rebours, pas la saisie.
    PolicyInput in = armed(1000);
    in.config.hasDate = true;
    in.config.openDate = 5000;
    in.config.hasPassword = true;
    in.passwordSatisfied = true; // même satisfait, la date prime
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Countdown, (int)decide(in).state);
}

void test_combined_date_met_then_password() {
    PolicyInput in = armed(6000);
    in.config.hasDate = true;
    in.config.openDate = 5000;
    in.config.hasPassword = true;
    in.passwordSatisfied = false;
    TEST_ASSERT_EQUAL_INT((int)PolicyState::AskPassword, (int)decide(in).state);
}

void test_combined_all_satisfied_is_unlock() {
    PolicyInput in = armed(6000);
    in.config.hasDate = true;
    in.config.openDate = 5000;
    in.config.hasPassword = true;
    in.passwordSatisfied = true;
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Unlock, (int)decide(in).state);
}

void test_armed_with_no_condition_is_unlock() {
    // Config dégénérée (ni date ni mdp) : la boîte s'ouvre. L'UI empêche d'armer ainsi.
    PolicyInput in = armed(1000);
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Unlock, (int)decide(in).state);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_not_armed_is_setup);
    RUN_TEST(test_anomaly_is_alert);
    RUN_TEST(test_date_required_but_time_untrusted_is_waiting);
    RUN_TEST(test_date_not_reached_is_countdown_with_remaining);
    RUN_TEST(test_date_reached_no_password_is_unlock);
    RUN_TEST(test_password_only_not_satisfied_is_askpassword);
    RUN_TEST(test_password_lockout_window_sets_lockedout);
    RUN_TEST(test_password_satisfied_is_unlock);
    RUN_TEST(test_combined_date_gate_comes_before_password);
    RUN_TEST(test_combined_date_met_then_password);
    RUN_TEST(test_combined_all_satisfied_is_unlock);
    RUN_TEST(test_armed_with_no_condition_is_unlock);
    return UNITY_END();
}
