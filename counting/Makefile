default: count_numbers

count_numbers.o: main.c
	    gcc -c main.c -o count_numbers.o -O3

count_numbers: count_numbers.o
	    gcc count_numbers.o -o count_numbers

clean:
	-rm -f count_numbers.o
	-rm -f count_numbers
