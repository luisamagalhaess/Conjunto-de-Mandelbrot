#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <pthread.h>

#define REAL_MIN -2.0
#define REAL_MAX 1.0
#define IMAG_MIN -1.5
#define IMAG_MAX 1.5

typedef struct {
    int *imagem;
    int largura;
    int altura;
    int max_iteracoes;
    int linha_inicio;
    int linha_fim;
} DadosThread;

void converterPixelParaComplexo(int x, int y, int largura, int altura, double *c_real, double *c_imag) {
    *c_real = REAL_MIN + ((double)x / (largura - 1)) * (REAL_MAX - REAL_MIN);
    *c_imag = IMAG_MIN + ((double)y / (altura - 1)) * (IMAG_MAX - IMAG_MIN);
}

int calcularIteracoes(int x, int y, int largura, int altura, int max_iteracoes) {
    double c_real;
    double c_imag;
    converterPixelParaComplexo(x, y, largura, altura, &c_real, &c_imag);

    double z_real = 0.0;
    double z_imag = 0.0;
    int iteracao = 0;

    while (z_real * z_real + z_imag * z_imag <= 4.0 && iteracao < max_iteracoes) {
        double novo_real = z_real * z_real - z_imag * z_imag + c_real;
        
        double novo_imag = 2.0 * z_real * z_imag + c_imag;

        z_real = novo_real;
        z_imag = novo_imag;

        iteracao++;
    }

    return iteracao;
}

void mandelbrotSerial( int *imagem, int largura, int altura, int max_iteracoes) {
    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            int iteracoes = calcularIteracoes( x, y, largura, altura, max_iteracoes);
            imagem[y * largura + x] = (iteracoes * 255) / max_iteracoes;
        }
    }
}

void mandelbrotOpenMP(int *imagem, int largura, int altura, int max_iteracoes, int num_threads) {
    #pragma omp parallel for num_threads(num_threads)

    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            int iteracoes = calcularIteracoes(x, y, largura, altura, max_iteracoes);

            imagem[y * largura + x] = (iteracoes * 255) / max_iteracoes;
        }
    }
}

int salvarImagem(const char *nome_arquivo, int *imagem, int largura, int altura) {
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL) {
        return 0;
    }

    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            fprintf(arquivo, "%d", imagem[y * largura + x]);
            if (x < largura - 1) {
                fprintf(arquivo, " ");
            }
        }
        fprintf(arquivo, "\n");
    }

    fclose(arquivo);

    return 1;
}

double calcularTempo(struct timespec inicio, struct timespec fim) {
    double segundos = fim.tv_sec - inicio.tv_sec;
    double nanossegundos = (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;
    return segundos + nanossegundos;
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

    int total_pixels = largura * altura;
    int *imagem = malloc(total_pixels * sizeof(int));
    if (imagem == NULL) {
        fprintf(stderr, "Erro: falha na alocacao de memoria!\n");
        return 1;
    }

    struct timespec inicio;
    struct timespec fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    mandelbrotSerial(imagem, largura, altura, max_iteracoes);

    clock_gettime(CLOCK_MONOTONIC, &fim);
    double tempo_serial = calcularTempo(inicio, fim);
    
    if (!salvarImagem("mandelbrot_lfm3_serial.pgm", imagem, largura, altura)) {
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo de saida!\n");
        free(imagem);
        return 1;
    }

    FILE *arquivo_tempo = fopen("times.txt", "w");
    if (arquivo_tempo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar times.txt!\n");
        free(imagem);
        return 1;
    }
    fprintf(arquivo_tempo, "Serial: %.6f\n", tempo_serial);
    fclose(arquivo_tempo);
    
    clock_gettime(CLOCK_MONOTONIC, &inicio);
    mandelbrotOpenMP(imagem, largura, altura, max_iteracoes, num_threads);
    clock_gettime(CLOCK_MONOTONIC, &fim);
    double tempo_openmp = calcularTempo(inicio, fim);
    
    if (!salvarImagem("mandelbrot_lfm3_openmp.pgm", imagem, largura, altura)) {
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo OpenMP!\n");
        free(imagem);
        return 1;
    }
    
    arquivo_tempo = fopen("times.txt", "a");
    if (arquivo_tempo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir times.txt!\n");
        free(imagem);
        return 1;
    }
    fprintf(arquivo_tempo, "OpenMP: %.6f\n", tempo_openmp);
    fclose(arquivo_tempo);
    free(imagem);

    return 0;
}