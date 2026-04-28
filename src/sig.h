#ifndef SIG_H
#define SIG_H

/**
 * @file sig.h
 * @brief Modulo central de configuracao e execucao do sistema.
 *
 * Este TAD opaco armazena os parametros da linha de comando, inicializa
 * os HashFiles, coordena o processamento dos arquivos de entrada e
 * libera os recursos ao final da execucao.
 */
typedef void* SIG;

/**
 * @brief Cria uma instancia vazia do sistema.
 */
SIG sig_create(void);

/**
 * @brief Define o diretorio-base de entrada.
 */
void sig_set_base_entrada(SIG s, const char* dir);

/**
 * @brief Define o diretorio-base de saida.
 */
void sig_set_base_saida(SIG s, const char* dir);

/**
 * @brief Define o arquivo .geo.
 */
void sig_set_arquivo_geo(SIG s, const char* file);

/**
 * @brief Define o arquivo .qry.
 */
void sig_set_arquivo_qry(SIG s, const char* file);

/**
 * @brief Define o arquivo .pm.
 */
void sig_set_arquivo_pm(SIG s, const char* file);

/**
 * @brief Exibe a configuracao carregada pela linha de comando.
 */
void sig_print_config(SIG s);

/**
 * @brief Inicializa arquivos, processa entradas e gera saidas.
 */
void sig_init_files(SIG s);

/**
 * @brief Fecha HashFiles e libera memoria.
 */
void sig_destroy(SIG s);

#endif // SIG_H
