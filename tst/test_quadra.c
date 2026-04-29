#include "unity.h"
#include "quadra.h"

void setUp(void) {}
void tearDown(void) {}

void test_quadra_dados_basicos(void) {
    Quadra q = quadra_create("b01.1", 10.0, 20.0, 100.0, 80.0, "blue", "red", 2.0);

    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_STRING("b01.1", quadra_get_cep(q));
    TEST_ASSERT_EQUAL_INT(10, (int)quadra_get_x(q));
    TEST_ASSERT_EQUAL_INT(20, (int)quadra_get_y(q));
    TEST_ASSERT_EQUAL_INT(100, (int)quadra_get_w(q));
    TEST_ASSERT_EQUAL_INT(80, (int)quadra_get_h(q));
    TEST_ASSERT_EQUAL_STRING("blue", quadra_get_cfill(q));
    TEST_ASSERT_EQUAL_STRING("red", quadra_get_cstrk(q));
    TEST_ASSERT_EQUAL_INT(2, (int)quadra_get_sw(q));

    quadra_free(q);
}

void test_quadra_pontos_de_referencia(void) {
    Quadra q = quadra_create("b01.1", 10.0, 20.0, 100.0, 80.0, "blue", "red", 2.0);
    double x = 0.0;
    double y = 0.0;

    quadra_get_anchor(q, &x, &y);
    TEST_ASSERT_EQUAL_INT(110, (int)x);
    TEST_ASSERT_EQUAL_INT(100, (int)y);

    quadra_get_address_point(q, 'N', 30.0, &x, &y);
    TEST_ASSERT_EQUAL_INT(40, (int)x);
    TEST_ASSERT_EQUAL_INT(100, (int)y);

    quadra_get_address_point(q, 'S', 30.0, &x, &y);
    TEST_ASSERT_EQUAL_INT(40, (int)x);
    TEST_ASSERT_EQUAL_INT(20, (int)y);

    quadra_get_address_point(q, 'L', 30.0, &x, &y);
    TEST_ASSERT_EQUAL_INT(10, (int)x);
    TEST_ASSERT_EQUAL_INT(50, (int)y);

    quadra_get_address_point(q, 'O', 30.0, &x, &y);
    TEST_ASSERT_EQUAL_INT(110, (int)x);
    TEST_ASSERT_EQUAL_INT(50, (int)y);

    quadra_free(q);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_quadra_dados_basicos);
    RUN_TEST(test_quadra_pontos_de_referencia);
    return UNITY_END();
}
