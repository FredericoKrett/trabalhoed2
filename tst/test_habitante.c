#include "unity.h"
#include "habitante.h"

void setUp(void) {}
void tearDown(void) {}

void test_habitante_dados_pessoais(void) {
    Habitante h = habitante_create("123", "Ana", "Silva", 'F', "01/02/2000");

    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_EQUAL_STRING("123", habitante_get_cpf(h));
    TEST_ASSERT_EQUAL_STRING("Ana", habitante_get_nome(h));
    TEST_ASSERT_EQUAL_STRING("Silva", habitante_get_sobrenome(h));
    TEST_ASSERT_EQUAL_CHAR('F', habitante_get_sexo(h));
    TEST_ASSERT_EQUAL_STRING("01/02/2000", habitante_get_nascimento(h));
    TEST_ASSERT_FALSE(habitante_is_morador(h));

    habitante_free(h);
}

void test_habitante_endereco(void) {
    Habitante h = habitante_create("456", "Joao", "Souza", 'M', "03/04/1999");

    habitante_set_endereco(h, "b01.1", 'N', 25.0, "ap-1");
    TEST_ASSERT_TRUE(habitante_is_morador(h));
    TEST_ASSERT_EQUAL_STRING("b01.1", habitante_get_cep(h));
    TEST_ASSERT_EQUAL_CHAR('N', habitante_get_face(h));
    TEST_ASSERT_EQUAL_INT(250, (int)(habitante_get_num(h) * 10.0));
    TEST_ASSERT_EQUAL_STRING("ap-1", habitante_get_compl(h));

    habitante_remove_endereco(h);
    TEST_ASSERT_FALSE(habitante_is_morador(h));
    TEST_ASSERT_EQUAL_STRING("", habitante_get_cep(h));

    habitante_free(h);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_habitante_dados_pessoais);
    RUN_TEST(test_habitante_endereco);
    return UNITY_END();
}
