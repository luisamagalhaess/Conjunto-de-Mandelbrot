#include <stdio.h>
#include <stdlib.h>

#define REAL_MIN -2.0
#define REAL_MAX 1.0
#define IMAG_MIN -1.5
#define IMAG_MAX 1.5

void converterPixelParaComplexo(int x, int y, int largura, int altura, double *c_real, double *c_imag) {
    *c_real = REAL_MIN + ((double)x / (largura - 1)) * (REAL_MAX - REAL_MIN);
    *c_imag = IMAG_MIN + ((double)y / (altura - 1)) * (IMAG_MAX - IMAG_MIN);
}

int main(int argc, char *argv[]) {

    if (argc != 5) {
        fprintf(stderr, "Erro: quantidade incorreta de argumentos!\n");
        return 1;
    }

    int largura = atoi(argv[1]);
    int altura = atoi(argv[2]);
    int max_iteracoes = atoi(argv[3]);
    int num_threads = atoi(argv[4]);

    if (largura < 2 || altura < 2 || max_iteracoes <= 0 || num_threads <= 0) {
        fprintf(stderr, "Erro: todos os argumentos devem ser inteiros positivos!\n");
        return 1;
    }

    return 0;
}