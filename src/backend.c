#include <stdio.h>
#include <string.h>
#include "backend.h"
#include "arquivos.h"

/*
 * backend.c — Gestao de Salas POLI-UPE
 *
 * Toda a informacao do sistema fica em tres vetores de struct.
 * A interface nunca mexe neles diretamente: ela chama as funcoes
 * declaradas em backend.h, e cada funcao faz:
 *
 *   validar -> procurar no vetor com for -> alterar -> salvar -> return
 */

/* ============================================================ */
/*                     DADOS EM MEMORIA                         */
/* ============================================================ */

struct Ambiente ambientes[MAX_AMBIENTES];
int total_ambientes = 0;
int proximo_id_ambiente = 1;

struct Alocacao alocacoes[MAX_ALOCACOES];
int total_alocacoes = 0;
int proximo_id_alocacao = 1;

struct Reserva reservas[MAX_RESERVAS];
int total_reservas = 0;
int proximo_id_reserva = 1;

/* ============================================================ */
/*                       INICIALIZACAO                          */
/* ============================================================ */

int inicializar_backend(char mensagem[]) {
    int i;

    total_ambientes = carregar_ambientes(ambientes, MAX_AMBIENTES);
    total_alocacoes = carregar_alocacoes(alocacoes, MAX_ALOCACOES);
    total_reservas  = carregar_reservas(reservas, MAX_RESERVAS);

    /* o proximo id e sempre o maior id ja usado + 1 */
    proximo_id_ambiente = 1;
    for (i = 0; i < total_ambientes; i++) {
        if (ambientes[i].id >= proximo_id_ambiente) {
            proximo_id_ambiente = ambientes[i].id + 1;
        }
    }

    proximo_id_alocacao = 1;
    for (i = 0; i < total_alocacoes; i++) {
        if (alocacoes[i].id >= proximo_id_alocacao) {
            proximo_id_alocacao = alocacoes[i].id + 1;
        }
    }

    proximo_id_reserva = 1;
    for (i = 0; i < total_reservas; i++) {
        if (reservas[i].id >= proximo_id_reserva) {
            proximo_id_reserva = reservas[i].id + 1;
        }
    }

    strcpy(mensagem, "Dados carregados");
    return 0;
}

/* ============================================================ */
/*                      FUNCOES COMUNS                          */
/* ============================================================ */

/* Horario valido: comeca antes de terminar e esta dentro da faixa */
int horario_valido(int inicio, int fim) {
    if (inicio >= fim) {
        return 0;
    }
    if (inicio < HORA_MINIMA || fim > HORA_MAXIMA) {
        return 0;
    }
    return 1;
}

/* Dois intervalos se sobrepoem quando cada um comeca antes do outro
 * terminar. Horarios encostados (fim de um = inicio do outro) NAO
 * se sobrepoem.
 *   08:00-10:00 e 09:00-11:00 -> 1 (conflito)
 *   08:00-10:00 e 10:00-11:00 -> 0 (sem conflito)                   */
int horarios_se_sobrepoem(int inicioA, int fimA, int inicioB, int fimB) {
    if (inicioA < fimB && inicioB < fimA) {
        return 1;
    }
    return 0;
}

/* Verifica se o ambiente ja esta tomado nesse dia e horario, olhando
 * as alocacoes ATIVAS e as reservas CONFIRMADAS. Reserva PENDENTE
 * nao bloqueia. id_reserva_ignorar serve para a propria reserva nao
 * contar contra ela mesma na hora de confirmar (use NENHUM_ID
 * quando nao houver nada a ignorar).                                */
int ambiente_ocupado(int id_ambiente, int dia, int inicio, int fim,
                     int id_reserva_ignorar) {
    int i;

    for (i = 0; i < total_alocacoes; i++) {
        if (alocacoes[i].ativo == 1
            && alocacoes[i].id_ambiente == id_ambiente
            && alocacoes[i].dia_semana == dia
            && horarios_se_sobrepoem(inicio, fim,
                                     alocacoes[i].hora_inicio,
                                     alocacoes[i].hora_fim)) {
            return 1;
        }
    }

    for (i = 0; i < total_reservas; i++) {
        if (reservas[i].status == CONFIRMADA
            && reservas[i].id != id_reserva_ignorar
            && reservas[i].id_ambiente == id_ambiente
            && reservas[i].dia_semana == dia
            && horarios_se_sobrepoem(inicio, fim,
                                     reservas[i].hora_inicio,
                                     reservas[i].hora_fim)) {
            return 1;
        }
    }

    return 0;
}

