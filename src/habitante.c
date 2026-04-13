#include "habitante.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

struct habitante {
    char cpf[32];
    char nome[64];
    char sobrenome[64];
    char sexo;
    char nascimento[16];
    
    // Dados de Moradia (Endereco do Morador)
    bool is_morador;
    char cep[32];
    char face;
    double num;
    char compl[32];
};

// djb2 function foi movida internamente para o HashFile.

Habitante* habitante_create(const char* cpf, const char* nome, const char* sobrenome, char sexo, const char* nascimento) {
    Habitante* h = (Habitante*) calloc(1, sizeof(struct habitante));
    if (!h) return NULL;
    
    strncpy(h->cpf, cpf, sizeof(h->cpf) - 1);
    h->cpf[31] = '\0';
    
    strncpy(h->nome, nome, sizeof(h->nome) - 1);
    h->nome[63] = '\0';
    
    strncpy(h->sobrenome, sobrenome, sizeof(h->sobrenome) - 1);
    h->sobrenome[63] = '\0';
    
    h->sexo = sexo;
    
    strncpy(h->nascimento, nascimento, sizeof(h->nascimento) - 1);
    h->nascimento[15] = '\0';
    
    h->is_morador = false; // Pela definicao padrao nao possui endereco ainda
    
    return h;
}

void habitante_set_endereco(Habitante* h, const char* cep, char face, double num, const char* compl) {
    if (!h) return;
    h->is_morador = true;
    strncpy(h->cep, cep, sizeof(h->cep) - 1);
    h->cep[31] = '\0';
    h->face = face;
    h->num = num;
    if (compl) {
        strncpy(h->compl, compl, sizeof(h->compl) - 1);
        h->compl[31] = '\0';
    } else {
        h->compl[0] = '\0';
    }
}

void habitante_remove_endereco(Habitante* h) {
    if (!h) return;
    h->is_morador = false;
    h->cep[0] = '\0';
}

int habitante_get_record_size(void) {
    return (int)sizeof(struct habitante);
}

int habitante_get_key_offset(void) {
    return (int)offsetof(struct habitante, cpf);
}

int habitante_get_key_size(void) {
    return 32;
}

void habitante_free(Habitante* h) {
    free(h);
}

const char* habitante_get_cpf(Habitante* h) { return h ? h->cpf : ""; }
const char* habitante_get_nome(Habitante* h) { return h ? h->nome : ""; }
const char* habitante_get_sobrenome(Habitante* h) { return h ? h->sobrenome : ""; }
char habitante_get_sexo(Habitante* h) { return h ? h->sexo : '\0'; }
const char* habitante_get_nascimento(Habitante* h) { return h ? h->nascimento : ""; }

bool habitante_is_morador(Habitante* h) { return h ? h->is_morador : false; }
const char* habitante_get_cep(Habitante* h) { return h ? h->cep : ""; }
char habitante_get_face(Habitante* h) { return h ? h->face : '\0'; }
double habitante_get_num(Habitante* h) { return h ? h->num : 0.0; }
const char* habitante_get_compl(Habitante* h) { return h ? h->compl : ""; }
