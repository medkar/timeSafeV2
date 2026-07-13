#include <unity.h>
#include <LocalTime.h>

using namespace tsafe;

void setUp() {}
void tearDown() {}

// --- Décalage Europe/Paris (CET +1 / CEST +2, DST auto) ---
void test_offset_winter_is_cet() {
    TEST_ASSERT_EQUAL_INT(1, parisOffsetHours(2026, 1, 15));
    TEST_ASSERT_EQUAL_INT(1, parisOffsetHours(2026, 12, 25));
}
void test_offset_summer_is_cest() {
    TEST_ASSERT_EQUAL_INT(2, parisOffsetHours(2026, 7, 1));
    TEST_ASSERT_EQUAL_INT(2, parisOffsetHours(2026, 5, 10));
}
// Bascule 2026 : dernier dimanche de mars = 29, dernier dimanche d'octobre = 25.
void test_offset_march_dst_boundary() {
    TEST_ASSERT_EQUAL_INT(1, parisOffsetHours(2026, 3, 28)); // veille -> encore CET
    TEST_ASSERT_EQUAL_INT(2, parisOffsetHours(2026, 3, 29)); // bascule -> CEST
}
void test_offset_october_dst_boundary() {
    TEST_ASSERT_EQUAL_INT(2, parisOffsetHours(2026, 10, 24)); // encore CEST
    TEST_ASSERT_EQUAL_INT(1, parisOffsetHours(2026, 10, 25)); // retour CET
}

// --- Heure locale Paris -> epoch UTC ---
void test_local_to_epoch_winter() {
    // 2026-01-01 00:00 Paris (CET) = 2025-12-31 23:00 UTC = 1767222000
    TEST_ASSERT_EQUAL_INT64(1767222000LL, parisLocalToEpoch(2026, 1, 1, 0, 0));
}
void test_local_to_epoch_summer() {
    // 2026-07-01 12:00 Paris (CEST) = 2026-07-01 10:00 UTC = 1782900000
    TEST_ASSERT_EQUAL_INT64(1782900000LL, parisLocalToEpoch(2026, 7, 1, 12, 0));
}

// --- Aller-retour ---
void test_round_trip() {
    int64_t e = parisLocalToEpoch(2026, 12, 25, 18, 30);
    int y, mo, d, h, mi;
    epochToParisLocal(e, y, mo, d, h, mi);
    TEST_ASSERT_EQUAL_INT(2026, y);
    TEST_ASSERT_EQUAL_INT(12, mo);
    TEST_ASSERT_EQUAL_INT(25, d);
    TEST_ASSERT_EQUAL_INT(18, h);
    TEST_ASSERT_EQUAL_INT(30, mi);
}
void test_round_trip_summer() {
    int64_t e = parisLocalToEpoch(2027, 8, 9, 7, 5);
    int y, mo, d, h, mi;
    epochToParisLocal(e, y, mo, d, h, mi);
    TEST_ASSERT_EQUAL_INT(2027, y);
    TEST_ASSERT_EQUAL_INT(8, mo);
    TEST_ASSERT_EQUAL_INT(9, d);
    TEST_ASSERT_EQUAL_INT(7, h);
    TEST_ASSERT_EQUAL_INT(5, mi);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_offset_winter_is_cet);
    RUN_TEST(test_offset_summer_is_cest);
    RUN_TEST(test_offset_march_dst_boundary);
    RUN_TEST(test_offset_october_dst_boundary);
    RUN_TEST(test_local_to_epoch_winter);
    RUN_TEST(test_local_to_epoch_summer);
    RUN_TEST(test_round_trip);
    RUN_TEST(test_round_trip_summer);
    return UNITY_END();
}
