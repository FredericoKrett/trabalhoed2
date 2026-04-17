#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sig.h"

int main(int argc, char* argv[]) {
    // Instancia o novo modulo SIG para gerenciar o projeto
    SIG sig = sig_create();

    // Faz o parse manual de cada string dos argumentos
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
            sig_set_base_entrada(sig, argv[++i]);
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            sig_set_arquivo_geo(sig, argv[++i]);
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            sig_set_base_saida(sig, argv[++i]);
        } else if (strcmp(argv[i], "-q") == 0 && i + 1 < argc) {
            sig_set_arquivo_qry(sig, argv[++i]);
        } else if (strcmp(argv[i], "-pm") == 0 && i + 1 < argc) {
            sig_set_arquivo_pm(sig, argv[++i]);
        } else {
            fprintf(stderr, "Aviso: Argumento desconhecido ou mal formatado: %s\n", argv[i]);
        }
    }

    // Mostra como ficou a extracao pro avaliador ver
    sig_print_config(sig);

    // (Futuro) - Construtor de arquivos e Hash
    sig_init_files(sig);

    // Finaliza (libera TODA a memoria, importante)
    sig_destroy(sig);

    return 0;
}
