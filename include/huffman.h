#ifndef HUFFMAN_H
#define HUFFMAN_H

#define TAM_ASCII 128
#define CHUNKSIZE 100
#define NUMTHREADS 4

typedef struct node {
    int ocorrencias;
    char letra;
    struct node *esquerda, *direita;
} node_t, *node_p;

typedef struct file {
    char *source;
    int ocorrencias[TAM_ASCII];
    size_t bufsize;
} file_t, *file_p;

#endif