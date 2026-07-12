#include <unity.h>
#include <Domain.h>

using namespace tsafe;

void setUp() {}
void tearDown() {}

void test_boxconfig_defaults() {
    BoxConfig c;
    TEST_ASSERT_FALSE(c.armed);
    TEST_ASSERT_FALSE(c.hasDate);
    TEST_ASSERT_FALSE(c.hasPassword);
    TEST_ASSERT_EQUAL_INT64(0, c.openDate);
}

void test_policyresult_defaults() {
    PolicyResult r;
    TEST_ASSERT_EQUAL_INT((int)PolicyState::Setup, (int)r.state);
    TEST_ASSERT_FALSE(r.lockedOut);
    TEST_ASSERT_EQUAL_INT64(0, r.remainingSeconds);
}

void test_timestatus_defaults() {
    TimeStatus t;
    TEST_ASSERT_FALSE(t.trusted);
    TEST_ASSERT_FALSE(t.anomaly);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_boxconfig_defaults);
    RUN_TEST(test_policyresult_defaults);
    RUN_TEST(test_timestatus_defaults);
    return UNITY_END();
}
