#include <stdio.h>
#include <string.h>
#include "backend.h"

/*
 * carregar_exemplo.c — cadastra dados de exemplo para demonstracao.
 *
 * Os ambientes vem da planilha "SALAS LIVRES 2026.2" e as alocacoes
 * das primeiras linhas da aba SEG da planilha "TURMA - HORARIO 2026.2"
 * do DATP. Rode UMA vez, na raiz do projeto, com a pasta dados/ vazia.
 *
 * Compilar (na raiz do projeto):
 *   gcc src/backend.c src/arquivos.c teste/carregar_exemplo.c -Iinclude -o carregar_exemplo
 * Executar:
 *   ./carregar_exemplo        (Linux)   ou   carregar_exemplo.exe (Windows)
 */

/* horarios dos blocos da POLI, em minutos desde 00:00 */
#define H0710  430
#define H0850  530
#define H1030  630
#define H1210  730
#define H1350  830
#define H1530  930
#define H1710 1030
#define H1850 1130
#define H2030 1230
#define H2210 1330

/* procura o id de um ambiente pelo codigo, na lista devolvida pelo backend */
int id_do_codigo(struct Ambiente lista[], int n, char codigo[]) {
    int i;
    for (i = 0; i < n; i++) {
        if (strcmp(lista[i].codigo, codigo) == 0) {
            return lista[i].id;
        }
    }
    return -1;
}

int main() {
    char msg[TAM_MENSAGEM];
    char codigo[TAM_CODIGO];
    struct Ambiente lista[MAX_AMBIENTES];
    int i, n, id;

    /* salas e laboratorios da planilha SALAS LIVRES */
    char salas[][TAM_CODIGO] = {
        "A03", "B01", "B02", "B03", "B04", "C03", "C04", "C06",
        "I03", "I06", "I07", "I08", "I09", "I10", "I11", "I12",
        "I13", "I14", "I15", "J08",
        "K03", "K05", "K09", "K10", "K11", "K12", "K13", "K14",
        "K15", "K16"
    };
    char laboratorios[][TAM_CODIGO] = {
        "LIP03", "LIP05", "LIP07", "LIPECOMP", "LMC", "LMS", "LMT"
    };
    int total_salas = 30;
    int total_laboratorios = 7;

    inicializar_backend(msg);

    printf("Cadastrando ambientes...\n");
    for (i = 0; i < total_salas; i++) {
        strcpy(codigo, salas[i]);
        cadastrar_ambiente(codigo, SALA, msg);
        printf("  %-10s %s\n", codigo, msg);
    }
    for (i = 0; i < total_laboratorios; i++) {
        strcpy(codigo, laboratorios[i]);
        cadastrar_ambiente(codigo, LABORATORIO, msg);
        printf("  %-10s %s\n", codigo, msg);
    }

    n = listar_ambientes(lista, MAX_AMBIENTES);

    printf("\nCadastrando alocacoes de segunda-feira (planilha TURMA - HORARIO)...\n");

    id = id_do_codigo(lista, n, "A03");
    criar_alocacao("AM", "Metodologia da Pesquisa", "Micaella Raissa Falcao de Moura", id, SEGUNDA, H0710, H0850, msg); printf("  %s\n", msg);
    criar_alocacao("QM", "Mecanica dos Solos 2", "Stela Fucale / Antonio Carlos Costa", id, SEGUNDA, H0850, H1030, msg); printf("  %s\n", msg);
    criar_alocacao("ZM", "Gestao da Construcao Civil 2", "Adolpho Guido de Araujo", id, SEGUNDA, H1030, H1210, msg); printf("  %s\n", msg);
    criar_alocacao("F2", "Fund. da Ondulatoria e Termodinamica", "Rogerio Mendes da Silva", id, SEGUNDA, H1350, H1530, msg); printf("  %s\n", msg);
    criar_alocacao("ZB", "Formacao de Empreendedores", "Luis Cordeiro de Barros Filho", id, SEGUNDA, H1710, H1850, msg); printf("  %s\n", msg);
    criar_alocacao("SN", "Fontes Alternativas de Energia", "Alcides Codeceira / Sergio Peres Ramos", id, SEGUNDA, H1850, H2030, msg); printf("  %s\n", msg);
    criar_alocacao("TR", "Termodinamica 2", "Alcides Codeceira / Sergio Peres Ramos", id, SEGUNDA, H2030, H2210, msg); printf("  %s\n", msg);

    id = id_do_codigo(lista, n, "B01");
    criar_alocacao("EM", "Quimica", "Aleksandros El Aurens Meira de Souza", id, SEGUNDA, H0710, H1030, msg); printf("  %s\n", msg);
    criar_alocacao("CM", "Probabilidade e Estatistica", "Roberta Rodrigues Albuquerque", id, SEGUNDA, H1030, H1210, msg); printf("  %s\n", msg);
    criar_alocacao("TC", "Calculo Dif. Int. em Uma Variavel", "Lenin Alexandre de Almeida Bezerra", id, SEGUNDA, H1350, H1530, msg); printf("  %s\n", msg);
    criar_alocacao("DT", "Direito para Engenheiros", "Eduardo Cesar Ferreira de Oliveira", id, SEGUNDA, H1530, H1710, msg); printf("  %s\n", msg);
    criar_alocacao("ZN", "Gestao da Construcao Civil 2", "Adolpho Guido de Araujo", id, SEGUNDA, H1710, H1850, msg); printf("  %s\n", msg);
    criar_alocacao("IC", "Instrumentacao e Controle", "Rodrigo de Paula Monteiro", id, SEGUNDA, H1850, H2030, msg); printf("  %s\n", msg);
    criar_alocacao("LG", "Analise de Sistemas de Potencia 1", "Methodio Varejao de Godoy", id, SEGUNDA, H2030, H2210, msg); printf("  %s\n", msg);

    id = id_do_codigo(lista, n, "B02");
    criar_alocacao("4C", "Fund. da Ondulatoria e Termodinamica", "Augusto Otavio Galvao de Lima", id, SEGUNDA, H0710, H0850, msg); printf("  %s\n", msg);
    criar_alocacao("1C", "Geometria Analitica", "Deivson Cesar Silva Sales", id, SEGUNDA, H0850, H1030, msg); printf("  %s\n", msg);

    printf("\nUma reserva pendente de exemplo...\n");
    id = id_do_codigo(lista, n, "B02");
    registrar_reserva("Monitor de Calculo", "Monitoria", id, SEGUNDA, H1030, H1210, msg);
    printf("  %s\n", msg);

    printf("\nPronto. %d ambientes cadastrados.\n", n);
    return 0;
}
