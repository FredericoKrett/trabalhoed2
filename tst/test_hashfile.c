#include "unity.h"
#include "hashfile.h"
#include <stdio.h>
#include <stddef.h>
#include <string.h>

// estrutura do registro deslocada do hash.h para o dominio de negocio local / test runner
typedef struct {
    char id[32]; // chave primaria alfanumerica
    char nome[50];
    int idade;
} Record;

HashFile hf;

void setUp(void) {
    remove("teste.hfc");
    remove("teste.hf");
    hf = hash_create(".", "teste", sizeof(Record), offsetof(Record, id), 32);
}

void tearDown(void) {
    if (hf) {
        hash_close(hf);
        hf = NULL;
    }
}

void test_hash_insert_and_search(void) {
    for (int i = 0; i < 20; i++) {
        Record r;
        snprintf(r.id, sizeof(r.id), "ID-%d", i);
        snprintf(r.nome, sizeof(r.nome), "Registro %d", i);
        r.idade = 20 + i;
        TEST_ASSERT_TRUE(hash_insert(hf, &r));
    }
    
    for (int i = 0; i < 20; i++) {
        Record out;
        char id_str[32];
        snprintf(id_str, sizeof(id_str), "ID-%d", i);
        TEST_ASSERT_TRUE(hash_search(hf, id_str, &out));
        TEST_ASSERT_EQUAL_STRING(id_str, out.id);
        TEST_ASSERT_EQUAL_INT(20 + i, out.idade);
    }
}

void test_hash_delete(void) {
    for (int i = 0; i < 20; i++) {
        Record r;
        snprintf(r.id, sizeof(r.id), "ID-%d", i);
        snprintf(r.nome, sizeof(r.nome), "Registro %d", i);
        r.idade = 20 + i;
        hash_insert(hf, &r);
    }

    for (int i = 0; i < 10; i++) {
        char id_str[32];
        snprintf(id_str, sizeof(id_str), "ID-%d", i);
        TEST_ASSERT_TRUE(hash_delete(hf, id_str));
    }

    for (int i = 0; i < 10; i++) {
        Record out;
        char id_str[32];
        snprintf(id_str, sizeof(id_str), "ID-%d", i);
        TEST_ASSERT_FALSE(hash_search(hf, id_str, &out));
    }

    for (int i = 10; i < 20; i++) {
        Record out;
        char id_str[32];
        snprintf(id_str, sizeof(id_str), "ID-%d", i);
        TEST_ASSERT_TRUE(hash_search(hf, id_str, &out));
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_hash_insert_and_search);
    RUN_TEST(test_hash_delete);
    return UNITY_END();
}
