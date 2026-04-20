#include "svg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quadra.h"

// Lista simplesmente encadeada no C para Overlays dinâmicos
struct overlay_node {
    char data[256];
    struct overlay_node* next;
};

struct svg {
    struct overlay_node* overlay_head;
    struct overlay_node* overlay_tail;
};

Svg svg_create(void) {
    struct svg* s = (struct svg*)calloc(1, sizeof(struct svg));
    return s;
}

void svg_add_overlay(Svg svg_gen, const char* str) {
    if (!svg_gen || !str) return;
    struct svg* s = (struct svg*)svg_gen;

    struct overlay_node* node = (struct overlay_node*)calloc(1, sizeof(struct overlay_node));
    strncpy(node->data, str, 255);

    if (!s->overlay_head) {
        s->overlay_head = node;
        s->overlay_tail = node;
    } else {
        s->overlay_tail->next = node;
        s->overlay_tail = node;
    }
}

void svg_free(Svg svg_gen) {
    if (!svg_gen) return;
    struct svg* s = (struct svg*)svg_gen;

    struct overlay_node* curr = s->overlay_head;
    while (curr) {
        struct overlay_node* tmp = curr;
        curr = curr->next;
        free(tmp);
    }
    free(s);
}

// Callback do iterador da HashFile das Quadras
static void draw_quadra_cb(void* record, void* ctx_file) {
    FILE* f = (FILE*)ctx_file;
    Quadra q = (Quadra)record;

    const char* cep = quadra_get_cep(q);
    double x = quadra_get_x(q);
    double y = quadra_get_y(q);
    double w = quadra_get_w(q);
    double h = quadra_get_h(q);
    const char* cfill = quadra_get_cfill(q);
    const char* cstrk = quadra_get_cstrk(q);
    double sw = quadra_get_sw(q);

    // Ajuste de "none" se houver para algo padronizado no SVG, ou passar direto
    // A tag <rect> em si:
    fprintf(f, "  <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\"\n", x, y, w, h);
    fprintf(f, "        style=\"fill:%s;stroke:%s;stroke-width:%.1f\" />\n", cfill, cstrk, sw);

    // A tag <text> com o CEP centralizado. Para centralizar a fonte:
    fprintf(f, "  <text x=\"%.2f\" y=\"%.2f\" font-family=\"Arial\" font-size=\"12\"\n", x + (w / 2.0), y + (h / 2.0));
    fprintf(f, "        text-anchor=\"middle\" alignment-baseline=\"middle\" fill=\"%s\" opacity=\"0.5\">%s</text>\n", cstrk, cep);
}

void svg_render_and_close(Svg svg_gen, HashFile hf_quadras, HashFile hf_pessoas, const char* filepath) {
    if (!svg_gen || !filepath) return;
    struct svg* s = (struct svg*)svg_gen;

    FILE* f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "Erro ao abrir '%s' para escrita de modelo.\n", filepath);
        svg_free(s);
        return;
    }

    // Cabecalho Global
    fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n");

    // 1. Z-Order Layer (Fundo): Quadras
    if (hf_quadras) {
        hash_for_each(hf_quadras, draw_quadra_cb, f);
    }


    // 2. Z-Order Layer (Topo): Overlays
    struct overlay_node* curr = s->overlay_head;
    while (curr) {
        fprintf(f, "  %s\n", curr->data);
        struct overlay_node* tmp = curr;
        curr = curr->next;
        free(tmp);
    }

    fprintf(f, "</svg>\n");
    fclose(f);

    // Limpa a struct base
    s->overlay_head = NULL;
    s->overlay_tail = NULL;
    free(s);
}
