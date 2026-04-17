#ifndef HABITANTE_H
#define HABITANTE_H

#include <stdbool.h>

/**
 * @file habitante.h
 * @brief Implementacao do dominio de Habitante e Endereco (Morador).
 */

typedef void* Habitante;

/**
 * @brief Cria um novo habitante sem endereco vinculado.
 */
Habitante habitante_create(const char* cpf, const char* nome, const char* sobrenome, char sexo, const char* nascimento);

/**
 * @brief Atualiza um habitante para vincula-lo a um endereco.
 */
void habitante_set_endereco(Habitante h, const char* cep, char face, double num, const char* compl);

/**
 * @brief Remove o endereco de um habitante.
 */
void habitante_remove_endereco(Habitante h);

/**
 * @brief Funcoes utilitarias de metadados para amarracao com o HashFile.
 */
int habitante_get_record_size(void);
int habitante_get_key_offset(void);
int habitante_get_key_size(void);

/**
 * @brief Libera a memoria.
 */
void habitante_free(Habitante h);

// Getters basicos
const char* habitante_get_cpf(Habitante h);
const char* habitante_get_nome(Habitante h);
const char* habitante_get_sobrenome(Habitante h);
char habitante_get_sexo(Habitante h);
const char* habitante_get_nascimento(Habitante h);

// Getters de Moradia
bool habitante_is_morador(Habitante h);
const char* habitante_get_cep(Habitante h);
char habitante_get_face(Habitante h);
double habitante_get_num(Habitante h);
const char* habitante_get_compl(Habitante h);

#endif // HABITANTE_H
