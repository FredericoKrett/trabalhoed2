#include "svg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "quadra.h"

// Lista simplesmente encadeada no C para Overlays dinâmicos
struct overlay_node {
    char* data;
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
    node->data = malloc(strlen(str) + 1);
    if (node->data) {
        strcpy(node->data, str);
    }

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
        if (tmp->data) free(tmp->data);
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

    fprintf(f, "  <rect id=\"%s\" x=\"%.6f\" y=\"%.6f\" width=\"%.6f\" height=\"%.6f\" fill=\"%s\" stroke=\"%s\" fill-opacity=\"0.8\" stroke-width=\"%.1fpx\" />\n",
            cep, x, y, w, h, cfill, cstrk, sw);
    fprintf(f, "  <text x=\"%.6f\" y=\"%.6f\" fill=\"%s\" stroke=\"black\" font-size=\"12\">%s</text>\n",
            x + 5.0, y + 9.0, cstrk, cep);
}

struct bbox {
    double min_x, min_y, max_x, max_y;
    int has_item;
};

static void compute_bbox_cb(void* record, void* ctx) {
    struct bbox* b = (struct bbox*)ctx;
    Quadra q = (Quadra)record;
    double x = quadra_get_x(q);
    double y = quadra_get_y(q);
    double w = quadra_get_w(q);
    double h = quadra_get_h(q);

    if (!b->has_item) {
        b->min_x = x;
        b->max_x = x + w;
        b->min_y = y;
        b->max_y = y + h;
        b->has_item = 1;
    } else {
        if (x < b->min_x) b->min_x = x;
        if (y < b->min_y) b->min_y = y;
        if (x + w > b->max_x) b->max_x = x + w;
        if (y + h > b->max_y) b->max_y = y + h;
    }
}

static void bbox_include_point(struct bbox* b, double x, double y, double padding) {
    if (!b->has_item) {
        b->min_x = x - padding;
        b->max_x = x + padding;
        b->min_y = y - padding;
        b->max_y = y + padding;
        b->has_item = 1;
        return;
    }

    if (x - padding < b->min_x) b->min_x = x - padding;
    if (y - padding < b->min_y) b->min_y = y - padding;
    if (x + padding > b->max_x) b->max_x = x + padding;
    if (y + padding > b->max_y) b->max_y = y + padding;
}

static int svg_attr_value(const char* element, const char* attr, double* value) {
    size_t attr_len = strlen(attr);
    const char* cursor = element;

    while ((cursor = strstr(cursor, attr)) != NULL) {
        const char* next = cursor + attr_len;
        int starts_attr = cursor == element || isspace((unsigned char)cursor[-1]) || cursor[-1] == '<';
        if (starts_attr && next[0] == '=' && next[1] == '"') {
            *value = strtod(next + 2, NULL);
            return 1;
        }
        cursor = next;
    }

    return 0;
}

static void expand_bbox_from_overlay_element(struct bbox* b, const char* element) {
    double x = 0.0, y = 0.0, w = 0.0, h = 0.0;
    double x1 = 0.0, y1 = 0.0, x2 = 0.0, y2 = 0.0;
    double cx = 0.0, cy = 0.0, r = 0.0;

    if (strncmp(element, "<rect", 5) == 0 &&
        svg_attr_value(element, "x", &x) &&
        svg_attr_value(element, "y", &y) &&
        svg_attr_value(element, "width", &w) &&
        svg_attr_value(element, "height", &h)) {
        bbox_include_point(b, x, y, 0.0);
        bbox_include_point(b, x + w, y + h, 0.0);
        return;
    }

    if (strncmp(element, "<line", 5) == 0 &&
        svg_attr_value(element, "x1", &x1) &&
        svg_attr_value(element, "y1", &y1) &&
        svg_attr_value(element, "x2", &x2) &&
        svg_attr_value(element, "y2", &y2)) {
        bbox_include_point(b, x1, y1, 5.0);
        bbox_include_point(b, x2, y2, 5.0);
        return;
    }

    if (strncmp(element, "<circle", 7) == 0 &&
        svg_attr_value(element, "cx", &cx) &&
        svg_attr_value(element, "cy", &cy)) {
        if (!svg_attr_value(element, "r", &r)) r = 0.0;
        bbox_include_point(b, cx, cy, r + 2.0);
        return;
    }

    if (strncmp(element, "<text", 5) == 0 &&
        svg_attr_value(element, "x", &x) &&
        svg_attr_value(element, "y", &y)) {
        bbox_include_point(b, x, y, 5.0);
    }
}

static void expand_bbox_from_overlay(struct bbox* b, const char* overlay) {
    const char* cursor = overlay;
    while ((cursor = strchr(cursor, '<')) != NULL) {
        expand_bbox_from_overlay_element(b, cursor);
        cursor++;
    }
}

void svg_render_and_close(Svg svg_gen, HashFile hf_quadras, HashFile hf_pessoas, const char* filepath) {
    if (!svg_gen || !filepath) return;
    (void)hf_pessoas;
    struct svg* s = (struct svg*)svg_gen;

    FILE* f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "Erro ao abrir '%s' para escrita de modelo.\n", filepath);
        svg_free(s);
        return;
    }

    struct bbox b = {0, 0, 0, 0, 0};
    if (hf_quadras) {
        hash_for_each(hf_quadras, compute_bbox_cb, &b);
    }

    if (b.has_item) {
        // Adds margin
        double margin = 100.0;
        b.min_x -= margin;
        b.min_y -= margin;
        b.max_x += margin;
        b.max_y += margin;
    } else {
        b.min_x = 0; b.min_y = 0; b.max_x = 1000; b.max_y = 1000;
    }

    // Expande a area visivel tambem para textos, linhas e marcadores dinamicos.
    struct overlay_node* curr_b = s->overlay_head;
    while (curr_b) {
        expand_bbox_from_overlay(&b, curr_b->data);
        curr_b = curr_b->next;
    }

    fprintf(f, "<svg viewBox=\"%.2f %.2f %.2f %.2f\" xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n", 
            b.min_x, b.min_y, (b.max_x - b.min_x), (b.max_y - b.min_y));

    if (hf_quadras) {
        hash_for_each(hf_quadras, draw_quadra_cb, f);
    }

    struct overlay_node* curr = s->overlay_head;
    while (curr) {
        if (curr->data) {
            fprintf(f, "  %s\n", curr->data);
            free(curr->data);
        }
        struct overlay_node* tmp = curr;
        curr = curr->next;
        free(tmp);
    }

    fprintf(f, "</svg>\n");
    fclose(f);

    s->overlay_head = NULL;
    s->overlay_tail = NULL;
    free(s);
}
