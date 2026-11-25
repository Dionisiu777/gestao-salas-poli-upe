#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "planilha.h"
#include "backend.h"
int c_invalido (char c){            //Verifica se o caracter é inválido
    if (c != '\0' && strchr(" \n\r\t", c) != NULL)
        return 1;
    else
    return 0;
}
void limpar_string (char *string){
    int ini, i, fim = (int)strlen(string) - 1;
    while (c_invalido(string[ini])){
        ini++;                      //Se for inválido move o inicio da string para a próxima posição
    }
    while (c_invalido(string[fim])){
        fim--;                      //Se for inválido remove o último caracter até ser válido
    }
    while (ini <= fim){
        string[i++] = string[ini++]; //Reescreve a string ignorando os espaços em branco
    }
    string[i] = '\0';                //Fim de string
}
