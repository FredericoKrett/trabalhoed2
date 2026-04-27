#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "quadra.h"
#include "habitante.h"
#include "svg.h"

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
    HashFile hf_quadras;
    Svg svg;
    
    int hab_totais;
    int hab_m;
    int hab_f;
    int mor_totais;
    int mor_m;
    int mor_f;
    int st_totais;
    int st_m;
    int st_f;
    
    int moradores_N;
    int moradores_S;
    int moradores_L;
    int moradores_O;
    
    char target_cep[64];
    char cpfs_afetados[1000][64]; // Anota quem vai ser despejado
    int qtd_afetados;
} QryContext;

static void find_max_x_cb(void* rec, void* ctx) {
    Quadra q = (Quadra)rec;
    double x = quadra_get_x(q) + quadra_get_w(q);
    double* max_x = (double*)ctx;
    if (x > *max_x) *max_x = x;
}

static void draw_out_box(Svg svg, double map_max_x, int* out_count, const char* cpf, const char* text, const char* fill, const char* text_color) {
    double ox = map_max_x + 30.0 + (*out_count % 10) * 110.0;
    double oy = (*out_count / 10) * 60.0;
    char svg_txt[512];
    sprintf(svg_txt, 
        "<rect width=\"100.000000\" height=\"50.000000\" x=\"%.2f\" y=\"%.2f\" fill=\"%s\" stroke=\"black\" stroke-width=\"1\" fill-opacity=\"1.000000\" rx=\"20.000000\" ry=\"20.000000\" />\n"
        "<text x=\"%.2f\" y=\"%.2f\" fill=\"%s\" stroke=\"%s\" font-size=\"14\" text-anchor=\"middle\">%s</text>\n"
        "<text x=\"%.2f\" y=\"%.2f\" fill=\"%s\" stroke=\"%s\" font-size=\"9\" text-anchor=\"middle\">%s</text>",
        ox, oy, fill,
        ox + 50.0, oy + 12.0, text_color, text_color, text,
        ox + 50.0, oy + 28.0, text_color, text_color, cpf);
    svg_add_overlay(svg, svg_txt);
    (*out_count)++;
}

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
        
        strcpy(ctx->cpfs_afetados[ctx->qtd_afetados], habitante_get_cpf(h));
        ctx->qtd_afetados++;
    }
}

static void pq_callback(void* rec, void* ctx_gen) {
    QryContext* ctx = (QryContext*)ctx_gen;
    Habitante h = (Habitante)rec;
    
    // Conta os moradores por face
    if (habitante_is_morador(h) && strcmp(habitante_get_cep(h), ctx->target_cep) == 0) {
        char face = habitante_get_face(h);
        if (face == 'N') ctx->moradores_N++;
        else if (face == 'S') ctx->moradores_S++;
        else if (face == 'L') ctx->moradores_L++;
        else if (face == 'O') ctx->moradores_O++;
    }
}

