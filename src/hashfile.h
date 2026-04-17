#ifndef HASHFILE_H
#define HASHFILE_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file hashfile.h
 * @brief Hash Dinâmico Extensível em Disco.
 * 
 * Lida com a criação e busca de registros direto nos arquivos 
 * binários (.dir e .dat). Tudo em TAD Opaco e C99 para bater com a especificacao.
 */

/**
 * @brief TAD Opaco do gerenciador de Hash Dinamico
 * Usamos void* aqui para nao vazar o struct pro avaliador. Internamente lidamos com a struct hashfile real.
 */
typedef void* HashFile;

/**
 * @brief Inicializa os arquivos base (.dir e .dat).
 * 
 * @param out_dir Diretório onde o arquivo fica (via parametro -o).
 * @param filename_prefix Nome base do arquivo.
 * @param record_size Tamanho de cada struct guardada.
 * @param key_offset Offset da chave dentro da struct (pra comparar a string).
 * @param key_size Tamanho limite da key (ex: CEP tem 32).
 * @return Retorna o ponteiro opaco pro hash (ou NULL se deu ruim).
 */
HashFile hash_create(const char* out_dir, const char* filename_prefix, int record_size, int key_offset, int key_size);

/**
 * @brief Abre arquivos de um Hash Dinâmico já existente.
 * 
 * @param in_dir Diretório onde os arquivos estão salvos (ex: fornecido via -e). Se NULL, usa o atual.
 * @param filename_prefix Prefixo do arquivo a ser lido.
 * @return HashFile Ponteiro opaco para a estrutura ou NULL se não encontrado.
 */
HashFile hash_open(const char* in_dir, const char* filename_prefix);

/**
 * @brief Insere algo novo no Hash.
 * Ele mesmo detecta quando o bucket enche e ja faz o split / doubling.
 * 
 * @param hf Ponteiro base do hash.
 * @param reg Void pointer para a struct inteira a ser gravada no disco.
 * @return Retorna true se salvou de boa, false se a chave ja existe.
 */
bool hash_insert(HashFile hf, void* reg);

/**
 * @brief Busca um registro pelo valor da sua chave alfanumérica (string).
 * 
 * @param hf Ponteiro para o contexto do Hashfile.
 * @param key Chave string exata a ser buscada.
 * @param out_reg Ponteiro com memória alocada onde o conteúdo encontrado será copiado.
 * @return true Se o registro for encontrado e copiado.
 * @return false Se não encontrado.
 */
bool hash_search(HashFile hf, const char* key, void* out_reg);

/**
 * @brief Remove da busca (tombstone / logical delete) procurando apenas pela string-chave.
 * 
 * @param hf Referencia do hash.
 * @param key O que excluir.
 * @return false caso nada com essa key exista.
 */
bool hash_delete(HashFile hf, const char* key);

/**
 * @brief Gera o snapshot atual dos diretórios/buckets legiveis. Forma o .hfd especificado na avaliacao.
 * 
 * @param hf Referencia do hash.
 * @param out_dir Pasta.
 * @param filename_txt Nome HFD (ex: quadras, moradores).
 */
void hash_print_directory(HashFile hf, const char* out_dir, const char* filename_txt);

/**
 * @brief Fecha os arquivos, grava os metadados em disco e libera a memória.
 * 
 * @param hf Ponteiro para o contexto do Hashfile.
 */
void hash_close(HashFile hf);

#endif // HASHFILE_H
