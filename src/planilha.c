#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "planilha.h"
#include "backend.h"
//Lê arquivos em CSV

int chr_invalido (char c){            //Verifica se o caracter é inválido
    if (c != '\0' && strchr(" \n\r\t", c) != NULL)
        return 1;
    else
    return 0;
}
void limpar_string (char *string){
    int ini, i, fim = (int)strlen(string) - 1;
    while (chr_invalido(string[ini])){
        ini++;                      //Se for inválido move o inicio da string para a próxima posição
    }
    while (chr_invalido(string[fim])){
        fim--;                      //Se for inválido remove o último caracter até ser válido
    }
    while (ini <= fim){
        string[i++] = string[ini++]; //Reescreve a string ignorando os espaços em branco
    }
    string[i] = '\0';                //Fim de string
}
int prox_campo(char **ponteiro, char destino[], int tam_max) {//CSV seapara campos com ";"
    if (*ponteiro == NULL || **ponteiro == '\0') { //Verifica se há texto
        destino[0] = '\0';
        return 0;
    }
    char *separador = strchr(*ponteiro, ';'); //Procura separador ";"
    int len;
    if (separador != NULL) {
        len = (int)(separador - *ponteiro); //Número de caracteres antes do separador
        if (len >= tam_max) {
            len = tam_max - 1;
        }
        strncpy(destino, *ponteiro, len); //Copia o texto para destino
        destino[len] = '\0';
        *ponteiro = separador + 1;
    }
    else {
        strncpy(destino, *ponteiro, tam_max - 1); //Copia até o final se não existir ";"
        destino[tam_max - 1] = '\0';
        *ponteiro = NULL;
    }
    limpar_string(destino);
    return 1;
}
int string_na_lista(const char *string, const char *lista[], int total) { //
    for (int i = 0; i < total; i++) {
        if (strcmp(string, lista[i]) == 0) {
            return 1;
        }
    }
    return 0;
}