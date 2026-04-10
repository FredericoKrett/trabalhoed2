#include "quadra.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

// Estrutura oculta no arquivo .c (TAD Opaco)
struct quadra {
    int id_hash; // campo 'inteiro' usado pelo nosso Extensible Hashing (hashfile.c)
    char cep[32];
    double x;
    double y;
    double w;
    double h;
    char cfill[32];
    char cstrk[32];
    double sw;
};

// Funcao de hash djb2
static int hash_djb2(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return (int)(hash & 0x7FFFFFFF);
}

Quadra* quadra_create(const char* cep, double x, double y, double width, double height, const char* cfill, const char* cstrk, double sw) {
    Quadra* q = (Quadra*) malloc(sizeof(struct quadra));
    if (!q) return NULL;
    
    // Calcula a chave primaria inteira baseada na string.
    q->id_hash = hash_djb2(cep);
    
    strncpy(q->cep, cep, sizeof(q->cep) - 1);
    q->cep[31] = '\0';
    
    q->x = x;
    q->y = y;
    q->w = width;
    q->h = height;
    
    strncpy(q->cfill, cfill, sizeof(q->cfill) - 1);
    q->cfill[31] = '\0';
    
    strncpy(q->cstrk, cstrk, sizeof(q->cstrk) - 1);
    q->cstrk[31] = '\0';
    
    q->sw = sw;
    
    return q;
}

int quadra_get_record_size(void) {
    return (int)sizeof(struct quadra);
}

int quadra_get_key_offset(void) {
    return (int)offsetof(struct quadra, id_hash);
}

void quadra_free(Quadra* q) {
    free(q);
}

const char* quadra_get_cep(Quadra* q) { return q ? q->cep : ""; }
double quadra_get_x(Quadra* q) { return q ? q->x : 0.0; }
double quadra_get_y(Quadra* q) { return q ? q->y : 0.0; }
double quadra_get_w(Quadra* q) { return q ? q->w : 0.0; }
double quadra_get_h(Quadra* q) { return q ? q->h : 0.0; }
const char* quadra_get_cfill(Quadra* q) { return q ? q->cfill : ""; }
const char* quadra_get_cstrk(Quadra* q) { return q ? q->cstrk : ""; }
double quadra_get_sw(Quadra* q) { return q ? q->sw : 0.0; }

// Logica Geometrica do ponto de ancoragem
void quadra_get_anchor(Quadra* q, double* out_x, double* out_y) {
    if (!q) return;
    if (out_x) *out_x = q->x + q->w; // canto sudeste (x+w)
    if (out_y) *out_y = q->y + q->h; // canto sudeste (y+h, assumindo viewbox svg)
}
