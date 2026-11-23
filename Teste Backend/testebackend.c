
#include <stdio.h>
#include <string.h>
#include "backend.h"

int main(void) {
    char mensagem[TAM_MENSAGEM];
    struct Ambiente ambientes[MAX_AMBIENTES];
    struct Alocacao alocacoes[MAX_ALOCACOES];

    int resultado;
    int id_ambiente;
    int id_alocacao;

    printf("=== TESTE DO BACKEND ===\n\n");

    /* Inicializa o sistema */
    resultado = inicializar_backend(mensagem);
    printf("Inicializacao: %s\n", mensagem);

    /* Teste 1: cadastrar ambiente */
    char codigo[] = "TESTE01";

    resultado = cadastrar_ambiente(codigo, SALA, mensagem);
    printf("\nCadastro de ambiente: %s\n", mensagem);

    if (resultado != 0) {
        printf("Nao foi possivel continuar os testes.\n");
        return 1;
    }

    /* Localiza o ambiente cadastrado */
    resultado = listar_ambientes(ambientes, MAX_AMBIENTES);
    id_ambiente = ambientes[resultado - 1].id;

    /* Teste 2: editar ambiente */
    char novo_codigo[] = "TESTE02";

    resultado = editar_ambiente(id_ambiente, novo_codigo, LABORATORIO, mensagem);
    printf("Edicao de ambiente: %s\n", mensagem);

    /* Teste 3: cadastrar alocacao */
    char turma[] = "TURMA TESTE";
    char disciplina[] = "PROGRAMACAO";
    char professor[] = "PROFESSOR TESTE";

    resultado = criar_alocacao(
        turma,
        disciplina,
        professor,
        id_ambiente,
        SEGUNDA,
        480,
        600,
        mensagem
    );

    printf("Cadastro de alocacao: %s\n", mensagem);

    if (resultado != 0) {
        printf("\nNao foi possivel cadastrar a alocacao.\n");
        return 1;
    }

    /* Localiza a alocacao cadastrada */
    resultado = listar_alocacoes(alocacoes, MAX_ALOCACOES);
    id_alocacao = alocacoes[resultado - 1].id;

    /* Teste 4: editar alocacao */
    char nova_turma[] = "TURMA NOVA";
    char nova_disciplina[] = "ESTRUTURA DE DADOS";
    char novo_professor[] = "PROFESSOR NOVO";

    resultado = editar_alocacao(
        id_alocacao,
        nova_turma,
        nova_disciplina,
        novo_professor,
        id_ambiente,
        SEGUNDA,
        600,
        720,
        mensagem
    );

    printf("Edicao de alocacao: %s\n", mensagem);

    printf("\n=== FIM DOS TESTES ===\n");

    return 0;
}

