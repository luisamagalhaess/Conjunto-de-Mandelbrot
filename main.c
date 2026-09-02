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

typedef struct {
    int *iteracoes;
    int *imagem;
    int inicio;
    int fim;
    int max_iteracoes;
} DadosNormalizacao;

void *normalizarBloco(void *arg) {
    DadosNormalizacao *dados = (DadosNormalizacao *)arg;
    for (int i = dados->inicio; i < dados->fim; i++) {
        dados->imagem[i] = (dados->iteracoes[i] * 255) / dados->max_iteracoes;
    }

    return NULL;
}

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

void *calcularBloco(void *arg) {
    DadosThread *dados = (DadosThread *)arg;
    for (int y = dados->linha_inicio; y < dados->linha_fim; y++) {
        for (int x = 0; x < dados->largura; x++) {
            int iteracoes = calcularIteracoes(x, y, dados->largura, dados->altura, dados->max_iteracoes);
            dados->imagem[y * dados->largura + x] = (iteracoes * 255) / dados->max_iteracoes;
        }
    }
    return NULL;
}

int mandelbrotPthreads1(int *imagem, int largura, int altura, int max_iteracoes, int num_threads) {
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    DadosThread *dados = malloc(num_threads * sizeof(DadosThread));
    if (threads == NULL || dados == NULL) {
        free(threads);
        free(dados);
        return 0;
    }

    int linhas_por_thread = altura / num_threads;

    for (int i = 0; i < num_threads; i++) {
        dados[i].imagem = imagem;
        dados[i].largura = largura;
        dados[i].altura = altura;
        dados[i].max_iteracoes = max_iteracoes;

        dados[i].linha_inicio = i * linhas_por_thread;

        if (i == num_threads - 1) {
            dados[i].linha_fim = altura;
        } else {
            dados[i].linha_fim = (i + 1) * linhas_por_thread;
        }

        int erro = pthread_create(&threads[i], NULL, calcularBloco, &dados[i]);
        if (erro != 0) {
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            free(threads);
            free(dados);
            return 0;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(dados);

    return 1;
}

void calcularImagemIteracoes(int *iteracoes, int largura, int altura, int max_iteracoes) {
    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            iteracoes[y * largura + x] = calcularIteracoes(x, y, largura, altura, max_iteracoes);
        }
    }
}

int mandelbrotPthreads2(int *iteracoes, int *imagem, int total_pixels, int max_iteracoes, int num_threads) {
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    DadosNormalizacao *dados = malloc(num_threads * sizeof(DadosNormalizacao));
    if (threads == NULL || dados == NULL) {
        free(threads);
        free(dados);
        return 0;
    }

    int pixels_por_thread = total_pixels / num_threads;
    for (int i = 0; i < num_threads; i++) {
        dados[i].iteracoes = iteracoes;
        dados[i].imagem = imagem;
        dados[i].max_iteracoes = max_iteracoes;

        dados[i].inicio = i * pixels_por_thread;

        if (i == num_threads - 1) {
            dados[i].fim = total_pixels;
        } else {
            dados[i].fim = (i + 1) * pixels_por_thread;
        }
        int erro = pthread_create(&threads[i], NULL, normalizarBloco, &dados[i]);
        if (erro != 0) {
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            free(threads);
            free(dados);
            return 0;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    free(threads);
    free(dados);

    return 1;
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

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    if (!mandelbrotPthreads1(imagem, largura, altura, max_iteracoes, num_threads)) {
        fprintf(stderr, "Erro: falha na execucao do Pthreads1!\n");
        free(imagem);
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &fim);
    double tempo_pthreads1 = calcularTempo(inicio, fim);
    if (!salvarImagem("mandelbrot_lfm3_pthreads1.pgm", imagem, largura, altura)) {
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo Pthreads1!\n");
        free(imagem);
        return 1;
    }

    arquivo_tempo = fopen("times.txt", "a");
    if (arquivo_tempo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir times.txt!\n");
        free(imagem);
        return 1;
    }
    fprintf(arquivo_tempo, "Pthreads1: %.6f\n", tempo_pthreads1);
    fclose(arquivo_tempo);

    int *iteracoes = malloc(total_pixels * sizeof(int));

    if (iteracoes == NULL) {
        fprintf(stderr, "Erro: falha na alocacao de memoria!\n");
        free(imagem);
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    calcularImagemIteracoes(iteracoes, largura, altura, max_iteracoes);
    if (!mandelbrotPthreads2(iteracoes, imagem, total_pixels, max_iteracoes, num_threads)) {
        fprintf(stderr, "Erro: falha na execucao do Pthreads2!\n");
        free(iteracoes);
        free(imagem);
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &fim);
    double tempo_pthreads2 = calcularTempo(inicio, fim);
    if (!salvarImagem("mandelbrot_lfm3_pthreads2.pgm", imagem, largura, altura)) {
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo Pthreads2!\n");
        free(iteracoes);
        free(imagem);
        return 1;
    }
    arquivo_tempo = fopen("times.txt", "a");
    if (arquivo_tempo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir times.txt!\n");
        free(iteracoes);
        free(imagem);
        return 1;
    }
    fprintf(arquivo_tempo, "Pthreads2: %.6f\n", tempo_pthreads2);
    fclose(arquivo_tempo);
    free(iteracoes);
    free(imagem);

    return 0;
}