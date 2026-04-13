#ifndef HASHFILE_H
#define HASHFILE_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file hashfile.h
 * @brief Implementação de Hashing Dinâmico Extensível em Disco.
 * 
 * Este módulo abstrai a criação, inserção, busca e remoção de registros em arquivos
 * binários '.dir' (Directory) e '.dat' (Buckets). 
 * Foi desenvolvido utilizando o conceito de TAD Opaco e aderindo ao padrão C99.
 */

/**
 * @typedef HashFile
 * @brief Tipo Abstrato de Dados (TAD Opaco) que representa o contexto do Hashfile.
 * A estrutura interna está oculta no arquivo .c para encapsulamento.
 */
typedef struct HashFile HashFile;

/**
 * @brief Inicializa e cria os arquivos base (.dir e .dat) do Hash Dinâmico.
 * 
 * @param out_dir Diretório de saída onde os arquivos serão salvos (ex: fornecido via -o). Se NULL, usa o atual.
 * @param filename_prefix Prefixo do nome dos arquivos (ex: "quadras").
 * @param record_size Tamanho em bytes de cada registro.
 * @param key_offset Distância (offset) em bytes do início do registro até o inicio da chave string.
 * @param key_size Tamanho fixo em bytes da chave string (ex: 32).
 * @return HashFile* Ponteiro opaco para a estrutura inicializada ou NULL em erro.
 */
HashFile* hash_create(const char* out_dir, const char* filename_prefix, int record_size, int key_offset, int key_size);

/**
 * @brief Abre arquivos de um Hash Dinâmico já existente.
 * 
 * @param in_dir Diretório onde os arquivos estão salvos (ex: fornecido via -e). Se NULL, usa o atual.
 * @param filename_prefix Prefixo do arquivo a ser lido.
 * @return HashFile* Ponteiro opaco para a estrutura ou NULL se não encontrado.
 */
HashFile* hash_open(const char* in_dir, const char* filename_prefix);

/**
 * @brief Insere um registro no Hash Dinâmico.
 * Trata automaticamente detecção de overflow e split de buckets.
 * 
 * @param hf Ponteiro para o contexto do Hashfile.
 * @param reg Ponteiro genérico para o registro a ser copiado para o disco.
 * @return true Se inserido com sucesso.
 * @return false Se ocorreu um erro ou a chave já existe.
 */
bool hash_insert(HashFile* hf, void* reg);

/**
 * @brief Busca um registro pelo valor da sua chave alfanumérica (string).
 * 
 * @param hf Ponteiro para o contexto do Hashfile.
 * @param key Chave string exata a ser buscada.
 * @param out_reg Ponteiro com memória alocada onde o conteúdo encontrado será copiado.
 * @return true Se o registro for encontrado e copiado.
 * @return false Se não encontrado.
 */
bool hash_search(HashFile* hf, const char* key, void* out_reg);

/**
 * @brief Remove logicamente (tombstone) um registro do Hash pela sua chave iterativa.
 * 
 * @param hf Ponteiro para o contexto do Hashfile.
 * @param key Chave string exata a ser removida.
 * @return true Se removido com sucesso.
 * @return false Se a chave não existir.
 */
bool hash_delete(HashFile* hf, const char* key);

/**
 * @brief Gera o arquivo legível (.hfd) contendo o estado global do diretório e buckets.
 * 
 * @param hf Ponteiro para o contexto do Hashfile.
 * @param out_dir Diretório de saída para gerar o relatório textual.
 * @param filename_txt Nome/prefixo do arquivo HFD.
 */
void hash_print_directory(HashFile* hf, const char* out_dir, const char* filename_txt);

/**
 * @brief Fecha os arquivos, grava os metadados em disco e libera a memória.
 * 
 * @param hf Ponteiro para o contexto do Hashfile.
 */
void hash_close(HashFile* hf);

#endif // HASHFILE_H
