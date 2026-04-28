#include "unity.h"
#include "sig.h"

void setUp(void) {}
void tearDown(void) {}

void test_sig_create_and_destroy(void) {
    SIG sig = sig_create();
    TEST_ASSERT_NOT_NULL(sig);
    sig_destroy(sig);
}

void test_sig_setters_accept_values(void) {
    SIG sig = sig_create();
    TEST_ASSERT_NOT_NULL(sig);

    sig_set_base_entrada(sig, "entrada");
    sig_set_base_saida(sig, "saida");
    sig_set_arquivo_geo(sig, "cidade.geo");
    sig_set_arquivo_pm(sig, "pessoas.pm");
    sig_set_arquivo_qry(sig, "consulta.qry");

    sig_destroy(sig);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_sig_create_and_destroy);
    RUN_TEST(test_sig_setters_accept_values);
    return UNITY_END();
}
