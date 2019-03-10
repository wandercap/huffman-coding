/********************************************************
** Huffman Coding
**
** Opções:
**  1 - Compactar arquivo
**  2 - Descompactar arquivo
**
** Execução: ./huffman <opcao> <entrada> <saida>
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <omp.h>
#include <xmmintrin.h>
#include <pmmintrin.h>

#include "../include/huffman.h"
#include "../include/util.h"

/*!
    \fn static size_t acha_menor(node_p *ASCII, int naoRepetir)
    \param ASCII
    \param naoRepetir
    \return int
*/
static int acha_menor(node_p *ASCII, int naoRepetir) {
    int menor, i, j;

    i = j = 0;

    // Encontra primeiro caracter da tabela ASCII que estava presente no arquivo de texto
    while(ASCII[i]->ocorrencias == -1) 
        i++;

    menor = i;
    i++;

    // Encontra segundo carater da tabela ASCII que estava presente no arquivo de texto
    while(ASCII[i]->ocorrencias == -1)
        i++;

    if(naoRepetir == -1) {
        for(i = i; i < TAM_ASCII; i++) {
            if(ASCII[i]->ocorrencias != -1 && ASCII[i]->ocorrencias < ASCII[menor]->ocorrencias)
                menor = i;
        }   
    } else { // Acha segundo menor
        // Queremos o caracter com o menor número de ocorrências, com exceção de ASCII[naoRepetir]
        int temp = ASCII[naoRepetir]->ocorrencias;

        ASCII[naoRepetir]->ocorrencias = INT_MAX;

        for(i = i; i < TAM_ASCII; i++) {
            if(ASCII[i]->ocorrencias != -1 && ASCII[i]->ocorrencias < ASCII[menor]->ocorrencias)
                menor = i;
        }

        ASCII[naoRepetir]->ocorrencias = temp;
    }

    return menor;
}

/*!
    \fn static void conta_ocorrencias(int *ocorrencias, char *source, long bufsize, char *output)
    \param ocorrencias
    \param source
    \param bufsize
    \param output
  
    \brief Detecta o número de ocorrências de cada char da tabela ASCII no 
    arquivo txt de entrada e armazena em um array.
*/
static void conta_ocorrencias(int *ocorrencias, char *source, size_t bufsize, char *output) {
    size_t i;
    FILE *outputFile;

    double start, end;

    start = omp_get_wtime();

    memset(ocorrencias, 0, TAM_ASCII*sizeof(int));

    for(i=0; i<bufsize; i++)
        ocorrencias[(unsigned char)source[i]]++;

    outputFile = fopen(output, "w");
    // Escreve número de ocorrências no arquivo de saída, para ser possível remontar a árvore depois
    for(i = 0; i < TAM_ASCII-1; i++)
        fprintf(outputFile, "%d ", ocorrencias[i]);

    fprintf(outputFile, "%d\n", ocorrencias[TAM_ASCII-1]);

    fclose(outputFile);

    end = omp_get_wtime();
    printf("Tempo conta ocorrencias: %lf \n", end-start);
}

/*!
    \fn static void cria_arvore(node_p *raiz, int *ocorrencias)
    \param raiz
    \param ocorrencias
  
    \brief Cria arvore de compressão utilizando algoritmo de huffman.
*/
static void cria_arvore(node_p *raiz, int ocorrencias[TAM_ASCII]) {
    int i, subArvores, menor, segundoMenor;
    node_p ASCII[TAM_ASCII], aux;

    double start, end;

    start = omp_get_wtime();

    for(i = 0; i < TAM_ASCII; i++) {
        ASCII[i] = (node_p)malloc(sizeof(node_t));
        if(!ASCII[i])
            err_exit("Erro ao alocar memória");

        ASCII[i]->ocorrencias = ocorrencias[i];
        ASCII[i]->letra = i;
        ASCII[i]->esquerda = NULL;
        ASCII[i]->direita = NULL;
    }

    subArvores = TAM_ASCII;

    while(subArvores > 1) {
        menor = acha_menor(ASCII, -1);
        segundoMenor = acha_menor(ASCII, menor);

        aux = ASCII[menor];

        ASCII[menor] = (node_p)malloc(sizeof(node_t));
        if(!ASCII[menor])
            err_exit("Erro ao alocar memória");

        ASCII[menor]->ocorrencias = aux->ocorrencias + ASCII[segundoMenor]->ocorrencias;
        ASCII[menor]->letra = -1;
        ASCII[menor]->esquerda = ASCII[segundoMenor];
        ASCII[menor]->direita = aux;
        ASCII[segundoMenor]->ocorrencias = -1;

        subArvores--;
    }

    *raiz = ASCII[menor]; 
    
    end = omp_get_wtime();
    printf("Tempo cria arvore: %lf \n", end-start);
}

