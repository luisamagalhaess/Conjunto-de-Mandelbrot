mandelbrot:
	gcc -Wall -Wextra -Wpedantic -std=c11 -fopenmp -pthread main.c -o mandelbrot

run:
	./mandelbrot 800 600 1000 4

clean:
	rm -f mandelbrot

.PHONY: mandelbrot run clean