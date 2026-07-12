#include <unity.h>
#include <TimeCore.h>

using namespace tsafe;

void setUp() {}
void tearDown() {}

static TimeResolveInput baseInput() {
    TimeResolveInput in;
    in.toleranceSeconds = 900; // 15 min
    return in;
}

void test_only_https_present_is_trusted() {
    TimeResolveInput in = baseInput();
    in.https = {true, 1000};
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_TRUE(s.trusted);
    TEST_ASSERT_FALSE(s.anomaly);
    TEST_ASSERT_EQUAL_INT64(1000, s.effectiveNow);
}

void test_only_rtc_present_is_trusted() {
    TimeResolveInput in = baseInput();
    in.rtc = {true, 2000};
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_TRUE(s.trusted);
    TEST_ASSERT_EQUAL_INT64(2000, s.effectiveNow);
}

void test_both_agree_uses_minimum() {
    TimeResolveInput in = baseInput();
    in.https = {true, 5000};
    in.rtc = {true, 5100}; // écart 100s < tolérance
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_TRUE(s.trusted);
    TEST_ASSERT_FALSE(s.anomaly);
    TEST_ASSERT_EQUAL_INT64(5000, s.effectiveNow); // minimum conservateur
}

void test_both_diverge_is_anomaly() {
    TimeResolveInput in = baseInput();
    in.https = {true, 5000};
    in.rtc = {true, 100000}; // écart énorme > tolérance
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_TRUE(s.anomaly);
    TEST_ASSERT_FALSE(s.trusted);
}

void test_no_source_is_untrusted_not_anomaly() {
    TimeResolveInput in = baseInput();
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_FALSE(s.trusted);
    TEST_ASSERT_FALSE(s.anomaly);
}

void test_rollback_below_last_known_is_anomaly() {
    TimeResolveInput in = baseInput();
    in.https = {true, 1000};
    in.hasLastKnown = true;
    in.lastKnownGood = 100000; // très en avance sur la source -> retour arrière suspect
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_TRUE(s.anomaly);
    TEST_ASSERT_FALSE(s.trusted);
}

void test_small_backward_within_tolerance_is_ok() {
    TimeResolveInput in = baseInput();
    in.https = {true, 99500};
    in.hasLastKnown = true;
    in.lastKnownGood = 100000; // recul de 500s < tolérance -> toléré
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_TRUE(s.trusted);
    TEST_ASSERT_FALSE(s.anomaly);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_only_https_present_is_trusted);
    RUN_TEST(test_only_rtc_present_is_trusted);
    RUN_TEST(test_both_agree_uses_minimum);
    RUN_TEST(test_both_diverge_is_anomaly);
    RUN_TEST(test_no_source_is_untrusted_not_anomaly);
    RUN_TEST(test_rollback_below_last_known_is_anomaly);
    RUN_TEST(test_small_backward_within_tolerance_is_ok);
    return UNITY_END();
}