/*!
    \fn static void cria_tabela_codigos(unsigned long int *tabelaCodigos, node_p raiz, unsigned long int codigo)
    \param tabelaCodigos
    \param raiz
    \param codigo
  
    \brief Cria a codificação de cada caracter.
*/
static void cria_tabela_codigos(unsigned long int *tabelaCodigos, node_p raiz, unsigned long int codigo) {

    // Caso encontre uma folha
    if(raiz->letra != -1) {
        tabelaCodigos[(unsigned char)raiz->letra] = codigo;
    } else {
        // 1 aqui significa 0 na base binária. Seria como passar o número binário 100 para 1000
        cria_tabela_codigos(tabelaCodigos, raiz->esquerda, codigo*10+1); 

        // 2 aqui significa 1 na base binária. Seria como passar o número binário 100 para 1001
        cria_tabela_codigos(tabelaCodigos, raiz->direita, codigo*10+2); 
    }

}

/*!
    \fn static void inverte_codigos(unsigned long int *tabelaCodigos)
    \param tabelaCodigos
  
    \brief Como os códigos vêm ao contrário, é necessário invertê-los.
*/
static void inverte_codigos(unsigned long int *tabelaCodigos) {
    int i;
    unsigned long int codigoInvertido, aux;

    double start, end;

    start = omp_get_wtime();
    
    for(i = 0; i < TAM_ASCII; i++) {
        aux = tabelaCodigos[i];
        codigoInvertido = 0;

        while(aux > 0) {
            codigoInvertido = codigoInvertido * 10 + aux%10;
            aux /= 10;
        }

        tabelaCodigos[i] = codigoInvertido;
    }

    end = omp_get_wtime();
    printf("Tempo inverte codigos: %lf \n", end-start);
}

/*!
    \fn static void compacta_arquivo(char *source, long bufsize, char *output, unsigned long int *tabelaCodigos)
    \param source
    \param bufsize
    \param output
    \param tabelaCodigos
*/
static void compacta_arquivo(char *source, size_t bufsize, char *output, unsigned long int *tabelaCodigos) {
    size_t i, tam;
    char bit, c, x;
    int length, bitsRestantes;
    unsigned long int n;
    char *out;
    FILE *outputFile;

    double start, end;

    start = omp_get_wtime();

    x = 0;
    tam = 1;
    bitsRestantes = 8;

    out = (char *)malloc(sizeof(char)*tam);

    for(i=0; i<bufsize; i++) {
        c = source[i];
        //Tamanho, em bits, do código
        length = ceil(log10(tabelaCodigos[(unsigned char)c]));

        n = tabelaCodigos[(unsigned char)c];
        
        while(length > 0) {
            bit = n % 10 - 1;
            n /= 10;
            x = x | bit;
            
            bitsRestantes--;
            length--;

            if(bitsRestantes == 0) {
                out[tam-1] = x;
                tam++;
                x = 0;
                bitsRestantes = 8;

                out = (char *)realloc(out, sizeof(char)*tam);
            }

            x = x << 1;
        }
    }
    
    if(bitsRestantes != 8) {
        x = x << (bitsRestantes - 1);
        out[tam-1] = x;
    }

    outputFile = fopen(output, "a");

    fwrite(out, 1, tam, outputFile);

    fclose(outputFile);

    end = omp_get_wtime();
    printf("Tempo compacta arquivo: %lf \n", end-start);
} 