/* Devolve a posicao do ambiente no vetor, ou -1 se nao existe */
int buscar_ambiente_por_id(int id) {
    int i;
    for (i = 0; i < total_ambientes; i++) {
        if (ambientes[i].id == id) {
            return i;
        }
    }
    return -1;
}

/* Deixa o codigo num formato unico: sem espacos, sem hifen e em
 * maiusculas. Assim "a-03", " A03 " e "A-03" viram todos "A03".
 * A conversao usa a tabela ASCII: 'a' vale 97 e 'A' vale 65.       */
void padronizar_codigo(char codigo[]) {
    char novo[TAM_CODIGO];
    int i, j = 0;
    int tamanho = strlen(codigo);

    for (i = 0; i < tamanho && j < TAM_CODIGO - 1; i++) {
        char c = codigo[i];

        if (c == ' ' || c == '-' || c == '\n') {
            continue;   /* pula o caractere */
        }
        if (c >= 'a' && c <= 'z') {
            c = c - 'a' + 'A';
        }
        novo[j] = c;
        j++;
    }
    novo[j] = '\0';

    strcpy(codigo, novo);
}

/* ============================================================ */
/*                          AMBIENTES                           */
/* ============================================================ */

int listar_ambientes(struct Ambiente saida[], int maximo) {
    int i, n = 0;
    for (i = 0; i < total_ambientes && n < maximo; i++) {
        if (ambientes[i].ativo == 1) {
            saida[n] = ambientes[i];
            n++;
        }
    }
    return n;
}

int cadastrar_ambiente(char codigo[], int tipo, char mensagem[]) {
    int i;

    padronizar_codigo(codigo);

    if (strlen(codigo) == 0) {
        strcpy(mensagem, "Informe o codigo do ambiente");
        return -1;
    }
    if (tipo != SALA && tipo != LABORATORIO) {
        strcpy(mensagem, "Tipo de ambiente invalido");
        return -1;
    }
    if (total_ambientes >= MAX_AMBIENTES) {
        strcpy(mensagem, "Limite de ambientes atingido");
        return -1;
    }

    /* codigo nao pode se repetir entre os ambientes ativos */
    for (i = 0; i < total_ambientes; i++) {
        if (ambientes[i].ativo == 1 && strcmp(ambientes[i].codigo, codigo) == 0) {
            strcpy(mensagem, "Ambiente ja cadastrado");
            return -1;
        }
    }

    ambientes[total_ambientes].id = proximo_id_ambiente;
    strcpy(ambientes[total_ambientes].codigo, codigo);
    ambientes[total_ambientes].tipo = tipo;
    ambientes[total_ambientes].ativo = 1;
    total_ambientes++;
    proximo_id_ambiente++;

    if (salvar_ambientes(ambientes, total_ambientes) != 0) {
        strcpy(mensagem, "Erro ao gravar arquivo de ambientes");
        return -1;
    }

    strcpy(mensagem, "Ambiente cadastrado");
    return 0;
}

/* faz a edição do codigo e o tipo de um ambiente ja cadastrado. */
int editar_ambiente(int id, char codigo[], int tipo, char mensagem[]) {
    int pos, i;
    struct Ambiente anterior;

    pos = buscar_ambiente_por_id(id);
    if (pos == -1 || ambientes[pos].ativo == 0) {
        strcpy(mensagem, "Ambiente nao encontrado");
        return -1;
    }

    padronizar_codigo(codigo);
    if (strlen(codigo) == 0) {
        strcpy(mensagem, "Informe o codigo do ambiente");
        return -1;
    }

    if (tipo != SALA && tipo != LABORATORIO) {
        strcpy(mensagem, "Tipo de ambiente invalido");
        return -1;
    }

    /* Verifica se o codigo ja pertence a outro ambiente ativo. */
    for (i = 0; i < total_ambientes; i++) {
        if (i != pos && ambientes[i].ativo == 1
            && strcmp(ambientes[i].codigo, codigo) == 0) {
            strcpy(mensagem, "Ja existe outro ambiente com esse codigo");
            return -1;
        }
    }

    anterior = ambientes[pos];
    strcpy(ambientes[pos].codigo, codigo);
    ambientes[pos].tipo = tipo;

    if (salvar_ambientes(ambientes, total_ambientes) != 0) {
        ambientes[pos] = anterior;
        strcpy(mensagem, "Erro ao gravar arquivo de ambientes");
        return -1;
    }

    strcpy(mensagem, "Ambiente atualizado com sucesso");
    return 0;
}

