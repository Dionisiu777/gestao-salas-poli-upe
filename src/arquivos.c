#include <stdio.h>
#include "arquivos.h"

/*
 * arquivos.c — Gestao de Salas POLI-UPE
 *
 * As seis funcoes seguem o mesmo esqueleto:
 *
 *   1. fopen  -> abre o arquivo (modo "rb" para ler, "wb" para gravar)
 *   2. testa se fopen devolveu NULL
 *   3. fread em um while  (ler)  /  fwrite uma vez  (gravar)
 *   4. fclose
 *
 * So muda o nome do arquivo e o tipo da struct.
 */

/* ============================================================ */
/*                          AMBIENTES                           */
/* ============================================================ */

int carregar_ambientes(struct Ambiente vetor[], int maximo) {
    FILE *fptr;
    int total = 0;

    fptr = fopen(ARQUIVO_AMBIENTES, "rb");
    if (fptr == NULL) {
        return 0;   /* arquivo ainda nao existe: lista vazia */
    }

    /* le um registro por vez ate o fim do arquivo (ou ate encher o vetor) */
    while (total < maximo && fread(&vetor[total], sizeof(struct Ambiente), 1, fptr) == 1) {
        total++;
    }

    fclose(fptr);
    return total;
}

int salvar_ambientes(struct Ambiente vetor[], int total) {
    FILE *fptr;
    int gravados;

    fptr = fopen(ARQUIVO_AMBIENTES, "wb");
    if (fptr == NULL) {
        return -1;
    }

    /* grava o vetor inteiro de uma vez: total registros do tamanho da struct */
    gravados = fwrite(vetor, sizeof(struct Ambiente), total, fptr);
    fclose(fptr);

    if (gravados != total) {
        return -1;
    }
    return 0;
}

/* ============================================================ */
/*                          ALOCACOES                           */
/* ============================================================ */

int carregar_alocacoes(struct Alocacao vetor[], int maximo) {
    FILE *fptr;
    int total = 0;

    fptr = fopen(ARQUIVO_ALOCACOES, "rb");
    if (fptr == NULL) {
        return 0;
    }

    while (total < maximo && fread(&vetor[total], sizeof(struct Alocacao), 1, fptr) == 1) {
        total++;
    }

    fclose(fptr);
    return total;
}

int salvar_alocacoes(struct Alocacao vetor[], int total) {
    FILE *fptr;
    int gravados;

    fptr = fopen(ARQUIVO_ALOCACOES, "wb");
    if (fptr == NULL) {
        return -1;
    }

    gravados = fwrite(vetor, sizeof(struct Alocacao), total, fptr);
    fclose(fptr);

    if (gravados != total) {
        return -1;
    }
    return 0;
}

/* ============================================================ */
/*                           RESERVAS                           */
/* ============================================================ */

int carregar_reservas(struct Reserva vetor[], int maximo) {
    FILE *fptr;
    int total = 0;

    fptr = fopen(ARQUIVO_RESERVAS, "rb");
    if (fptr == NULL) {
        return 0;
    }

    while (total < maximo && fread(&vetor[total], sizeof(struct Reserva), 1, fptr) == 1) {
        total++;
    }

    fclose(fptr);
    return total;
}

int salvar_reservas(struct Reserva vetor[], int total) {
    FILE *fptr;
    int gravados;

    fptr = fopen(ARQUIVO_RESERVAS, "wb");
    if (fptr == NULL) {
        return -1;
    }

    gravados = fwrite(vetor, sizeof(struct Reserva), total, fptr);
    fclose(fptr);

    if (gravados != total) {
        return -1;
    }
    return 0;
}
