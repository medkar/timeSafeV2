#include <unity.h>
#include <ThemeRegistry.h>
#include <cstring>

using namespace tsafe;

void setUp() {}
void tearDown() {}

void test_default_is_safe() {
    TEST_ASSERT_EQUAL_UINT8(0, defaultThemeId());
    TEST_ASSERT_EQUAL_INT((int)ThemeId::Safe, (int)themeById(0).id);
}

void test_count_is_five() {
    TEST_ASSERT_EQUAL_UINT32(5, (uint32_t)themeCount());
}

void test_safe_tokens() {
    const Theme& t = themeById((uint8_t)ThemeId::Safe);
    TEST_ASSERT_EQUAL_HEX32(0x38bdf8, t.palette.accent);
    TEST_ASSERT_EQUAL_INT((int)ThemeFont::Mono, (int)t.font);
    TEST_ASSERT_FALSE(t.sparkles);
    TEST_ASSERT_EQUAL_STRING("Ouverture dans", t.wording.countdownLabel);
    TEST_ASSERT_EQUAL_STRING("Armer la bo\xC3\xAe" "te", t.wording.armButton); // "Armer la boîte" en UTF-8
}

void test_gift_tokens_and_wording() {
    const Theme& t = themeById((uint8_t)ThemeId::Gift);
    TEST_ASSERT_EQUAL_HEX32(0xf6c85a, t.palette.accent);
    TEST_ASSERT_EQUAL_INT((int)ThemeFont::Round, (int)t.font);
    TEST_ASSERT_TRUE(t.sparkles);
    TEST_ASSERT_EQUAL_STRING("Emballer", t.wording.armButton);
}

void test_all_five_present_with_distinct_accents() {
    // Les 5 thèmes montrés : Coffre, Cadeau, Souvenir, Fête, Enfant.
    const uint32_t accents[5] = { 0x38bdf8, 0xf6c85a, 0xf0d79a, 0xffd76a, 0xffe24d };
    for (uint8_t i = 0; i < 5; ++i) {
        TEST_ASSERT_EQUAL_HEX32(accents[i], themeById(i).palette.accent);
    }
}

void test_unknown_id_falls_back_to_safe() {
    TEST_ASSERT_EQUAL_INT((int)ThemeId::Safe, (int)themeById(99).id);
}

void test_all_indices_have_distinct_ids() {
    bool seen[5] = { false, false, false, false, false };
    for (size_t i = 0; i < themeCount(); ++i) {
        uint8_t id = (uint8_t)themeByIndex(i).id;
        TEST_ASSERT_TRUE(id < 5);
        TEST_ASSERT_FALSE(seen[id]);
        seen[id] = true;
    }
}

void test_every_theme_has_a_warning_color() {
    // La couleur d'avertissement doit rester définie sur chaque thème (lisibilité alerte).
    for (size_t i = 0; i < themeCount(); ++i) {
        TEST_ASSERT_NOT_EQUAL(0u, themeByIndex(i).palette.warn);
    }
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_default_is_safe);
    RUN_TEST(test_count_is_five);
    RUN_TEST(test_safe_tokens);
    RUN_TEST(test_gift_tokens_and_wording);
    RUN_TEST(test_all_five_present_with_distinct_accents);
    RUN_TEST(test_unknown_id_falls_back_to_safe);
    RUN_TEST(test_all_indices_have_distinct_ids);
    RUN_TEST(test_every_theme_has_a_warning_color);
    return UNITY_END();
}
