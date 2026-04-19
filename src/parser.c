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

typedef struct {
    FILE* txt;
    HashFile hf_habitantes;
    
    int hab_totais;
    int hab_m;
    int hab_f;
    int mor_totais;
    int mor_m;
    int mor_f;
    int st_totais;
    int st_m;
    int st_f;
    
    char target_cep[64];
} QryContext;

static void censo_callback(void* rec, void* ctx_gen) {
    QryContext* ctx = (QryContext*)ctx_gen;
    Habitante h = (Habitante)rec;
    ctx->hab_totais++;
    char s = habitante_get_sexo(h);
    if (s == 'M') ctx->hab_m++;
    else ctx->hab_f++;
    
    if (habitante_is_morador(h)) {
        ctx->mor_totais++;
        if (s == 'M') ctx->mor_m++;
        else ctx->mor_f++;
    } else {
        ctx->st_totais++;
        if (s == 'M') ctx->st_m++;
        else ctx->st_f++;
    }
}

static void rq_callback(void* rec, void* ctx_gen) {
    QryContext* ctx = (QryContext*)ctx_gen;
    Habitante h = (Habitante)rec;
    if (habitante_is_morador(h) && strcmp(habitante_get_cep(h), ctx->target_cep) == 0) {
        fprintf(ctx->txt, "Morador afetado (agora sem-teto) | CPF: %s | Nome: %s %s\n", 
                habitante_get_cpf(h), habitante_get_nome(h), habitante_get_sobrenome(h));
        
        habitante_remove_endereco(h);
        hash_delete(ctx->hf_habitantes, habitante_get_cpf(h)); 
        hash_insert(ctx->hf_habitantes, h);
    }
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
            char cpf[64], cep[64], compl[64];
            char face; double num;
            if (sscanf(line, "%*s %63s %63s %c %lf %63s", cpf, cep, &face, &num, compl) >= 4) {
               if (sscanf(line, "%*s %*s %*s %*c %*s %63s", compl) != 1) compl[0] = '\0';
               void* buffer = calloc(1, habitante_get_record_size());
               if(hash_search(hf_habitantes, cpf, buffer)){
                   habitante_set_endereco((Habitante)buffer, cep, face, num, compl);
                   hash_delete(hf_habitantes, cpf);
                   hash_insert(hf_habitantes, buffer);
               }
               free(buffer);
            }
        } 
        else if (strcmp(type, "nasc") == 0) {
            char cpf[64], nome[64], sobre[64], nasc[64];
            char sexo;
            if (sscanf(line, "%*s %63s %63s %63s %c %63s", cpf, nome, sobre, &sexo, nasc) == 5) {
                Habitante h = habitante_create(cpf, nome, sobre, sexo, nasc);
                hash_insert(hf_habitantes, h);
                habitante_free(h);
            }
        }
        else if (strcmp(type, "dspj") == 0) {
            char cpf[64];
            if (sscanf(line, "%*s %63s", cpf) == 1) {
                void* buffer = calloc(1, habitante_get_record_size());
                if(hash_search(hf_habitantes, cpf, buffer)){
                    Habitante h = (Habitante)buffer;
                    fprintf(f_out, "Despejo -> CPF: %s | End antigo: %s Face %c Num %.1f\n", 
                            cpf, habitante_get_cep(h), habitante_get_face(h), habitante_get_num(h));
                    habitante_remove_endereco(h);
                    hash_delete(hf_habitantes, cpf);
                    hash_insert(hf_habitantes, h);
                }
                free(buffer);
            }
        }
        else if (strcmp(type, "rip") == 0) {
            char cpf[64];
            if (sscanf(line, "%*s %63s", cpf) == 1) {
                void* buffer = calloc(1, habitante_get_record_size());
                if(hash_search(hf_habitantes, cpf, buffer)){
                    Habitante h = (Habitante)buffer;
                    fprintf(f_out, "RIP Falecimento -> CPF: %s | Nome: %s %s\n", 
                            cpf, habitante_get_nome(h), habitante_get_sobrenome(h));
                    if(habitante_is_morador(h)) {
                        fprintf(f_out, "  L Residia em: %s Face %c Num %.1f\n", habitante_get_cep(h), habitante_get_face(h), habitante_get_num(h));
                    }
                    hash_delete(hf_habitantes, cpf);
                }
                free(buffer);
            }
        }
        else if (strcmp(type, "h?") == 0) {
            char cpf[64];
            if (sscanf(line, "%*s %63s", cpf) == 1) {
                void* buffer = calloc(1, habitante_get_record_size());
                if(hash_search(hf_habitantes, cpf, buffer)){
                    Habitante h = (Habitante)buffer;
                    fprintf(f_out, "Dados h? -> %s %s, CPF %s, Sexo %c, Nasc: %s\n", 
                            habitante_get_nome(h), habitante_get_sobrenome(h), cpf, habitante_get_sexo(h), habitante_get_nascimento(h));
                    if(habitante_is_morador(h)) {
                        fprintf(f_out, "  L Endereco: CEP %s Face %c Num %.1f Compl: %s\n", 
                                habitante_get_cep(h), habitante_get_face(h), habitante_get_num(h), habitante_get_compl(h));
                    } else {
                        fprintf(f_out, "  L Sem-teto\n");
                    }
                }
                free(buffer);
            }
        }
        else if (strcmp(type, "rq") == 0) {
            char cep[64];
            if (sscanf(line, "%*s %63s", cep) == 1) {
                fprintf(f_out, "\nRemovendo Quadra: %s\n", cep);
                QryContext ctx = {0};
                ctx.txt = f_out;
                ctx.hf_habitantes = hf_habitantes;
                strncpy(ctx.target_cep, cep, 63);
                hash_for_each(hf_habitantes, rq_callback, &ctx);
                hash_delete(hf_quadras, cep);
                fprintf(f_out, "Quadra removida do HashFile!\n");
            }
        }
        else if (strcmp(type, "pq") == 0) {
            
        }
        else if (strcmp(type, "censo") == 0) {
            QryContext ctx = {0};
            hash_for_each(hf_habitantes, censo_callback, &ctx);
            fprintf(f_out, "\n--- ESTATISTICAS CENSO ---\n");
            fprintf(f_out, "Total Habitantes: %d\n", ctx.hab_totais);
            if (ctx.hab_totais > 0) {
                fprintf(f_out, "Total Moradores: %d (%.2f%%)\n", ctx.mor_totais, ctx.hab_totais ? ((double)ctx.mor_totais*100.0)/ctx.hab_totais : 0.0);
                fprintf(f_out, "Habitantes Homens: %d (%.2f%%)\n", ctx.hab_m, ctx.hab_totais ? ((double)ctx.hab_m*100.0)/ctx.hab_totais : 0.0);
                fprintf(f_out, "Habitantes Mulheres: %d (%.2f%%)\n", ctx.hab_f, ctx.hab_totais ? ((double)ctx.hab_f*100.0)/ctx.hab_totais : 0.0);
            }
            if (ctx.mor_totais > 0) {
                fprintf(f_out, "Moradores Homens: %d (%.2f%%)\n", ctx.mor_m, ctx.mor_totais ? ((double)ctx.mor_m*100.0)/ctx.mor_totais : 0.0);
                fprintf(f_out, "Moradores Mulheres: %d (%.2f%%)\n", ctx.mor_f, ctx.mor_totais ? ((double)ctx.mor_f*100.0)/ctx.mor_totais : 0.0);
            }
            fprintf(f_out, "Total Sem-Teto: %d\n", ctx.st_totais);
            if (ctx.st_totais > 0) {
                fprintf(f_out, "Sem-teto Homens: %d (%.2f%%)\n", ctx.st_m, ctx.st_totais ? ((double)ctx.st_m*100.0)/ctx.st_totais : 0.0);
                fprintf(f_out, "Sem-teto Mulheres: %d (%.2f%%)\n", ctx.st_f, ctx.st_totais ? ((double)ctx.st_f*100.0)/ctx.st_totais : 0.0);
            }
            fprintf(f_out, "--------------------------\n");
        }
    }

    fclose(f_in);
    fclose(f_out);
}
