#ifndef PARSER_H
#define PARSER_H

#include "hashfile.h"

/**
 * @file parser.h
 * @brief Modulo de Parsing e Carga de dados (TAD Opaco).
 *
 * Responsavel por processar os arquivos e descarregar 
 * objetos recem-criados diretamente para suas respectivas memorias (Hashfiles).
 */

/**
 * @brief Le o arquivo .geo, instrucao por instrucao e salva as quadras criadas no HashFile.
 * 
 * Comando 'cq': Modifica a cor padrao das proximas quadras.
 * Comando 'q': Cria uma quadra e a salva via hash_insert usando O(1).
 * 
 * @param hf_quadras HashFile para inserir as Quadras criadas.
 * @param geo_filepath Caminho absoluto ou relativo para o arquivo .geo
 */
void parser_parse_geo(HashFile hf_quadras, const char* geo_filepath);

/**
 * @brief Le o arquivo .pm, instrucao por instrucao e salva as pessoas criadas no HashFile, resolvendo mudancas de endereco.
 * 
 * Comando 'p': Cria habitante (pessoa).
 * Comando 'm': Faz uma busca em hf_habitantes (no disco), vincula as coisas e re-insere/sobrepoe a pessoa.
 * 
 * @param hf_habitantes HashFile apontando para as estruturas populacionais.
 * @param pm_filepath Caminho absoluto ou relativo para o arquivo .pm
 */
void parser_parse_pm(HashFile hf_habitantes, const char* pm_filepath);

#endif // PARSER_H
