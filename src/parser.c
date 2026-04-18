#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "quadra.h"
#include "habitante.h"

void parser_parse_geo(HashFile hf_quadras, const char* geo_filepath) {
    if (!geo_filepath || !hf_quadras) return;

    FILE* f = fopen(geo_filepath, "r");
    if (!f) {
        fprintf(stderr, "Erro ao abrir o arquivo: %s\n", geo_filepath);
        return;
    }

    char line[512];
    double current_sw = 1.0;
    char current_cstrk[64] = "black";
    char current_cfill[64] = "none";

    while (fgets(line, sizeof(line), f)) {
        char type[16];
        if (sscanf(line, "%15s", type) != 1) continue;

        if (strcmp(type, "cq") == 0) {
            char sw_str[32];
            sw_str[0] = '\0';
            current_cstrk[0] = '\0';
            current_cfill[0] = '\0';
            sscanf(line, "%*s %31s %63s %63s", sw_str, current_cstrk, current_cfill);
            
            // Remove o sufixo "px" se existir na espessura
            char *px = strstr(sw_str, "px");
            if (px) *px = '\0';
            
            if (sw_str[0] != '\0') {
                current_sw = atof(sw_str);
            }
        } else if (strcmp(type, "q") == 0) {
            char cep[64];
            double x, y, width, height;
            if (sscanf(line, "%*s %63s %lf %lf %lf %lf", cep, &x, &y, &width, &height) == 5) {
                Quadra q = quadra_create(cep, x, y, width, height, current_cfill, current_cstrk, current_sw);
                if (q) {
                    hash_insert(hf_quadras, q);
                    quadra_free(q);
                }
            }
        }
    }
    fclose(f);
}

void parser_parse_pm(HashFile hf_habitantes, const char* pm_filepath) {
    if (!pm_filepath || !hf_habitantes) return;

    FILE* f = fopen(pm_filepath, "r");
    if (!f) {
        fprintf(stderr, "Erro ao abrir o arquivo: %s\n", pm_filepath);
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char type[16];
        if (sscanf(line, "%15s", type) != 1) continue;

        if (strcmp(type, "p") == 0) {
            char cpf[64], nome[64], sobrenome[64], nascimento[64];
            char sexo;
            if (sscanf(line, "%*s %63s %63s %63s %c %63s", cpf, nome, sobrenome, &sexo, nascimento) == 5) {
                Habitante h = habitante_create(cpf, nome, sobrenome, sexo, nascimento);
                if (h) {
                    hash_insert(hf_habitantes, h);
                    habitante_free(h);
                }
            }
        } else if (strcmp(type, "m") == 0) {
            char cpf[64], cep[64], compl[64];
            char face;
            double num;
            // Considerando o formato: m <cpf> <cep> <face> <num> <compl>
            // A leitura eh controlada por sscanf para ser limpa e direta
            if (sscanf(line, "%*s %63s %63s %c %lf %63s", cpf, cep, &face, &num, compl) >= 4) {
                // Caso falhe de ler o compemento, limpa.
                if (sscanf(line, "%*s %*s %*s %*c %*s %63s", compl) != 1) {
                    compl[0] = '\0';
                }
                
                void* buffer = calloc(1, habitante_get_record_size());
                if (buffer) {
                    if (hash_search(hf_habitantes, cpf, buffer)) {
                        habitante_set_endereco((Habitante)buffer, cep, face, num, compl);
                        // Atualiza removendo e ressalvando
                        hash_delete(hf_habitantes, cpf);
                        hash_insert(hf_habitantes, buffer);
                    } else {
                        fprintf(stderr, "Aviso: Tentou vincular endereco, mas Habitante %s nao existe.\n", cpf);
                    }
                    free(buffer);
                }
            }
        }
    }
    fclose(f);
}

void parser_parse_qry(HashFile hf_quadras, HashFile hf_habitantes, const char* qry_filepath, const char* txt_out_filepath) {
    if (!qry_filepath || !txt_out_filepath || !hf_quadras || !hf_habitantes) return;

    FILE* f_in = fopen(qry_filepath, "r");
    if (!f_in) {
        fprintf(stderr, "Erro ao abrir o arquivo QRY: %s\n", qry_filepath);
        return;
    }

    FILE* f_out = fopen(txt_out_filepath, "w");
    if (!f_out) {
        fprintf(stderr, "Erro ao criar o arquivo TXT: %s\n", txt_out_filepath);
        fclose(f_in);
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), f_in)) {
        char type[16];
        if (sscanf(line, "%15s", type) != 1) continue;

        if (strcmp(type, "mud") == 0) {
            fprintf(f_out, "O comando 'mud' foi lido, mas a logica ainda sera implementada\n");
        } else if (strcmp(type, "pq") == 0) {
            fprintf(f_out, "O comando 'pq' foi lido, mas a logica ainda sera implementada\n");
        } else {
            fprintf(f_out, "Registro/Comando lido (%s), ignorando...\n", type);
        }
    }

    fclose(f_in);
    fclose(f_out);
}
