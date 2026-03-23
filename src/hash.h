#ifndef HASH_EXTENSIVEL_H
#define HASH_EXTENSIVEL_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// estrutura do registro
typedef struct {
    int id; // chave primaria
    char nome[50];
    int idade;
} Record;

// tipo opaco para o contexto do hash file ocultando a implementacao
typedef struct HashFile HashFile;

// cria um novo arquivo hash vazio
// argumento filename prefix ex dados gera dados dir e dados dat
// retorna true se sucesso
bool hash_create(const char* filename_prefix);

// abre um arquivo hash existente
// retorna um ponteiro para contexto ou null se falhar
HashFile* hash_open(const char* filename_prefix);

// insere um registro no arquivo hash
// lida com split de bucket e duplicacao do diretorio quando necessario
// retorna true se sucesso false se chave ja existe ou erro
bool hash_insert(HashFile* hf, Record rec);

// busca por um registro atraves da chave
// o dado encontrado sera copiado para out rec
// retorna true se encontrou
bool hash_search(HashFile* hf, int key, Record* out_rec);

// remove um registro pela chave
// retorna true se sucesso false se nao encontrado
bool hash_delete(HashFile* hf, int key);

// fecha o contexto do hash salvando o diretorio em disco
void hash_close(HashFile* hf);

#endif // HASH_EXTENSIVEL_H
