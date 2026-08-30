#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    if (argc != 5) {
        fprintf(stderr, "Erro: quantidade incorreta de argumentos!\n");
        return 1;
    }

    int largura = atoi(argv[1]);
    int altura = atoi(argv[2]);
    int max_iteracoes = atoi(argv[3]);
    int num_threads = atoi(argv[4]);

    if (largura <= 0 || altura <= 0 || max_iteracoes <= 0 || num_threads <= 0) {
        fprintf(stderr, "Erro: todos os argumentos devem ser inteiros positivos!\n");
        return 1;
    }

    return 0;
}