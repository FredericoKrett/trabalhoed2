#ifndef QUADRA_H
#define QUADRA_H

/**
 * @file quadra.h
 * @brief Implementacao do dominio de Quadra da cidade usando TAD Opaco.
 */

typedef void* Quadra;

/**
 * @brief Cria uma nova quadra na memoria, pronta para ser inserida no Hash.
 */
Quadra quadra_create(const char* cep, double x, double y, double width, double height, const char* cfill, const char* cstrk, double sw);

/**
 * @brief Funcoes utilitarias de metadados para amarracao com o HashFile.
 */
int quadra_get_record_size(void);
int quadra_get_key_offset(void);
int quadra_get_key_size(void);

/**
 * @brief Libera a memoria alocada para a quadra original (caso nao use mais).
 */
void quadra_free(Quadra q);

// Getters de Leitura
const char* quadra_get_cep(Quadra q);
double quadra_get_x(Quadra q);
double quadra_get_y(Quadra q);
double quadra_get_w(Quadra q);
double quadra_get_h(Quadra q);
const char* quadra_get_cfill(Quadra q);
const char* quadra_get_cstrk(Quadra q);
double quadra_get_sw(Quadra q);

/**
 * @brief Retorna as coordenadas X e Y do ponto de ancoragem (canto sudeste).
 * Usado para calculos de distancia nos comandos do relatorio.
 */
void quadra_get_anchor(Quadra q, double* out_x, double* out_y);

#endif // QUADRA_H
