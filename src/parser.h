#ifndef PARSER_H
#define PARSER_H

#include "hashfile.h"
#include "svg.h"

/**
 * @file parser.h
 * @brief Processamento dos arquivos .geo, .pm e .qry.
 *
 * As rotinas deste modulo interpretam os comandos de entrada e aplicam
 * suas operacoes diretamente sobre os HashFiles de quadras e habitantes.
 */

/**
 * @brief Processa um arquivo .geo e persiste as quadras.
 *
 * Comandos reconhecidos:
 * - cq sw cfill cstrk
 * - q cep x y w h
 *
 * @param hf_quadras HashFile usado para armazenar quadras.
 * @param geo_filepath Caminho do arquivo .geo.
 */
void parser_parse_geo(HashFile hf_quadras, const char* geo_filepath);

/**
 * @brief Processa um arquivo .pm e persiste habitantes e moradias.
 *
 * Comandos reconhecidos:
 * - p cpf nome sobrenome sexo nasc
 * - m cpf cep face num compl
 *
 * @param hf_habitantes HashFile usado para armazenar habitantes.
 * @param pm_filepath Caminho do arquivo .pm.
 */
void parser_parse_pm(HashFile hf_habitantes, const char* pm_filepath);

/**
 * @brief Processa consultas e atualizacoes de um arquivo .qry.
 *
 * A rotina altera os HashFiles, registra o relatorio textual e adiciona
 * os elementos graficos correspondentes no SVG recebido.
 *
 * @param hf_quadras HashFile de quadras.
 * @param hf_habitantes HashFile de habitantes.
 * @param svg Instancia que acumula os elementos graficos da consulta.
 * @param qry_filepath Caminho do arquivo .qry.
 * @param txt_out_filepath Caminho do relatorio .txt de saida.
 */
void parser_parse_qry(HashFile hf_quadras, HashFile hf_habitantes, Svg svg, const char* qry_filepath, const char* txt_out_filepath);

#endif // PARSER_H
