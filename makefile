CC = mpicc
CFLAGS = -g -Wall -O3 -W -lm

ODIR = bin
SDIR = src
IDIR = include

_DEPS = huffman.h util.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))


_OBJ_P = huffman_mpi.o
OBJ_P = $(patsubst %,$(ODIR)/%,$(_OBJ_P))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

huffman_mpi: $(OBJ_P)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f huffman_mpi $(ODIR)/*.o
