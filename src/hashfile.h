#ifndef HASHFILE_H
#define HASHFILE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file hashfile.h
 * @brief TAD opaco para hash dinamico extensivel armazenado em disco.
 *
 * O modulo persiste registros de tamanho fixo em arquivo binario .hf e
 * mantem os metadados do diretorio em .hfc. Ao final da execucao, o
 * estado do diretorio e as expansoes de buckets podem ser emitidos em
 * arquivo texto .hfd.
 */

/**
 * @brief Ponteiro opaco para o gerenciador de HashFile.
 */
typedef void* HashFile;

/**
 * @brief Cria um HashFile vazio.
 *
 * @param out_dir Diretorio onde os arquivos serao criados.
 * @param filename_prefix Prefixo dos arquivos .hf, .hfc e .hfd.
 * @param record_size Tamanho de cada registro armazenado.
 * @param key_offset Offset da chave dentro do registro.
 * @param key_size Tamanho maximo da chave.
 * @return Ponteiro opaco para o HashFile criado, ou NULL em caso de erro.
 */
HashFile hash_create(const char* out_dir, const char* filename_prefix, int record_size, int key_offset, int key_size);

/**
 * @brief Abre um HashFile existente.
 *
 * @param in_dir Diretorio onde os arquivos estao armazenados.
 * @param filename_prefix Prefixo dos arquivos do HashFile.
 * @return Ponteiro opaco para o HashFile aberto, ou NULL em caso de erro.
 */
HashFile hash_open(const char* in_dir, const char* filename_prefix);

/**
 * @brief Insere um registro no HashFile.
 *
 * A funcao rejeita chaves duplicadas e executa split/doubling quando o
 * bucket de destino atinge sua capacidade.
 *
 * @param hf HashFile de destino.
 * @param reg Registro completo a ser persistido.
 * @return true se a insercao ocorreu; false caso contrario.
 */
bool hash_insert(HashFile hf, void* reg);

/**
 * @brief Busca um registro pela chave.
 *
 * @param hf HashFile consultado.
 * @param key Chave alfanumerica procurada.
 * @param out_reg Area ja alocada para receber o registro encontrado.
 * @return true se a chave foi encontrada; false caso contrario.
 */
bool hash_search(HashFile hf, const char* key, void* out_reg);

/**
 * @brief Remove logicamente um registro pela chave.
 *
 * @param hf HashFile alterado.
 * @param key Chave a remover.
 * @return true se a chave existia e foi removida; false caso contrario.
 */
bool hash_delete(HashFile hf, const char* key);

/**
 * @brief Emite uma representacao textual do HashFile.
 *
 * O arquivo .hfd descreve o diretorio, os buckets e as expansoes
 * realizadas durante a execucao.
 *
 * @param hf HashFile inspecionado.
 * @param out_dir Diretorio de saida.
 * @param filename_txt Prefixo do arquivo .hfd.
 */
void hash_print_directory(HashFile hf, const char* out_dir, const char* filename_txt);

/**
 * @brief Fecha arquivos, grava metadados pendentes e libera memoria.
 *
 * @param hf HashFile a ser fechado.
 */
void hash_close(HashFile hf);

/**
 * @brief Assinatura usada para iterar pelos registros ativos.
 */
typedef void (*HashIteratorFunc)(void* record, void* context);

/**
 * @brief Percorre todos os registros ativos do HashFile.
 *
 * @param hf HashFile percorrido.
 * @param callback Funcao chamada uma vez para cada registro ativo.
 * @param context Contexto repassado ao callback.
 */
void hash_for_each(HashFile hf, HashIteratorFunc callback, void* context);

#endif // HASHFILE_H
