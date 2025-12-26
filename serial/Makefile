default: count_models

count_models.o: main.c
	    gcc -c main.c -o count_models.o -D_GNU_SOURCE -O3

count_models: count_models.o
	    gcc count_models.o -o count_models

clean:
	-rm -f count_models.o
	-rm -f count_models