void parser_parse_qry(HashFile hf_quadras, HashFile hf_habitantes, Svg svg, const char* qry_filepath, const char* txt_out_filepath) {
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

    double map_max_x = 0.0;
    hash_for_each(hf_quadras, find_max_x_cb, &map_max_x);
    int out_count = 0;

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
                   
                   // Evento grafico da Mudanca de Endereco
                   double dx = 0, dy = 0;
                   void* qbuf = calloc(1, quadra_get_record_size());
                   if(hash_search(hf_quadras, cep, qbuf)) {
                       quadra_get_anchor((Quadra)qbuf, &dx, &dy);
                   }
                   free(qbuf);
                   char svg_txt[256];
                   sprintf(svg_txt, "<rect x=\"%.2f\" y=\"%.2f\" width=\"20\" height=\"10\" fill=\"none\" stroke=\"red\"/><text x=\"%.2f\" y=\"%.2f\" font-size=\"7\" fill=\"red\">%s</text>", dx-10, dy-5, dx-10, dy+3, cpf);
                   svg_add_overlay(svg, svg_txt);
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
                    
                    // Evento grafico do despejo
                    if (habitante_is_morador(h)) {
                        double dx = 0, dy = 0;
                        void* qbuf = calloc(1, quadra_get_record_size());
                        if(hash_search(hf_quadras, habitante_get_cep(h), qbuf)) quadra_get_anchor((Quadra)qbuf, &dx, &dy);
                        free(qbuf);
                        char svg_txt[256];
                        sprintf(svg_txt, "<circle cx=\"%.2f\" cy=\"%.2f\" r=\"5\" fill=\"black\"/>", dx, dy);
                        svg_add_overlay(svg, svg_txt);
                    } else {
                        draw_out_box(svg, map_max_x, &out_count, cpf, "OUT", "#828062", "#F4EED7");
                    }
                    
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
                        
                        double dx = 0, dy = 0;
                        void* qbuf = calloc(1, quadra_get_record_size());
                        if(hash_search(hf_quadras, habitante_get_cep(h), qbuf)) quadra_get_anchor((Quadra)qbuf, &dx, &dy);
                        free(qbuf);
                        char svg_txt[256];
                        sprintf(svg_txt, "<path d=\"M%.2f,%.2f L%.2f,%.2f M%.2f,%.2f L%.2f,%.2f\" stroke=\"red\" stroke-width=\"2\"/>",
                                dx-5, dy, dx+5, dy, dx, dy-5, dx, dy+5);
                        svg_add_overlay(svg, svg_txt);
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
                        draw_out_box(svg, map_max_x, &out_count, cpf, ":::", "#e9afaf", "#55d400");
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
                strncpy(ctx.target_cep, cep, 63);
                
                // 1. Itera cegamente e anota
                hash_for_each(hf_habitantes, rq_callback, &ctx);
                
                // 2. Com a iteracao finalizada, modifica o disco em seguranca
                for(int i = 0; i < ctx.qtd_afetados; i++) {
                    void* buffer = calloc(1, habitante_get_record_size());
                    if(hash_search(hf_habitantes, ctx.cpfs_afetados[i], buffer)){
                        habitante_remove_endereco((Habitante)buffer);
                        hash_delete(hf_habitantes, ctx.cpfs_afetados[i]); 
                        hash_insert(hf_habitantes, buffer);
                    }
                    free(buffer);
                }
                
                // 3. Marca no SVG e exclui a quadra
                double qx = 0, qy = 0;
                void* qbuf = calloc(1, quadra_get_record_size());
                if(hash_search(hf_quadras, cep, qbuf)) quadra_get_anchor((Quadra)qbuf, &qx, &qy);
                free(qbuf);
                
                char svg_txt[256];
                sprintf(svg_txt, "<text x=\"%.2f\" y=\"%.2f\" font-family=\"Arial\" font-size=\"14\" fill=\"red\" text-anchor=\"middle\">X</text>", qx, qy);
                svg_add_overlay(svg, svg_txt);
                
                hash_delete(hf_quadras, cep);
                fprintf(f_out, "Quadra removida do HashFile!\n");
            }
        }
        else if (strcmp(type, "pq") == 0) {
            char cep[64];
            if (sscanf(line, "%*s %63s", cep) == 1) {
                QryContext ctx = {0};
                strncpy(ctx.target_cep, cep, 63);
                
                // Itera para contar
                hash_for_each(hf_habitantes, pq_callback, &ctx);
                
                double qx = 0, qy = 0, w = 0, h = 0;
                void* qbuf = calloc(1, quadra_get_record_size());
                if(hash_search(hf_quadras, cep, qbuf)) {
                    quadra_get_anchor((Quadra)qbuf, &qx, &qy);
                    w = quadra_get_w((Quadra)qbuf);
                    h = quadra_get_h((Quadra)qbuf);
                }
                free(qbuf);
                
                int total = ctx.moradores_N + ctx.moradores_S + ctx.moradores_L + ctx.moradores_O;
                
                // Injeta os textos no SVG (usando proporcoes relativas a ancora/tamanho da quadra)
                char svg_txt[512];
                sprintf(svg_txt, 
                    "<text x=\"%.2f\" y=\"%.2f\" font-size=\"10\" fill=\"blue\" text-anchor=\"middle\">%d</text>"  // Total (centro)
                    "<text x=\"%.2f\" y=\"%.2f\" font-size=\"8\" fill=\"black\" text-anchor=\"middle\">%d</text>" // N
                    "<text x=\"%.2f\" y=\"%.2f\" font-size=\"8\" fill=\"black\" text-anchor=\"middle\">%d</text>" // S
                    "<text x=\"%.2f\" y=\"%.2f\" font-size=\"8\" fill=\"black\" text-anchor=\"start\">%d</text>"  // L
                    "<text x=\"%.2f\" y=\"%.2f\" font-size=\"8\" fill=\"black\" text-anchor=\"end\">%d</text>",   // O
                    qx - w/2, qy - h/2, total,
                    qx - w/2, qy - h + 10, ctx.moradores_N,
                    qx - w/2, qy - 2, ctx.moradores_S,
                    qx - w + 2, qy - h/2, ctx.moradores_L,
                    qx - 2, qy - h/2, ctx.moradores_O
                );
                svg_add_overlay(svg, svg_txt);
            }
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