int excluir_ambiente(int id, char mensagem[]) {
    int pos, i;

    pos = buscar_ambiente_por_id(id);
    if (pos == -1 || ambientes[pos].ativo == 0) {
        strcpy(mensagem, "Ambiente nao encontrado");
        return -1;
    }

    /* nao pode excluir se ainda tem alocacao ativa */
    for (i = 0; i < total_alocacoes; i++) {
        if (alocacoes[i].ativo == 1 && alocacoes[i].id_ambiente == id) {
            strcpy(mensagem, "Ambiente possui alocacoes; nao pode excluir");
            return -1;
        }
    }

    /* nem se tem reserva pendente ou confirmada */
    for (i = 0; i < total_reservas; i++) {
        if (reservas[i].status != CANCELADA && reservas[i].id_ambiente == id) {
            strcpy(mensagem, "Ambiente possui reservas; nao pode excluir");
            return -1;
        }
    }

    /* exclusao logica: o registro continua no vetor, so fica inativo */
    ambientes[pos].ativo = 0;

    if (salvar_ambientes(ambientes, total_ambientes) != 0) {
        strcpy(mensagem, "Erro ao gravar arquivo de ambientes");
        return -1;
    }

    strcpy(mensagem, "Ambiente excluido");
    return 0;
}

/* ============================================================ */
/*                          ALOCACOES                           */
/* ============================================================ */

int listar_alocacoes(struct Alocacao saida[], int maximo) {
    int i, n = 0;
    for (i = 0; i < total_alocacoes && n < maximo; i++) {
        if (alocacoes[i].ativo == 1) {
            saida[n] = alocacoes[i];
            n++;
        }
    }
    return n;
}

int criar_alocacao(char turma[], char disciplina[], char professor[],
                   int id_ambiente, int dia, int inicio, int fim,
                   char mensagem[]) {
    int pos;

    if (strlen(turma) == 0 || strlen(disciplina) == 0) {
        strcpy(mensagem, "Informe turma e disciplina");
        return -1;
    }
    if (dia < SEGUNDA || dia > SABADO) {
        strcpy(mensagem, "Dia da semana invalido");
        return -1;
    }
    if (!horario_valido(inicio, fim)) {
        strcpy(mensagem, "Horario invalido");
        return -1;
    }

    pos = buscar_ambiente_por_id(id_ambiente);
    if (pos == -1 || ambientes[pos].ativo == 0) {
        strcpy(mensagem, "Ambiente nao encontrado");
        return -1;
    }

    if (total_alocacoes >= MAX_ALOCACOES) {
        strcpy(mensagem, "Limite de alocacoes atingido");
        return -1;
    }

    if (ambiente_ocupado(id_ambiente, dia, inicio, fim, NENHUM_ID)) {
        strcpy(mensagem, "Sala ocupada nesse periodo");
        return -1;
    }

    alocacoes[total_alocacoes].id = proximo_id_alocacao;
    strcpy(alocacoes[total_alocacoes].turma, turma);
    strcpy(alocacoes[total_alocacoes].disciplina, disciplina);
    strcpy(alocacoes[total_alocacoes].professor, professor);
    alocacoes[total_alocacoes].id_ambiente = id_ambiente;
    alocacoes[total_alocacoes].dia_semana = dia;
    alocacoes[total_alocacoes].hora_inicio = inicio;
    alocacoes[total_alocacoes].hora_fim = fim;
    alocacoes[total_alocacoes].ativo = 1;
    total_alocacoes++;
    proximo_id_alocacao++;

    if (salvar_alocacoes(alocacoes, total_alocacoes) != 0) {
        strcpy(mensagem, "Erro ao gravar arquivo de alocacoes");
        return -1;
    }

    strcpy(mensagem, "Alocacao cadastrada");
    return 0;
}

