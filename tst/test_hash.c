#include "unity.h"
#include "hash.h"
#include <stdio.h>
#include <stddef.h>

// estrutura do registro deslocada do hash.h para o dominio de negocio local / test runner
typedef struct {
    int id; // chave primaria
    char nome[50];
    int idade;
} Record;

HashFile* hf;

void setUp(void) {
    remove("teste.dir");
    remove("teste.dat");
    hash_create("teste", sizeof(Record), offsetof(Record, id));
    hf = hash_open("teste");
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
        r.id = i;
        snprintf(r.nome, sizeof(r.nome), "Registro %d", i);
        r.idade = 20 + i;
        TEST_ASSERT_TRUE(hash_insert(hf, &r));
    }
    
    for (int i = 0; i < 20; i++) {
        Record out;
        TEST_ASSERT_TRUE(hash_search(hf, i, &out));
        TEST_ASSERT_EQUAL_INT(i, out.id);
        TEST_ASSERT_EQUAL_INT(20 + i, out.idade);
    }
}

void test_hash_delete(void) {
    for (int i = 0; i < 20; i++) {
        Record r;
        r.id = i;
        snprintf(r.nome, sizeof(r.nome), "Registro %d", i);
        r.idade = 20 + i;
        hash_insert(hf, &r);
    }

    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_TRUE(hash_delete(hf, i));
    }

    for (int i = 0; i < 10; i++) {
        Record out;
        TEST_ASSERT_FALSE(hash_search(hf, i, &out));
    }

    for (int i = 10; i < 20; i++) {
        Record out;
        TEST_ASSERT_TRUE(hash_search(hf, i, &out));
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_hash_insert_and_search);
    RUN_TEST(test_hash_delete);
    return UNITY_END();
}
