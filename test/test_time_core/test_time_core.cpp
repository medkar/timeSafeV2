#include <unity.h>
#include <TimeCore.h>

using namespace tsafe;

void setUp() {}
void tearDown() {}

static TimeResolveInput baseInput() {
    TimeResolveInput in;
    in.toleranceSeconds = 900; // conservé dans la struct mais non utilisé (règle OU)
    return in;
}

void test_only_https_present_is_trusted() {
    TimeResolveInput in = baseInput();
    in.https = {true, 1000};
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_TRUE(s.trusted);    TEST_ASSERT_EQUAL_INT64(1000, s.effectiveNow);
}

void test_only_rtc_present_is_trusted() {
    TimeResolveInput in = baseInput();
    in.rtc = {true, 2000};
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_TRUE(s.trusted);
    TEST_ASSERT_EQUAL_INT64(2000, s.effectiveNow);
}

// Règle OU : deux sources présentes -> on garde le MAXIMUM (ouvre dès qu'une atteint la date).
void test_both_present_uses_maximum() {
    TimeResolveInput in = baseInput();
    in.https = {true, 5000};
    in.rtc = {true, 5100};
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_TRUE(s.trusted);    TEST_ASSERT_EQUAL_INT64(5100, s.effectiveNow); // maximum (règle OU)
}

// Grosse divergence entre sources : aucun blocage, on prend simplement le maximum.
void test_large_divergence_uses_maximum() {
    TimeResolveInput in = baseInput();
    in.https = {true, 5000};
    in.rtc = {true, 100000};
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_TRUE(s.trusted);    TEST_ASSERT_EQUAL_INT64(100000, s.effectiveNow);
}

void test_no_source_is_untrusted() {
    TimeResolveInput in = baseInput();
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_FALSE(s.trusted);}

// Le « retour arrière » sous le dernier temps connu n'entraîne plus de verrou.
void test_rollback_below_last_known_is_accepted() {
    TimeResolveInput in = baseInput();
    in.https = {true, 1000};
    in.hasLastKnown = true;
    in.lastKnownGood = 100000;
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_TRUE(s.trusted);    TEST_ASSERT_EQUAL_INT64(1000, s.effectiveNow);
}

// --- Robustesse & bornes (les valeurs implausibles restent ignorées) ---

void test_zero_epoch_source_is_ignored() {
    // Un RTC non initialisé renvoie souvent 0 : doit être ignoré, pas cru.
    TimeResolveInput in = baseInput();
    in.https = {true, 0};
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_FALSE(s.trusted);}

void test_absurd_epoch_source_is_ignored() {
    // Valeur aberrante (au-delà de ~an 3000) : ignorée (évite aussi l'overflow int64).
    TimeResolveInput in = baseInput();
    in.rtc = {true, 9223372036854775807LL}; // INT64_MAX
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_FALSE(s.trusted);
}

void test_one_garbage_one_valid_uses_valid() {
    TimeResolveInput in = baseInput();
    in.https = {true, 0};   // ignorée
    in.rtc = {true, 5000};  // valide
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_TRUE(s.trusted);
    TEST_ASSERT_EQUAL_INT64(5000, s.effectiveNow);
}

// Contrepartie explicite de la règle OU : une source haute (mais plausible) domine.
void test_high_but_plausible_source_dominates() {
    TimeResolveInput in = baseInput();
    in.https = {true, 1000};
    in.rtc = {true, 2000000000}; // ~2033, plausible
    TimeStatus s = resolveTime(in);
    TEST_ASSERT_TRUE(s.trusted);
    TEST_ASSERT_EQUAL_INT64(2000000000, s.effectiveNow);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_only_https_present_is_trusted);
    RUN_TEST(test_only_rtc_present_is_trusted);
    RUN_TEST(test_both_present_uses_maximum);
    RUN_TEST(test_large_divergence_uses_maximum);
    RUN_TEST(test_no_source_is_untrusted);
    RUN_TEST(test_rollback_below_last_known_is_accepted);
    RUN_TEST(test_zero_epoch_source_is_ignored);
    RUN_TEST(test_absurd_epoch_source_is_ignored);
    RUN_TEST(test_one_garbage_one_valid_uses_valid);
    RUN_TEST(test_high_but_plausible_source_dominates);
    return UNITY_END();
}
