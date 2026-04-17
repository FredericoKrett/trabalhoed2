#include "quadra.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

// Estrutura oculta no arquivo .c (TAD Opaco)
struct quadra {
    char cep[32];
    double x;
    double y;
    double w;
    double h;
    char cfill[32];
    char cstrk[32];
    double sw;
};

// hash_djb2 removida deste módulo. O HashFile cuida do hashing internamente com strings.

Quadra quadra_create(const char* cep, double x, double y, double width, double height, const char* cfill, const char* cstrk, double sw) {
    struct quadra* q = (struct quadra*) malloc(sizeof(struct quadra));
    if (!q) return NULL;
    
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
    return (int)offsetof(struct quadra, cep);
}

int quadra_get_key_size(void) {
    return 32; // char cep[32]
}

void quadra_free(Quadra q) {
    free(q);
}

const char* quadra_get_cep(Quadra q) { struct quadra* _q = (struct quadra*)q; return _q ? _q->cep : ""; }
double quadra_get_x(Quadra q) { struct quadra* _q = (struct quadra*)q; return _q ? _q->x : 0.0; }
double quadra_get_y(Quadra q) { struct quadra* _q = (struct quadra*)q; return _q ? _q->y : 0.0; }
double quadra_get_w(Quadra q) { struct quadra* _q = (struct quadra*)q; return _q ? _q->w : 0.0; }
double quadra_get_h(Quadra q) { struct quadra* _q = (struct quadra*)q; return _q ? _q->h : 0.0; }
const char* quadra_get_cfill(Quadra q) { struct quadra* _q = (struct quadra*)q; return _q ? _q->cfill : ""; }
const char* quadra_get_cstrk(Quadra q) { struct quadra* _q = (struct quadra*)q; return _q ? _q->cstrk : ""; }
double quadra_get_sw(Quadra q) { struct quadra* _q = (struct quadra*)q; return _q ? _q->sw : 0.0; }

// Logica Geometrica do ponto de ancoragem
void quadra_get_anchor(Quadra q, double* out_x, double* out_y) {
    struct quadra* _q = (struct quadra*)q;
    if (!_q) return;
    if (out_x) *out_x = _q->x + _q->w; // canto sudeste (x+w)
    if (out_y) *out_y = _q->y + _q->h; // canto sudeste (y+h, assumindo viewbox svg)
}
