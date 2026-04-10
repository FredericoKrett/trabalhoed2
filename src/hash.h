#ifndef HASH_EXTENSIVEL_H
#define HASH_EXTENSIVEL_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/*
    POR FAVOR, NOTEM QUE ESTE COMENTARIO INTRODUTORIO E' MUITO IMPORTANTE!

    Um Arquivo Hash Extensível armazena registros (de tamanho fixo) no sistema de arquivos,
    utilizando um diretório em memória para endereçar buckets (páginas de disco) baseando-se em
    uma chave primária inteira (id).
*/

// tipo opaco para o contexto do hash file ocultando a implementacao
typedef struct HashFile HashFile;

/*
    Cria os arquivos necessários para o Hash File (".dir" e ".dat").
    - filename_prefix: Prefixo do nome dos arquivos.
    - record_size: Tamanho, em bytes, de cada registro que será armazenado.
    - key_offset: Deslocamento (offset) em bytes a partir do início do registro até o campo chave primária (que deve ser do tipo "int").
    Retorna true se criado com sucesso, false caso contrário.
*/
bool hash_create(const char* filename_prefix, int record_size, int key_offset);

/*
    Abre os arquivos de um Hash Extensível já existente no disco para leitura ou manipulacao.
    Retorna o ponteiro de contexto opaco HashFile* se sucesso, ou NULL em caso de erro.
*/
HashFile* hash_open(const char* filename_prefix);

/*
    Insere o registro apontado por "reg" no arquivo hash.
    O módulo utilizará "key_offset", informado na criação, para extrair a chave do registro que servirá de endereço do bucket.
    Retorna true se a inserção for bem-sucedida, false se a chave já existir ou ocorrer outro erro.
*/
bool hash_insert(HashFile* hf, void* reg);

/*
    Busca por um registro que contenha a chave (key) fornecida.
    Se encontrado, copia o conteúdo para a memória apontada por "out_reg". O buffer de destino "out_reg" precisa estar alocado pelo chamador com tamanho "record_size".
    Retorna true se o registro foi encontrado, false do contrário (ou configuração incorreta).
*/
bool hash_search(HashFile* hf, int key, void* out_reg);

/*
    Remove o registro cuja chave seja "key" do arquivo hash.
    Retorna true se sucesso na remoção, false se a chave não existir.
*/
bool hash_delete(HashFile* hf, int key);

/*
    Salva o estado atual do diretorio no disco e fecha os arquivos (".dir" e ".dat").
    Libera a estrutura de memória do "HashFile".
*/
void hash_close(HashFile* hf);

#endif // HASH_EXTENSIVEL_H