int editar_alocacao(int id, char turma[], char disciplina[],
                    char professor[], int id_ambiente, int dia,
                    int inicio, int fim, char mensagem[]) {
    int i, pos;
    struct Alocacao anterior;

    /* Procura a alocacao pelo ID */
    for (i = 0; i < total_alocacoes; i++) {
        if (alocacoes[i].id == id && alocacoes[i].ativo == 1) {
            break;
        }
    }

    if (i == total_alocacoes) {
        strcpy(mensagem, "Alocacao nao encontrada");
        return -1;
    }

    /* Valida os dados informados */
    if (strlen(turma) == 0 || strlen(disciplina) == 0) {
        strcpy(mensagem, "Informe turma e disciplina");
        return -1;
    }

    if (dia < SEGUNDA || dia > SABADO) {
        strcpy(mensagem, "Dia da semana invalido");
        return -1;
    }

    if (!horario_valido(inicio, fim)) {
        strcpy(mensagem, "Horario invalido");
        return -1;
    }

    /* Verifica se o ambiente existe e esta ativo */
    pos = buscar_ambiente_por_id(id_ambiente);

    if (pos == -1 || ambientes[pos].ativo == 0) {
        strcpy(mensagem, "Ambiente nao encontrado");
        return -1;
    }

    /* Guarda os dados antigos para possivel restauracao */
    anterior = alocacoes[i];

    /* Desativa temporariamente a propria alocacao
       para nao gerar conflito com ela mesma */
    alocacoes[i].ativo = 0;

    if (ambiente_ocupado(id_ambiente, dia, inicio, fim, NENHUM_ID)) {
        alocacoes[i] = anterior;
        strcpy(mensagem, "Sala ocupada nesse periodo");
        return -1;
    }

    /* Atualiza os dados */
    strcpy(alocacoes[i].turma, turma);
    strcpy(alocacoes[i].disciplina, disciplina);
    strcpy(alocacoes[i].professor, professor);
    alocacoes[i].id_ambiente = id_ambiente;
    alocacoes[i].dia_semana = dia;
    alocacoes[i].hora_inicio = inicio;
    alocacoes[i].hora_fim = fim;
    alocacoes[i].ativo = 1;

    /* Salva as alteracoes */
    if (salvar_alocacoes(alocacoes, total_alocacoes) != 0) {
        alocacoes[i] = anterior;
        strcpy(mensagem, "Erro ao gravar arquivo de alocacoes");
        return -1;
    }

    strcpy(mensagem, "Alocacao atualizada com sucesso");
    return 0;
}

int cancelar_alocacao(int id, char mensagem[]) {
    int i;

    for (i = 0; i < total_alocacoes; i++) {
        if (alocacoes[i].id == id && alocacoes[i].ativo == 1) {
            alocacoes[i].ativo = 0;

            if (salvar_alocacoes(alocacoes, total_alocacoes) != 0) {
                strcpy(mensagem, "Erro ao gravar arquivo de alocacoes");
                return -1;
            }

            strcpy(mensagem, "Alocacao cancelada");
            return 0;
        }
    }

    strcpy(mensagem, "Alocacao nao encontrada");
    return -1;
}

/* ============================================================ */
/*                       DISPONIBILIDADE                        */
/* ============================================================ */

int buscar_ambientes_livres(int dia, int inicio, int fim,
                            struct Ambiente saida[], int maximo,
                            char mensagem[]) {
    int i, n = 0;

    if (dia < SEGUNDA || dia > SABADO) {
        strcpy(mensagem, "Dia da semana invalido");
        return -1;
    }
    if (!horario_valido(inicio, fim)) {
        strcpy(mensagem, "Horario invalido");
        return -1;
    }

    for (i = 0; i < total_ambientes && n < maximo; i++) {
        if (ambientes[i].ativo == 1
            && !ambiente_ocupado(ambientes[i].id, dia, inicio, fim, NENHUM_ID)) {
            saida[n] = ambientes[i];
            n++;
        }
    }

    if (n == 0) {
        strcpy(mensagem, "Nenhum ambiente disponivel nesse horario");
    } else {
        strcpy(mensagem, "Busca concluida");
    }
    return n;
}

