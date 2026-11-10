#include <stdio.h>
#include <string.h>
#include "backend.h"

/*
 * teste_backend.c — testes de aceitacao do backend, sem GTK.
 *
 * Roda no terminal e imprime OK ou FALHOU para cada item do
 * checklist. Deve ser executado a partir da raiz do projeto com a
 * pasta dados/ VAZIA (apague os .dat antes de rodar).
 *
 * Compilar (na raiz do projeto):
 *   gcc src/backend.c src/arquivos.c teste/teste_backend.c -Iinclude -o teste_backend
 * Executar:
 *   ./teste_backend          (Linux)   ou   teste_backend.exe (Windows)
 */

int total_ok = 0;
int total_falhou = 0;

void verificar(char descricao[], int condicao) {
    if (condicao) {
        printf("[ OK ]   %s\n", descricao);
        total_ok++;
    } else {
        printf("[FALHOU] %s\n", descricao);
        total_falhou++;
    }
}

/* converte "hh:mm" em minutos, para os testes ficarem legiveis */
int minutos(int hora, int minuto) {
    return hora * 60 + minuto;
}

int main() {
    char msg[TAM_MENSAGEM];
    char codigo[TAM_CODIGO];
    struct Ambiente lista_amb[MAX_AMBIENTES];
    struct Reserva lista_res[MAX_RESERVAS];
    int n, r;
    int id_a03, id_b01, id_reserva;

    printf("=== TESTES DO BACKEND ===\n\n");

    inicializar_backend(msg);
    n = listar_ambientes(lista_amb, MAX_AMBIENTES);
    verificar("Comeca sem nenhum ambiente (pasta dados/ vazia)", n == 0);

    /* ---------- AMBIENTES ---------- */
    strcpy(codigo, " a-03 ");
    r = cadastrar_ambiente(codigo, SALA, msg);
    verificar("Cadastrar ambiente ' a-03 ' vira 'A03'", r == 0 && strcmp(codigo, "A03") == 0);

    strcpy(codigo, "A03");
    r = cadastrar_ambiente(codigo, SALA, msg);
    verificar("Impedir codigo de ambiente duplicado", r == -1);

    strcpy(codigo, "B01");
    r = cadastrar_ambiente(codigo, SALA, msg);
    verificar("Cadastrar segundo ambiente B01", r == 0);

    strcpy(codigo, "");
    r = cadastrar_ambiente(codigo, SALA, msg);
    verificar("Rejeitar codigo vazio", r == -1);

    /* simula fechar e abrir o programa */
    inicializar_backend(msg);
    n = listar_ambientes(lista_amb, MAX_AMBIENTES);
    verificar("Encontrar os 2 ambientes apos reiniciar", n == 2);
    id_a03 = lista_amb[0].id;
    id_b01 = lista_amb[1].id;

    /* ---------- ALOCACOES ---------- */
    r = criar_alocacao("EM", "Calculo 1", "Prof. X", id_a03, SEGUNDA,
                       minutos(8, 0), minutos(10, 0), msg);
    verificar("Cadastrar alocacao sem conflito (A03 seg 08:00-10:00)", r == 0);

    r = criar_alocacao("CM", "Fisica 1", "Prof. Y", id_a03, SEGUNDA,
                       minutos(9, 0), minutos(11, 0), msg);
    verificar("Bloquear alocacao sobreposta (A03 seg 09:00-11:00)", r == -1);

    r = criar_alocacao("CM", "Fisica 1", "Prof. Y", id_a03, SEGUNDA,
                       minutos(10, 0), minutos(11, 0), msg);
    verificar("Permitir horario encostado (A03 seg 10:00-11:00)", r == 0);

    r = criar_alocacao("CM", "Fisica 1", "Prof. Y", id_a03, TERCA,
                       minutos(9, 0), minutos(11, 0), msg);
    verificar("Mesmo horario em outro dia nao conflita", r == 0);

    r = criar_alocacao("CM", "Fisica 1", "Prof. Y", id_a03, SEGUNDA,
                       minutos(10, 0), minutos(9, 0), msg);
    verificar("Rejeitar horario com fim antes do inicio", r == -1);

    r = criar_alocacao("CM", "Fisica 1", "Prof. Y", id_a03, SEGUNDA,
                       minutos(6, 0), minutos(7, 0), msg);
    verificar("Rejeitar horario fora da faixa (06:00-07:00)", r == -1);

    r = criar_alocacao("CM", "Fisica 1", "Prof. Y", 999, SEGUNDA,
                       minutos(8, 0), minutos(9, 0), msg);
    verificar("Rejeitar ambiente inexistente", r == -1);

    /* ---------- DISPONIBILIDADE ---------- */
    n = buscar_ambientes_livres(SEGUNDA, minutos(8, 0), minutos(9, 0),
                                lista_amb, MAX_AMBIENTES, msg);
    verificar("Salas livres seg 08:00-09:00: so B01", n == 1 && strcmp(lista_amb[0].codigo, "B01") == 0);

    n = buscar_ambientes_livres(SEGUNDA, minutos(11, 0), minutos(12, 0),
                                lista_amb, MAX_AMBIENTES, msg);
    verificar("Salas livres seg 11:00-12:00: A03 e B01", n == 2);

    /* ---------- RESERVAS ---------- */
    r = registrar_reserva("Monitor Z", "Monitoria", id_b01, SEGUNDA,
                          minutos(8, 0), minutos(9, 0), msg);
    verificar("Registrar reserva pendente em B01", r == 0);

    n = buscar_ambientes_livres(SEGUNDA, minutos(8, 0), minutos(9, 0),
                                lista_amb, MAX_AMBIENTES, msg);
    verificar("Reserva pendente NAO bloqueia B01", n == 1);

    n = listar_reservas(lista_res, MAX_RESERVAS);
    id_reserva = lista_res[n - 1].id;

    r = confirmar_reserva(id_reserva, msg);
    verificar("Confirmar reserva livre", r == 0);

    n = buscar_ambientes_livres(SEGUNDA, minutos(8, 0), minutos(9, 0),
                                lista_amb, MAX_AMBIENTES, msg);
    verificar("Reserva confirmada bloqueia B01", n == 0);

    r = criar_alocacao("ZZ", "Qualquer", "Prof. W", id_b01, SEGUNDA,
                       minutos(8, 30), minutos(9, 30), msg);
    verificar("Alocacao nova nao entra em cima de reserva confirmada", r == -1);

    r = confirmar_reserva(id_reserva, msg);
    verificar("Confirmar de novo devolve 'ja confirmada' sem erro", r == 0);

    r = registrar_reserva("Outro", "Palestra", id_a03, SEGUNDA,
                          minutos(8, 0), minutos(9, 0), msg);
    n = listar_reservas(lista_res, MAX_RESERVAS);
    r = confirmar_reserva(lista_res[n - 1].id, msg);
    verificar("Impedir confirmacao quando a sala esta ocupada", r == -1);

    r = cancelar_reserva(id_reserva, msg);
    n = buscar_ambientes_livres(SEGUNDA, minutos(8, 0), minutos(9, 0),
                                lista_amb, MAX_AMBIENTES, msg);
    verificar("Cancelar reserva libera B01", r == 0 && n == 1);

    r = cancelar_reserva(999, msg);
    verificar("Cancelar reserva inexistente da erro", r == -1);

    /* ---------- EXCLUSAO DE AMBIENTE ---------- */
    r = excluir_ambiente(id_a03, msg);
    verificar("Impedir exclusao de ambiente com alocacoes", r == -1);

    r = excluir_ambiente(id_b01, msg);
    verificar("Excluir ambiente sem vinculos (B01)", r == 0);

    inicializar_backend(msg);
    n = listar_ambientes(lista_amb, MAX_AMBIENTES);
    verificar("Apos reiniciar, so A03 continua ativo", n == 1 && strcmp(lista_amb[0].codigo, "A03") == 0);

    printf("\nResultado: %d OK, %d FALHOU\n", total_ok, total_falhou);
    return total_falhou;
}
