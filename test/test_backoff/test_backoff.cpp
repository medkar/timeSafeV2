#include <unity.h>
#include <Backoff.h>

using namespace tsafe;

void setUp() {}
void tearDown() {}

void test_first_failure_uses_base_delay() {
    BackoffParams p; // base 30s, maxShift 6
    AttemptState s = registerFailure(AttemptState{}, 1000, p);
    TEST_ASSERT_EQUAL_INT(1, s.failedCount);
    TEST_ASSERT_EQUAL_INT64(1000 + 30, s.lockedUntil); // 30 * 2^0
}

void test_second_failure_doubles_delay() {
    BackoffParams p;
    AttemptState first = registerFailure(AttemptState{}, 1000, p);
    AttemptState second = registerFailure(first, 2000, p);
    TEST_ASSERT_EQUAL_INT(2, second.failedCount);
    TEST_ASSERT_EQUAL_INT64(2000 + 60, second.lockedUntil); // 30 * 2^1
}

void test_delay_is_capped_at_max_shift() {
    BackoffParams p;
    p.baseSeconds = 30;
    p.maxShift = 2; // plafond = 30 * 2^2 = 120s
    AttemptState s;
    s.failedCount = 10; // très au-delà du plafond
    AttemptState next = registerFailure(s, 0, p);
    TEST_ASSERT_EQUAL_INT64(120, next.lockedUntil);
}

void test_is_locked_out() {
    AttemptState s;
    s.lockedUntil = 5000;
    TEST_ASSERT_TRUE(isLockedOut(s, 4999));
    TEST_ASSERT_FALSE(isLockedOut(s, 5000));
    TEST_ASSERT_FALSE(isLockedOut(s, 6000));
}

void test_reset_clears_state() {
    AttemptState s = resetAttempts();
    TEST_ASSERT_EQUAL_INT(0, s.failedCount);
    TEST_ASSERT_EQUAL_INT64(0, s.lockedUntil);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_first_failure_uses_base_delay);
    RUN_TEST(test_second_failure_doubles_delay);
    RUN_TEST(test_delay_is_capped_at_max_shift);
    RUN_TEST(test_is_locked_out);
    RUN_TEST(test_reset_clears_state);
    return UNITY_END();
}
