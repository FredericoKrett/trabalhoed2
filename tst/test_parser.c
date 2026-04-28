#include "unity.h"
#include "parser.h"
#include "quadra.h"
#include "habitante.h"

#include <stddef.h>
#include <stdio.h>

static const char* GEO_FILE = "parser_test.geo";
static const char* PM_FILE = "parser_test.pm";

void setUp(void) {
    remove(GEO_FILE);
    remove(PM_FILE);
    remove("parser_quadras.hf");
    remove("parser_quadras.hfc");
    remove("parser_pessoas.hf");
    remove("parser_pessoas.hfc");
}

void tearDown(void) {
    remove(GEO_FILE);
    remove(PM_FILE);
    remove("parser_quadras.hf");
    remove("parser_quadras.hfc");
    remove("parser_pessoas.hf");
    remove("parser_pessoas.hfc");
}

void test_parser_geo(void) {
    FILE* f = fopen(GEO_FILE, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "cq 1.0px steelblue MistyRose\n");
    fprintf(f, "q b01.1 10 20 100 80\n");
    fclose(f);

    HashFile hf = hash_create(".", "parser_quadras",
                              quadra_get_record_size(),
                              quadra_get_key_offset(),
                              quadra_get_key_size());
    TEST_ASSERT_NOT_NULL(hf);

    parser_parse_geo(hf, GEO_FILE);

    void* out = calloc(1, quadra_get_record_size());
    TEST_ASSERT_NOT_NULL(out);
    TEST_ASSERT_TRUE(hash_search(hf, "b01.1", out));
    TEST_ASSERT_EQUAL_STRING("steelblue", quadra_get_cfill((Quadra)out));
    TEST_ASSERT_EQUAL_STRING("MistyRose", quadra_get_cstrk((Quadra)out));

    free(out);
    hash_close(hf);
}

void test_parser_pm(void) {
    FILE* f = fopen(PM_FILE, "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "p 123 Ana Silva F 01/02/2000\n");
    fprintf(f, "m 123 b01.1 N 30 ap-1\n");
    fclose(f);

    HashFile hf = hash_create(".", "parser_pessoas",
                              habitante_get_record_size(),
                              habitante_get_key_offset(),
                              habitante_get_key_size());
    TEST_ASSERT_NOT_NULL(hf);

    parser_parse_pm(hf, PM_FILE);

    void* out = calloc(1, habitante_get_record_size());
    TEST_ASSERT_NOT_NULL(out);
    TEST_ASSERT_TRUE(hash_search(hf, "123", out));
    TEST_ASSERT_EQUAL_STRING("Ana", habitante_get_nome((Habitante)out));
    TEST_ASSERT_TRUE(habitante_is_morador((Habitante)out));
    TEST_ASSERT_EQUAL_STRING("b01.1", habitante_get_cep((Habitante)out));
    TEST_ASSERT_EQUAL_CHAR('N', habitante_get_face((Habitante)out));

    free(out);
    hash_close(hf);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_parser_geo);
    RUN_TEST(test_parser_pm);
    return UNITY_END();
}
