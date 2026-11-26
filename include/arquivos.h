#ifndef ARQUIVOS_H
#define ARQUIVOS_H

#include "backend.h"

/*
 * arquivos.h — Gestao de Salas POLI-UPE
 *
 * Leitura e gravacao dos dados em arquivos binarios (fread / fwrite).
 * Cada arquivo guarda os registros de um vetor, um atras do outro,
 * exatamente como eles ficam na memoria.
 *
 * carregar_*: le o arquivo para dentro do vetor recebido.
 *             Retorna a quantidade de registros lidos.
 *             Se o arquivo ainda nao existe, retorna 0 (lista vazia).
 *
 * salvar_*:   grava o vetor inteiro no arquivo (substitui o anterior).
 *             Retorna 0 se gravou tudo, -1 se houve falha.
 */

#define ARQUIVO_AMBIENTES "dados/ambientes.dat"
#define ARQUIVO_ALOCACOES "dados/alocacoes.dat"
#define ARQUIVO_RESERVAS  "dados/reservas.dat"

int carregar_ambientes(struct Ambiente vetor[], int maximo);
int salvar_ambientes(struct Ambiente vetor[], int total);

int carregar_alocacoes(struct Alocacao vetor[], int maximo);
int salvar_alocacoes(struct Alocacao vetor[], int total);

int carregar_reservas(struct Reserva vetor[], int maximo);
int salvar_reservas(struct Reserva vetor[], int total);

#endif
