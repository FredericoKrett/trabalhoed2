#ifndef SVG_H
#define SVG_H

#include "hashfile.h"

/**
 * @file svg.h
 * @brief Implementacao do TAD Opaco para geracao e gerenciamento de imagens SVG.
 */

typedef void* Svg;

/**
 * @brief Cria uma nova instancia de Svg alocada dinamicamente.
 * O arquivo fisico nao eh escrito imediatamente.
 */
Svg svg_create(void);

/**
 * @brief Funcoes diretas de overlay em memoria.
 * Armazena tags de string sob a forma de overlay raw, a serem adicionados
 * por ultimo para ficarem no topo da z-order do visual.
 */
void svg_add_overlay(Svg svg, const char* raw_svg_tag);

/**
 * @brief Limpa a memoria sem gerar arquivo fisico (caso ocorra erro prematuro).
 */
void svg_free(Svg svg);

/**
 * @brief Renderiza e fecha o documento no caminho especificado.
 * Desenha todas as quadras registradas e processa a fila de overlays acumulados.
 */
void svg_render_and_close(Svg svg, HashFile hf_quadras, HashFile hf_pessoas, const char* filepath);

#endif // SVG_H
