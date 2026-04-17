#ifndef SIG_H
#define SIG_H

/**
 * @file sig.h
 * @brief Sistema de Informacao Geografica (SIG).
 * Modulo principal responsável por gerenciar o estado da aplicacao
 * (arquivos configurados) e iniciar as estruturas sob demanda.
 */

// TAD Opaco para esconder detalhes de implementacao (como exigido).
typedef void* SIG;

/**
 * @brief Inicializa o Sistema de Informacao Geografica.
 * Aloca o TAD na memoria.
 */
SIG sig_create(void);

/**
 * @brief Define os parametros extraidos da linha de comando.
 */
void sig_set_base_entrada(SIG s, const char* dir);
void sig_set_base_saida(SIG s, const char* dir);
void sig_set_arquivo_geo(SIG s, const char* file);
void sig_set_arquivo_qry(SIG s, const char* file);
void sig_set_arquivo_pm(SIG s, const char* file);

/**
 * @brief Imprime um relatorio basico de configuracao no terminal (para auditoria e debug).
 */
void sig_print_config(SIG s);

/**
 * @brief Prepara o sistema abrindo Hashfiles que serao usados.
 * Ocorre apos configurar todos os parametros.
 */
void sig_init_files(SIG s);

/**
 * @brief Finaliza e libera todos os recursos do SIG (Hashfiles, strings alocadas).
 */
void sig_destroy(SIG s);

#endif // SIG_H