/* ============================================================ */
/*                           RESERVAS                           */
/* ============================================================ */

int listar_reservas(struct Reserva saida[], int maximo) {
    int i, n = 0;
    for (i = 0; i < total_reservas && n < maximo; i++) {
        saida[n] = reservas[i];   /* lista todas, com o status de cada uma */
        n++;
    }
    return n;
}

int registrar_reserva(char responsavel[], char finalidade[],
                      int id_ambiente, int dia, int inicio, int fim,
                      char mensagem[]) {
    int pos;

    if (strlen(responsavel) == 0 || strlen(finalidade) == 0) {
        strcpy(mensagem, "Informe responsavel e finalidade");
        return -1;
    }
    if (dia < SEGUNDA || dia > SABADO) {
        strcpy(mensagem, "Dia da semana invalido");
        return -1;
    }
    if (!horario_valido(inicio, fim)) {
        strcpy(mensagem, "Horario invalido");
        return -1;
    }

    pos = buscar_ambiente_por_id(id_ambiente);
    if (pos == -1 || ambientes[pos].ativo == 0) {
        strcpy(mensagem, "Ambiente nao encontrado");
        return -1;
    }

    if (total_reservas >= MAX_RESERVAS) {
        strcpy(mensagem, "Limite de reservas atingido");
        return -1;
    }

    /* a reserva nasce PENDENTE; conflito so e checado na confirmacao */
    reservas[total_reservas].id = proximo_id_reserva;
    strcpy(reservas[total_reservas].responsavel, responsavel);
    strcpy(reservas[total_reservas].finalidade, finalidade);
    reservas[total_reservas].id_ambiente = id_ambiente;
    reservas[total_reservas].dia_semana = dia;
    reservas[total_reservas].hora_inicio = inicio;
    reservas[total_reservas].hora_fim = fim;
    reservas[total_reservas].status = PENDENTE;
    total_reservas++;
    proximo_id_reserva++;

    if (salvar_reservas(reservas, total_reservas) != 0) {
        strcpy(mensagem, "Erro ao gravar arquivo de reservas");
        return -1;
    }

    strcpy(mensagem, "Reserva registrada como pendente");
    return 0;
}

int confirmar_reserva(int id, char mensagem[]) {
    int i, pos;

    for (i = 0; i < total_reservas; i++) {
        if (reservas[i].id == id) {

            if (reservas[i].status == CANCELADA) {
                strcpy(mensagem, "Reserva cancelada nao pode ser confirmada");
                return -1;
            }
            if (reservas[i].status == CONFIRMADA) {
                strcpy(mensagem, "Reserva ja confirmada");
                return 0;
            }

            pos = buscar_ambiente_por_id(reservas[i].id_ambiente);
            if (pos == -1 || ambientes[pos].ativo == 0) {
                strcpy(mensagem, "Ambiente da reserva nao esta mais ativo");
                return -1;
            }

            if (ambiente_ocupado(reservas[i].id_ambiente, reservas[i].dia_semana,
                                 reservas[i].hora_inicio, reservas[i].hora_fim,
                                 reservas[i].id)) {
                strcpy(mensagem, "Nao ha disponibilidade nesse horario");
                return -1;
            }

            reservas[i].status = CONFIRMADA;

            if (salvar_reservas(reservas, total_reservas) != 0) {
                strcpy(mensagem, "Erro ao gravar arquivo de reservas");
                return -1;
            }

            strcpy(mensagem, "Reserva confirmada");
            return 0;
        }
    }

    strcpy(mensagem, "Reserva nao encontrada");
    return -1;
}

int cancelar_reserva(int id, char mensagem[]) {
    int i;

    for (i = 0; i < total_reservas; i++) {
        if (reservas[i].id == id) {
            reservas[i].status = CANCELADA;

            if (salvar_reservas(reservas, total_reservas) != 0) {
                strcpy(mensagem, "Erro ao gravar arquivo de reservas");
                return -1;
            }

            strcpy(mensagem, "Reserva cancelada");
            return 0;
        }
    }

    strcpy(mensagem, "Reserva nao encontrada");
    return -1;
}
