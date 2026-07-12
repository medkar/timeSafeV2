#include <unity.h>

void setUp() {}
void tearDown() {}

void test_toolchain_ok() {
    TEST_ASSERT_EQUAL_INT(4, 2 + 2);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_toolchain_ok);
    return UNITY_END();
}
