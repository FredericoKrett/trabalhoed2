#include "sig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashfile.h"
#include "parser.h"
#include "quadra.h"
#include "habitante.h"

struct sig {
    char* base_entrada;
    char* base_saida;
    char* arquivo_geo;
    char* arquivo_qry;
    char* arquivo_pm;

    HashFile hash_quadras;
    HashFile hash_pessoas;
};

static char* duplicate_string(const char* src) {
    if (!src) return NULL;
    char* dst = (char*)malloc(strlen(src) + 1);
    if (dst) strcpy(dst, src);
    return dst;
}

SIG sig_create(void) {
    struct sig* s = (struct sig*)calloc(1, sizeof(struct sig));
    return s;
}

void sig_set_base_entrada(SIG s_gen, const char* dir) {
    struct sig* s = (struct sig*) s_gen;
    if (s->base_entrada) free(s->base_entrada);
    s->base_entrada = duplicate_string(dir);
}

void sig_set_base_saida(SIG s_gen, const char* dir) {
    struct sig* s = (struct sig*) s_gen;
    if (s->base_saida) free(s->base_saida);
    s->base_saida = duplicate_string(dir);
}

void sig_set_arquivo_geo(SIG s_gen, const char* file) {
    struct sig* s = (struct sig*) s_gen;
    if (s->arquivo_geo) free(s->arquivo_geo);
    s->arquivo_geo = duplicate_string(file);
}

void sig_set_arquivo_qry(SIG s_gen, const char* file) {
    struct sig* s = (struct sig*) s_gen;
    if (s->arquivo_qry) free(s->arquivo_qry);
    s->arquivo_qry = duplicate_string(file);
}

void sig_set_arquivo_pm(SIG s_gen, const char* file) {
    struct sig* s = (struct sig*) s_gen;
    if (s->arquivo_pm) free(s->arquivo_pm);
    s->arquivo_pm = duplicate_string(file);
}

void sig_print_config(SIG s_gen) {
    struct sig* s = (struct sig*) s_gen;
    printf("=== CONFIGURACAO DO SIG ===\n");
    printf("Base Entrada (BED) : %s\n", s->base_entrada ? s->base_entrada : "(nenhuma)");
    printf("Base Saida (BSD)   : %s\n", s->base_saida ? s->base_saida : "(nenhuma)");
    printf("Arquivo .geo       : %s\n", s->arquivo_geo ? s->arquivo_geo : "(nenhum)");
    printf("Arquivo .qry       : %s\n", s->arquivo_qry ? s->arquivo_qry : "(nenhum)");
    printf("Arquivo .pm        : %s\n", s->arquivo_pm ? s->arquivo_pm : "(nenhum)");
    printf("===========================\n");
}

void sig_init_files(SIG s_gen) {
    struct sig* s = (struct sig*) s_gen;

    printf("Inicializando o motor SIG e tabelas Hash...\n");

    s->hash_quadras = hash_create(s->base_saida, "quadras", 
                                  quadra_get_record_size(), 
                                  quadra_get_key_offset(), 
                                  quadra_get_key_size());

    s->hash_pessoas = hash_create(s->base_saida, "pessoas", 
                                  habitante_get_record_size(), 
                                  habitante_get_key_offset(), 
                                  habitante_get_key_size());

    if (!s->hash_quadras || !s->hash_pessoas) {
        fprintf(stderr, "Aviso: Falha ao inicializar o armazenamento persistente.\n");
    }

    if (s->arquivo_geo && s->hash_quadras) {
        char path[1024];
        if (s->base_entrada) {
            sprintf(path, "%s/%s", s->base_entrada, s->arquivo_geo);
        } else {
            strcpy(path, s->arquivo_geo);
        }
        printf("Interpretando %s...\n", path);
        parser_parse_geo(s->hash_quadras, path);
    }

    if (s->arquivo_pm && s->hash_pessoas) {
        char path[1024];
        if (s->base_entrada) {
            sprintf(path, "%s/%s", s->base_entrada, s->arquivo_pm);
        } else {
            strcpy(path, s->arquivo_pm);
        }
        printf("Interpretando %s...\n", path);
        parser_parse_pm(s->hash_pessoas, path);
    }
}

void sig_destroy(SIG s_gen) {
    struct sig* s = (struct sig*) s_gen;
    if (!s) return;
    
    // Finaliza as conexoes com o HashFile
    if (s->hash_quadras) hash_close(s->hash_quadras);
    if (s->hash_pessoas) hash_close(s->hash_pessoas);

    free(s->base_entrada);
    free(s->base_saida);
    free(s->arquivo_geo);
    free(s->arquivo_qry);
    free(s->arquivo_pm);
    free(s);
}
