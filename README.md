# Conjunto de Mandelbrot

Projeto desenvolvido para a disciplina de **Infraestrutura de Software**, com o objetivo de implementar a geração do conjunto de Mandelbrot em C e comparar diferentes estratégias de execução serial e paralela.

## Sobre o projeto

O programa calcula o conjunto de Mandelbrot para uma imagem com dimensões definidas pelo usuário.

Foram implementadas quatro versões:

- **Serial:** realiza o cálculo dos pixels sequencialmente.
- **OpenMP:** paraleliza o cálculo utilizando OpenMP.
- **Pthreads1:** utiliza Pthreads para dividir o cálculo do Mandelbrot em blocos de linhas.
- **Pthreads2:** utiliza Pthreads para dividir entre as threads a etapa de normalização das intensidades dos pixels.

Todas as implementações produzem a mesma imagem final, permitindo comparar seus tempos de execução.

## Compilação

Para compilar o programa:

```bash
make mandelbrot