/*!
    \fn static void descompacta_arquivo(char *source, long bufsize, char *output, node_p raiz)
    \param source
    \param bufsize
    \param output
    \param raiz
*/
static void descompacta_arquivo(char *source, size_t bufsize, char *output, node_p raiz) {
    int c;
    size_t i, j;
    char bit, mascara;
    node_p atual;
    long tam;
    char *out;
    FILE *outputFile;
    
    double start, end;

    start = omp_get_wtime();

    tam = 1;
    atual = raiz;
    mascara = 1 << 7;

    out = (char *)malloc(sizeof(char)*tam);

    for(j=0; j<bufsize; j++) {
        c = source[j];
        for(i = 0; i < 8; i++) {
            bit = c & mascara;
            c = c << 1;

            if(bit == 0){
                atual = atual->esquerda;

                if(atual->letra != -1){
                    out[tam-1] = atual->letra;
                    tam++;
                    atual = raiz;

                    out = (char *)realloc(out, sizeof(char)*tam);
                }
            } else {
                atual = atual->direita;
                if (atual->letra != -1){
                    out[tam-1] = atual->letra;
                    tam++;
                    atual = raiz;

                    out = (char *)realloc(out, sizeof(char)*tam);
                }
            }
        }
    }

    end = omp_get_wtime();
    printf("Tempo descompacta arquivo: %lf \n", end-start);

    outputFile = fopen(output, "w+");

    fwrite(out, 1, tam, outputFile);

    fclose(outputFile);
} 

static file_p ler_arquivo(char *input, int option) {
    FILE *in;
    file_p file;

    file = (file_p)malloc(sizeof(file_t));

    if(!(in = fopen(input, "r"))) {
        err_exit("Erro na leitura do arquivo txt");
    }
            
	fseek(in, 0L, SEEK_END);
	file->bufsize = ftell(in);

	file->source = malloc(sizeof(char) * file->bufsize);
	fseek(in, 0L, SEEK_SET);

    if(option == 2) {
        int i;
        char buff[1024];

        for(i = 0; i < TAM_ASCII; i++) {
            if(!fscanf(in, "%d", &file->ocorrencias[i]))
                err_exit("Erro na gravação do arquivo txt");
        }

        if(!(fgets(buff, sizeof(buff), in))) {
            err_exit("Erro na leitura do arquivo txt");
        }
    }
	
    if(!(fread(file->source, sizeof(char), file->bufsize, in))) {
        err_exit("Erro na escrita do arquivo txt");
    }

	fclose(in);

    return file;
}

int main(int argc, char *argv[]) {
    if(argc < 4)
        err_exit("Número de parâmetros inválido!");

    node_p raiz;
    file_p file;
    char *output, *input;
    unsigned long int tabelaCodigos[TAM_ASCII];
    int ocorrencias[TAM_ASCII];
    double start, end;

    input = argv[2];
    output = argv[3];

    if(strcmp(argv[1], "1") == 0) {
        
        start = omp_get_wtime();

        file = ler_arquivo(input, 1);

        conta_ocorrencias(ocorrencias, file->source, file->bufsize, output);

        cria_arvore(&raiz, ocorrencias);

        double s, e;

        s = omp_get_wtime();
        cria_tabela_codigos(tabelaCodigos, raiz, 0);
        e = omp_get_wtime();
        printf("Tempo cria tabela de codigos: %lf \n", e-s);

        inverte_codigos(tabelaCodigos);

        compacta_arquivo(file->source, file->bufsize, output, tabelaCodigos);
        
        end = omp_get_wtime();
        printf("Tempo Seq: %lf \n", end-start);

    } else if (strcmp(argv[1], "2") == 0) {
        start = omp_get_wtime();

        file = ler_arquivo(input, 2);

        cria_arvore(&raiz, file->ocorrencias);

        cria_tabela_codigos(tabelaCodigos, raiz, 0);

        inverte_codigos(tabelaCodigos);

        descompacta_arquivo(file->source, file->bufsize, output, raiz);

        end = omp_get_wtime();
        printf("Tempo Seq: %lf \n", end-start);

    } else err_exit("Entrada invalida!");

    return 0;
}
