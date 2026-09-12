
GCC=g++

CFLAGS=-O0 -Wall -Wextra -pedantic-errors -g -march=native

snasm.bin: always snasm.cpp
	$(GCC) snasm.cpp $(CFLAGS) -o bin/snasm.bin

always: 
	mkdir -p bin obj

clean:
	rm -rf bin obj

.PHONY: clean always