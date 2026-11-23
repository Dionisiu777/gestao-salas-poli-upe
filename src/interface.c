#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include "interface.h"
#include "backend.h"

/*
 * interface.c — Gestao de Salas POLI-UPE
 *
 * Monta a janela principal e as quatro telas usando GTK 4.
 * Toda validacao de regra e gravacao em arquivo e feita chamando
 * as funcoes declaradas em backend.h.
 */

/* ============================================================ */
/*                 VARIAVEIS E WIDGETS GLOBAIS                  */
/* ============================================================ */

GtkWidget *stack_telas;

/* Vetores auxiliares para saber qual item o usuario clicou na lista */
struct Ambiente v_ambientes[MAX_AMBIENTES];
int qtd_ambientes = 0;

struct Alocacao v_alocacoes[MAX_ALOCACOES];
int qtd_alocacoes = 0;

struct Reserva v_reservas[MAX_RESERVAS];
int qtd_reservas = 0;

/* Widgets da tela de Ambientes */
GtkWidget *lista_amb, *form_amb, *conf_amb, *msg_amb;
GtkWidget *ent_cod_amb, *drop_tipo_amb;
int id_edicao_amb = 0;

/* Widgets da tela de Disponibilidade */
GtkWidget *lista_disp, *msg_disp;
GtkWidget *drop_dia_disp, *ent_ini_disp, *ent_fim_disp;

/* Widgets da tela de Alocacoes */
GtkWidget *lista_aloc, *form_aloc, *conf_aloc, *msg_aloc, *box_alt_aloc;
GtkWidget *ent_turma, *ent_disc, *ent_prof, *ent_ini_aloc, *ent_fim_aloc;
GtkWidget *drop_amb_aloc, *drop_dia_aloc, *drop_tipo_alt;
int id_edicao_aloc = 0;

/* Widgets da tela de Reservas */
GtkWidget *lista_res, *form_res, *conf_res, *msg_res;
GtkWidget *ent_resp, *ent_fin, *ent_ini_res, *ent_fim_res;
GtkWidget *drop_amb_res, *drop_dia_res;

const char *nomes_dias[] = {"Segunda", "Terca", "Quarta", "Quinta", "Sexta", "Sabado", NULL};
const char *siglas_dias[] = {"SEG", "TER", "QUA", "QUI", "SEX", "SAB"};

/* ============================================================ */
/*                      FUNCOES AUXILIARES                      */
/* ============================================================ */

/* Converte texto "07:10" para minutos (430). Retorna -1 se invalido */
int texto_para_minutos(const char *texto) {
    int h, m;
    char sobra;
    if (sscanf(texto, "%d:%d%c", &h, &m, &sobra) != 2) {
        return -1;
    }
    if (h < 0 || h > 23 || m < 0 || m > 59) {
        return -1;
    }
    return h * 60 + m;
}

/* Converte minutos (430) para texto "07:10" */
void minutos_para_texto(int min, char saida[]) {
    sprintf(saida, "%02d:%02d", min / 60, min % 60);
}

/* Remove todas as linhas de um GtkListBox */
void limpar_lista(GtkWidget *lista) {
    GtkWidget *filho = gtk_widget_get_first_child(lista);
    while (filho != NULL) {
        GtkWidget *prox = gtk_widget_get_next_sibling(filho);
        gtk_list_box_remove(GTK_LIST_BOX(lista), filho);
        filho = prox;
    }
}

/* Insere uma linha de texto simples dentro de um GtkListBox */
void adicionar_linha(GtkWidget *lista, char texto[]) {
    GtkWidget *linha = gtk_list_box_row_new();
    GtkWidget *label = gtk_label_new(texto);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_margin_top(label, 5);
    gtk_widget_set_margin_bottom(label, 5);
    gtk_widget_set_margin_start(label, 8);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(linha), label);
    gtk_list_box_append(GTK_LIST_BOX(lista), linha);
}

/* Retorna o indice (0, 1, 2...) da linha selecionada ou -1 se nada foi clicado */
int linha_selecionada(GtkWidget *lista) {
    GtkListBoxRow *linha = gtk_list_box_get_selected_row(GTK_LIST_BOX(lista));
    if (linha == NULL) {
        return -1;
    }
    return gtk_list_box_row_get_index(linha);
}

/* Procura o codigo de uma sala (ex: "A03") pelo id_ambiente */
void buscar_codigo_sala(int id_ambiente, char saida[]) {
    int i;
    for (i = 0; i < qtd_ambientes; i++) {
        if (v_ambientes[i].id == id_ambiente) {
            strcpy(saida, v_ambientes[i].codigo);
            return;
        }
    }
    sprintf(saida, "ID %d", id_ambiente);
}

/* Atualiza as opcoes de ambientes nos seletores (DropDown) de Alocacoes e Reservas */
void atualizar_combos_ambientes(void) {
    int i;
    GtkStringList *modelo = gtk_string_list_new(NULL);
    qtd_ambientes = listar_ambientes(v_ambientes, MAX_AMBIENTES);

    for (i = 0; i < qtd_ambientes; i++) {
        gtk_string_list_append(modelo, v_ambientes[i].codigo);
    }
    gtk_drop_down_set_model(GTK_DROP_DOWN(drop_amb_aloc), G_LIST_MODEL(modelo));
    gtk_drop_down_set_model(GTK_DROP_DOWN(drop_amb_res), G_LIST_MODEL(modelo));
    g_object_unref(modelo);
}

/* ============================================================ */
/*                      TELA DE AMBIENTES                       */
/* ============================================================ */

void carregar_tela_ambientes(void) {
    int i;
    char linha[200];

    limpar_lista(lista_amb);
    qtd_ambientes = listar_ambientes(v_ambientes, MAX_AMBIENTES);

    for (i = 0; i < qtd_ambientes; i++) {
        sprintf(linha, "ID: %d  |  Codigo: %-8s  |  Tipo: %s",
                v_ambientes[i].id,
                v_ambientes[i].codigo,
                v_ambientes[i].tipo == LABORATORIO ? "Laboratorio" : "Sala");
        adicionar_linha(lista_amb, linha);
    }
}

void clicou_novo_amb(GtkWidget *b, gpointer d) {
    id_edicao_amb = 0;
    gtk_editable_set_text(GTK_EDITABLE(ent_cod_amb), "");
    gtk_drop_down_set_selected(GTK_DROP_DOWN(drop_tipo_amb), SALA);
    gtk_widget_set_visible(conf_amb, FALSE);
    gtk_widget_set_visible(form_amb, TRUE);
    gtk_label_set_text(GTK_LABEL(msg_amb), "Preencha os dados do novo ambiente.");
}

void clicou_editar_amb(GtkWidget *b, gpointer d) {
    int pos = linha_selecionada(lista_amb);
    if (pos == -1) {
        gtk_label_set_text(GTK_LABEL(msg_amb), "Selecione um ambiente na lista primeiro.");
        return;
    }
    id_edicao_amb = v_ambientes[pos].id;
    gtk_editable_set_text(GTK_EDITABLE(ent_cod_amb), v_ambientes[pos].codigo);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(drop_tipo_amb), v_ambientes[pos].tipo);
    gtk_widget_set_visible(conf_amb, FALSE);
    gtk_widget_set_visible(form_amb, TRUE);
    gtk_label_set_text(GTK_LABEL(msg_amb), "Altere os dados e clique em Salvar.");
}

void clicou_salvar_amb(GtkWidget *b, gpointer d) {
    char codigo[TAM_CODIGO], mensagem[TAM_MENSAGEM];
    int tipo = gtk_drop_down_get_selected(GTK_DROP_DOWN(drop_tipo_amb));
    int res;

    strcpy(codigo, gtk_editable_get_text(GTK_EDITABLE(ent_cod_amb)));
    if (strlen(codigo) == 0) {
        gtk_label_set_text(GTK_LABEL(msg_amb), "Informe o codigo do ambiente.");
        return;
    }

    if (id_edicao_amb == 0) {
        res = cadastrar_ambiente(codigo, tipo, mensagem);
    } else {
        res = editar_ambiente(id_edicao_amb, codigo, tipo, mensagem);
    }

    gtk_label_set_text(GTK_LABEL(msg_amb), mensagem);
    if (res == 0) {
        carregar_tela_ambientes();
        gtk_widget_set_visible(form_amb, FALSE);
    }
}

void clicou_excluir_amb(GtkWidget *b, gpointer d) {
    if (linha_selecionada(lista_amb) == -1) {
        gtk_label_set_text(GTK_LABEL(msg_amb), "Selecione um ambiente na lista para excluir.");
        return;
    }
    gtk_widget_set_visible(form_amb, FALSE);
    gtk_widget_set_visible(conf_amb, TRUE);
}

void confirmar_exclusao_amb(GtkWidget *b, gpointer d) {
    int pos = linha_selecionada(lista_amb);
    char mensagem[TAM_MENSAGEM];

    if (pos != -1 && excluir_ambiente(v_ambientes[pos].id, mensagem) == 0) {
        carregar_tela_ambientes();
    }
    gtk_label_set_text(GTK_LABEL(msg_amb), mensagem);
    gtk_widget_set_visible(conf_amb, FALSE);
}

void cancelar_exclusao_amb(GtkWidget *b, gpointer d) {
    gtk_widget_set_visible(conf_amb, FALSE);
    gtk_label_set_text(GTK_LABEL(msg_amb), "Exclusao cancelada.");
}

/* ============================================================ */
/*                   TELA DE DISPONIBILIDADE                    */
/* ============================================================ */

void clicou_pesquisar_disp(GtkWidget *b, gpointer d) {
    struct Ambiente livres[MAX_AMBIENTES];
    char mensagem[TAM_MENSAGEM], linha[200];
    int dia = gtk_drop_down_get_selected(GTK_DROP_DOWN(drop_dia_disp));
    int ini = texto_para_minutos(gtk_editable_get_text(GTK_EDITABLE(ent_ini_disp)));
    int fim = texto_para_minutos(gtk_editable_get_text(GTK_EDITABLE(ent_fim_disp)));
    int qtd, i;

    limpar_lista(lista_disp);
    if (ini == -1 || fim == -1) {
        gtk_label_set_text(GTK_LABEL(msg_disp), "Digite os horarios no formato HH:MM (ex: 07:10).");
        return;
    }

    qtd = buscar_ambientes_livres(dia, ini, fim, livres, MAX_AMBIENTES, mensagem);
    gtk_label_set_text(GTK_LABEL(msg_disp), mensagem);

    for (i = 0; i < qtd; i++) {
        sprintf(linha, "Codigo: %-8s  |  Tipo: %s  [DISPONIVEL]",
                livres[i].codigo,
                livres[i].tipo == LABORATORIO ? "Laboratorio" : "Sala");
        adicionar_linha(lista_disp, linha);
    }
}

/* ============================================================ */
/*                      TELA DE ALOCACOES                       */
/* ============================================================ */

void carregar_tela_alocacoes(void) {
    int i;
    char sala[TAM_CODIGO], ini[10], fim[10], linha[350];

    limpar_lista(lista_aloc);
    atualizar_combos_ambientes();
    qtd_alocacoes = listar_alocacoes(v_alocacoes, MAX_ALOCACOES);

    for (i = 0; i < qtd_alocacoes; i++) {
        buscar_codigo_sala(v_alocacoes[i].id_ambiente, sala);
        minutos_para_texto(v_alocacoes[i].hora_inicio, ini);
        minutos_para_texto(v_alocacoes[i].hora_fim, fim);

        sprintf(linha, "ID: %d | Sala: %-6s | %s %s-%s | Turma: %-4s | %s (%s)",
                v_alocacoes[i].id, sala, siglas_dias[v_alocacoes[i].dia_semana],
                ini, fim, v_alocacoes[i].turma,
                v_alocacoes[i].disciplina, v_alocacoes[i].professor);
        adicionar_linha(lista_aloc, linha);
    }
}

void clicou_nova_aloc(GtkWidget *b, gpointer d) {
    if (qtd_ambientes == 0) {
        gtk_label_set_text(GTK_LABEL(msg_aloc), "Cadastre um ambiente primeiro.");
        return;
    }
    id_edicao_aloc = 0;
    gtk_editable_set_text(GTK_EDITABLE(ent_turma), "");
    gtk_editable_set_text(GTK_EDITABLE(ent_disc), "");
    gtk_editable_set_text(GTK_EDITABLE(ent_prof), "");
    gtk_editable_set_text(GTK_EDITABLE(ent_ini_aloc), "");
    gtk_editable_set_text(GTK_EDITABLE(ent_fim_aloc), "");
    gtk_widget_set_visible(box_alt_aloc, FALSE);
    gtk_widget_set_visible(conf_aloc, FALSE);
    gtk_widget_set_visible(form_aloc, TRUE);
    gtk_label_set_text(GTK_LABEL(msg_aloc), "Preencha os campos da nova alocacao.");
}

void clicou_editar_aloc(GtkWidget *b, gpointer d) {
    int pos = linha_selecionada(lista_aloc);
    char ini[10], fim[10];
    int i;

    if (pos == -1) {
        gtk_label_set_text(GTK_LABEL(msg_aloc), "Selecione uma alocacao na lista primeiro.");
        return;
    }

    id_edicao_aloc = v_alocacoes[pos].id;
    minutos_para_texto(v_alocacoes[pos].hora_inicio, ini);
    minutos_para_texto(v_alocacoes[pos].hora_fim, fim);

    gtk_editable_set_text(GTK_EDITABLE(ent_turma), v_alocacoes[pos].turma);
    gtk_editable_set_text(GTK_EDITABLE(ent_disc), v_alocacoes[pos].disciplina);
    gtk_editable_set_text(GTK_EDITABLE(ent_prof), v_alocacoes[pos].professor);
    gtk_editable_set_text(GTK_EDITABLE(ent_ini_aloc), ini);
    gtk_editable_set_text(GTK_EDITABLE(ent_fim_aloc), fim);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(drop_dia_aloc), v_alocacoes[pos].dia_semana);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(drop_tipo_alt), 0);

    for (i = 0; i < qtd_ambientes; i++) {
        if (v_ambientes[i].id == v_alocacoes[pos].id_ambiente) {
            gtk_drop_down_set_selected(GTK_DROP_DOWN(drop_amb_aloc), i);
            break;
        }
    }

    gtk_widget_set_visible(box_alt_aloc, TRUE);
    gtk_widget_set_visible(conf_aloc, FALSE);
    gtk_widget_set_visible(form_aloc, TRUE);
}

void clicou_salvar_aloc(GtkWidget *b, gpointer d) {
    char turma[TAM_CODIGO], disc[TAM_TEXTO], prof[TAM_TEXTO], mensagem[TAM_MENSAGEM];
    int pos_amb = gtk_drop_down_get_selected(GTK_DROP_DOWN(drop_amb_aloc));
    int dia = gtk_drop_down_get_selected(GTK_DROP_DOWN(drop_dia_aloc));
    int ini = texto_para_minutos(gtk_editable_get_text(GTK_EDITABLE(ent_ini_aloc)));
    int fim = texto_para_minutos(gtk_editable_get_text(GTK_EDITABLE(ent_fim_aloc)));
    int res;

    strcpy(turma, gtk_editable_get_text(GTK_EDITABLE(ent_turma)));
    strcpy(disc, gtk_editable_get_text(GTK_EDITABLE(ent_disc)));
    strcpy(prof, gtk_editable_get_text(GTK_EDITABLE(ent_prof)));

    if (strlen(turma) == 0 || strlen(disc) == 0 || strlen(prof) == 0 || ini == -1 || fim == -1) {
        gtk_label_set_text(GTK_LABEL(msg_aloc), "Preencha todos os campos corretamente (horario HH:MM).");
        return;
    }

    if (id_edicao_aloc > 0 && gtk_drop_down_get_selected(GTK_DROP_DOWN(drop_tipo_alt)) == 1) {
        gtk_label_set_text(GTK_LABEL(msg_aloc), "Definir datas antes de habilitar alteracao temporaria.");
        return;
    }

    if (id_edicao_aloc == 0) {
        res = criar_alocacao(turma, disc, prof, v_ambientes[pos_amb].id, dia, ini, fim, mensagem);
    } else {
        res = editar_alocacao(id_edicao_aloc, turma, disc, prof, v_ambientes[pos_amb].id, dia, ini, fim, mensagem);
    }

    gtk_label_set_text(GTK_LABEL(msg_aloc), mensagem);
    if (res == 0) {
        carregar_tela_alocacoes();
        gtk_widget_set_visible(form_aloc, FALSE);
    }
}

void clicou_cancelar_aloc(GtkWidget *b, gpointer d) {
    if (linha_selecionada(lista_aloc) == -1) {
        gtk_label_set_text(GTK_LABEL(msg_aloc), "Selecione uma alocacao na lista para cancelar.");
        return;
    }
    gtk_widget_set_visible(form_aloc, FALSE);
    gtk_widget_set_visible(conf_aloc, TRUE);
}

void confirmar_cancelamento_aloc(GtkWidget *b, gpointer d) {
    int pos = linha_selecionada(lista_aloc);
    char mensagem[TAM_MENSAGEM];

    if (pos != -1 && cancelar_alocacao(v_alocacoes[pos].id, mensagem) == 0) {
        carregar_tela_alocacoes();
    }
    gtk_label_set_text(GTK_LABEL(msg_aloc), mensagem);
    gtk_widget_set_visible(conf_aloc, FALSE);
}

void desistir_cancelamento_aloc(GtkWidget *b, gpointer d) {
    gtk_widget_set_visible(conf_aloc, FALSE);
}

/* ============================================================ */
/*                       TELA DE RESERVAS                       */
/* ============================================================ */

void carregar_tela_reservas(void) {
    int i;
    char sala[TAM_CODIGO], ini[10], fim[10], linha[350];
    const char *nomes_status[] = {"PENDENTE", "CONFIRMADA", "CANCELADA"};

    limpar_lista(lista_res);
    atualizar_combos_ambientes();
    qtd_reservas = listar_reservas(v_reservas, MAX_RESERVAS);

    for (i = 0; i < qtd_reservas; i++) {
        buscar_codigo_sala(v_reservas[i].id_ambiente, sala);
        minutos_para_texto(v_reservas[i].hora_inicio, ini);
        minutos_para_texto(v_reservas[i].hora_fim, fim);

        sprintf(linha, "ID: %d | [%s] | Sala: %-6s | %s %s-%s | %s (%s)",
                v_reservas[i].id, nomes_status[v_reservas[i].status],
                sala, siglas_dias[v_reservas[i].dia_semana],
                ini, fim, v_reservas[i].responsavel, v_reservas[i].finalidade);
        adicionar_linha(lista_res, linha);
    }
}

void clicou_nova_res(GtkWidget *b, gpointer d) {
    if (qtd_ambientes == 0) {
        gtk_label_set_text(GTK_LABEL(msg_res), "Cadastre um ambiente primeiro.");
        return;
    }
    gtk_editable_set_text(GTK_EDITABLE(ent_resp), "");
    gtk_editable_set_text(GTK_EDITABLE(ent_fin), "");
    gtk_editable_set_text(GTK_EDITABLE(ent_ini_res), "");
    gtk_editable_set_text(GTK_EDITABLE(ent_fim_res), "");
    gtk_widget_set_visible(conf_res, FALSE);
    gtk_widget_set_visible(form_res, TRUE);
    gtk_label_set_text(GTK_LABEL(msg_res), "Preencha os dados da solicitacao de reserva.");
}

void clicou_salvar_res(GtkWidget *b, gpointer d) {
    char resp[TAM_TEXTO], fin[TAM_TEXTO], mensagem[TAM_MENSAGEM];
    int pos_amb = gtk_drop_down_get_selected(GTK_DROP_DOWN(drop_amb_res));
    int dia = gtk_drop_down_get_selected(GTK_DROP_DOWN(drop_dia_res));
    int ini = texto_para_minutos(gtk_editable_get_text(GTK_EDITABLE(ent_ini_res)));
    int fim = texto_para_minutos(gtk_editable_get_text(GTK_EDITABLE(ent_fim_res)));

    strcpy(resp, gtk_editable_get_text(GTK_EDITABLE(ent_resp)));
    strcpy(fin, gtk_editable_get_text(GTK_EDITABLE(ent_fin)));

    if (strlen(resp) == 0 || strlen(fin) == 0 || ini == -1 || fim == -1) {
        gtk_label_set_text(GTK_LABEL(msg_res), "Preencha todos os campos corretamente (horario HH:MM).");
        return;
    }

    if (registrar_reserva(resp, fin, v_ambientes[pos_amb].id, dia, ini, fim, mensagem) == 0) {
        carregar_tela_reservas();
        gtk_widget_set_visible(form_res, FALSE);
    }
    gtk_label_set_text(GTK_LABEL(msg_res), mensagem);
}

void clicou_confirmar_res(GtkWidget *b, gpointer d) {
    int pos = linha_selecionada(lista_res);
    char mensagem[TAM_MENSAGEM];

    if (pos == -1) {
        gtk_label_set_text(GTK_LABEL(msg_res), "Selecione uma reserva na lista para confirmar.");
        return;
    }
    if (confirmar_reserva(v_reservas[pos].id, mensagem) == 0) {
        carregar_tela_reservas();
    }
    gtk_label_set_text(GTK_LABEL(msg_res), mensagem);
}

void clicou_cancelar_res(GtkWidget *b, gpointer d) {
    if (linha_selecionada(lista_res) == -1) {
        gtk_label_set_text(GTK_LABEL(msg_res), "Selecione uma reserva na lista para cancelar.");
        return;
    }
    gtk_widget_set_visible(form_res, FALSE);
    gtk_widget_set_visible(conf_res, TRUE);
}

void confirmar_cancelamento_res(GtkWidget *b, gpointer d) {
    int pos = linha_selecionada(lista_res);
    char mensagem[TAM_MENSAGEM];

    if (pos != -1 && cancelar_reserva(v_reservas[pos].id, mensagem) == 0) {
        carregar_tela_reservas();
    }
    gtk_label_set_text(GTK_LABEL(msg_res), mensagem);
    gtk_widget_set_visible(conf_res, FALSE);
}

void desistir_cancelamento_res(GtkWidget *b, gpointer d) {
    gtk_widget_set_visible(conf_res, FALSE);
}

/* ============================================================ */
/*                  NAVEGACAO E CONSTRUCAO GTK                  */
/* ============================================================ */

void ir_para_tela(GtkWidget *botao, gpointer destino) {
    char *nome = (char *)destino;

    if (strcmp(nome, "ambientes") == 0) {
        carregar_tela_ambientes();
        gtk_widget_set_visible(form_amb, FALSE);
        gtk_widget_set_visible(conf_amb, FALSE);
    } else if (strcmp(nome, "alocacoes") == 0) {
        carregar_tela_alocacoes();
        gtk_widget_set_visible(form_aloc, FALSE);
        gtk_widget_set_visible(conf_aloc, FALSE);
    } else if (strcmp(nome, "disponibilidade") == 0) {
        limpar_lista(lista_disp);
    } else if (strcmp(nome, "reservas") == 0) {
        carregar_tela_reservas();
        gtk_widget_set_visible(form_res, FALSE);
        gtk_widget_set_visible(conf_res, FALSE);
    }

    gtk_stack_set_visible_child_name(GTK_STACK(stack_telas), nome);
}

/* Cria uma barra simples de confirmacao (Sim / Nao) */
GtkWidget* criar_barra_confirma(GCallback fn_sim, GCallback fn_nao) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *lbl = gtk_label_new("Tem certeza que deseja confirmar esta operacao?");
    GtkWidget *b_sim = gtk_button_new_with_label("Sim");
    GtkWidget *b_nao = gtk_button_new_with_label("Nao");

    g_signal_connect(b_sim, "clicked", fn_sim, NULL);
    g_signal_connect(b_nao, "clicked", fn_nao, NULL);
    gtk_box_append(GTK_BOX(box), lbl);
    gtk_box_append(GTK_BOX(box), b_sim);
    gtk_box_append(GTK_BOX(box), b_nao);
    gtk_widget_set_visible(box, FALSE);
    return box;
}

GtkWidget* montar_tela_ambientes(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *botoes = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *scroll = gtk_scrolled_window_new();
    GtkWidget *b_novo = gtk_button_new_with_label("Novo");
    GtkWidget *b_edit = gtk_button_new_with_label("Editar");
    GtkWidget *b_excl = gtk_button_new_with_label("Excluir");
    GtkWidget *b_volt = gtk_button_new_with_label("Voltar");
    GtkWidget *b_salv = gtk_button_new_with_label("Salvar");
    const char *tipos[] = {"Sala", "Laboratorio", NULL};

    gtk_widget_set_margin_top(box, 16); gtk_widget_set_margin_bottom(box, 16);
    gtk_widget_set_margin_start(box, 16); gtk_widget_set_margin_end(box, 16);

    g_signal_connect(b_novo, "clicked", G_CALLBACK(clicou_novo_amb), NULL);
    g_signal_connect(b_edit, "clicked", G_CALLBACK(clicou_editar_amb), NULL);
    g_signal_connect(b_excl, "clicked", G_CALLBACK(clicou_excluir_amb), NULL);
    g_signal_connect(b_volt, "clicked", G_CALLBACK(ir_para_tela), "menu");
    g_signal_connect(b_salv, "clicked", G_CALLBACK(clicou_salvar_amb), NULL);

    gtk_box_append(GTK_BOX(botoes), b_novo);
    gtk_box_append(GTK_BOX(botoes), b_edit);
    gtk_box_append(GTK_BOX(botoes), b_excl);
    gtk_box_append(GTK_BOX(botoes), b_volt);

    msg_amb = gtk_label_new("Gerenciamento de Ambientes");
    conf_amb = criar_barra_confirma(G_CALLBACK(confirmar_exclusao_amb), G_CALLBACK(cancelar_exclusao_amb));

    lista_amb = gtk_list_box_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), lista_amb);

    form_amb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    ent_cod_amb = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(ent_cod_amb), "Codigo (ex: A03)");
    drop_tipo_amb = gtk_drop_down_new_from_strings(tipos);

    gtk_box_append(GTK_BOX(form_amb), gtk_label_new("Codigo:"));
    gtk_box_append(GTK_BOX(form_amb), ent_cod_amb);
    gtk_box_append(GTK_BOX(form_amb), gtk_label_new("Tipo:"));
    gtk_box_append(GTK_BOX(form_amb), drop_tipo_amb);
    gtk_box_append(GTK_BOX(form_amb), b_salv);
    gtk_widget_set_visible(form_amb, FALSE);

    gtk_box_append(GTK_BOX(box), botoes);
    gtk_box_append(GTK_BOX(box), msg_amb);
    gtk_box_append(GTK_BOX(box), conf_amb);
    gtk_box_append(GTK_BOX(box), scroll);
    gtk_box_append(GTK_BOX(box), form_amb);
    return box;
}

GtkWidget* montar_tela_disponibilidade(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *filtros = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *scroll = gtk_scrolled_window_new();
    GtkWidget *b_pesq = gtk_button_new_with_label("Pesquisar");
    GtkWidget *b_volt = gtk_button_new_with_label("Voltar");

    gtk_widget_set_margin_top(box, 16); gtk_widget_set_margin_bottom(box, 16);
    gtk_widget_set_margin_start(box, 16); gtk_widget_set_margin_end(box, 16);

    drop_dia_disp = gtk_drop_down_new_from_strings(nomes_dias);
    ent_ini_disp = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(ent_ini_disp), "07:10");
    ent_fim_disp = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(ent_fim_disp), "08:50");

    g_signal_connect(b_pesq, "clicked", G_CALLBACK(clicou_pesquisar_disp), NULL);
    g_signal_connect(b_volt, "clicked", G_CALLBACK(ir_para_tela), "menu");

    gtk_box_append(GTK_BOX(filtros), gtk_label_new("Dia:"));
    gtk_box_append(GTK_BOX(filtros), drop_dia_disp);
    gtk_box_append(GTK_BOX(filtros), gtk_label_new("Inicio:"));
    gtk_box_append(GTK_BOX(filtros), ent_ini_disp);
    gtk_box_append(GTK_BOX(filtros), gtk_label_new("Fim:"));
    gtk_box_append(GTK_BOX(filtros), ent_fim_disp);
    gtk_box_append(GTK_BOX(filtros), b_pesq);
    gtk_box_append(GTK_BOX(filtros), b_volt);

    msg_disp = gtk_label_new("Selecione dia e horario para buscar salas livres.");
    lista_disp = gtk_list_box_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), lista_disp);

    gtk_box_append(GTK_BOX(box), filtros);
    gtk_box_append(GTK_BOX(box), msg_disp);
    gtk_box_append(GTK_BOX(box), scroll);
    return box;
}

GtkWidget* montar_tela_alocacoes(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *botoes = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *linha1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *linha2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *scroll = gtk_scrolled_window_new();
    GtkWidget *b_nova = gtk_button_new_with_label("Nova");
    GtkWidget *b_edit = gtk_button_new_with_label("Editar");
    GtkWidget *b_canc = gtk_button_new_with_label("Cancelar");
    GtkWidget *b_volt = gtk_button_new_with_label("Voltar");
    GtkWidget *b_salv = gtk_button_new_with_label("Salvar");
    const char *vazio[] = {NULL};
    const char *opcoes_alt[] = {"Permanente", "Temporaria", NULL};

    gtk_widget_set_margin_top(box, 16); gtk_widget_set_margin_bottom(box, 16);
    gtk_widget_set_margin_start(box, 16); gtk_widget_set_margin_end(box, 16);

    g_signal_connect(b_nova, "clicked", G_CALLBACK(clicou_nova_aloc), NULL);
    g_signal_connect(b_edit, "clicked", G_CALLBACK(clicou_editar_aloc), NULL);
    g_signal_connect(b_canc, "clicked", G_CALLBACK(clicou_cancelar_aloc), NULL);
    g_signal_connect(b_volt, "clicked", G_CALLBACK(ir_para_tela), "menu");
    g_signal_connect(b_salv, "clicked", G_CALLBACK(clicou_salvar_aloc), NULL);

    gtk_box_append(GTK_BOX(botoes), b_nova);
    gtk_box_append(GTK_BOX(botoes), b_edit);
    gtk_box_append(GTK_BOX(botoes), b_canc);
    gtk_box_append(GTK_BOX(botoes), b_volt);

    msg_aloc = gtk_label_new("Gerenciamento de Alocacoes");
    conf_aloc = criar_barra_confirma(G_CALLBACK(confirmar_cancelamento_aloc), G_CALLBACK(desistir_cancelamento_aloc));

    lista_aloc = gtk_list_box_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), lista_aloc);

    form_aloc = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    ent_turma = gtk_entry_new(); gtk_entry_set_placeholder_text(GTK_ENTRY(ent_turma), "Turma");
    ent_disc = gtk_entry_new();  gtk_entry_set_placeholder_text(GTK_ENTRY(ent_disc), "Disciplina");
    ent_prof = gtk_entry_new();  gtk_entry_set_placeholder_text(GTK_ENTRY(ent_prof), "Professor");
    drop_amb_aloc = gtk_drop_down_new_from_strings(vazio);
    drop_dia_aloc = gtk_drop_down_new_from_strings(nomes_dias);
    ent_ini_aloc = gtk_entry_new(); gtk_entry_set_placeholder_text(GTK_ENTRY(ent_ini_aloc), "07:10");
    ent_fim_aloc = gtk_entry_new(); gtk_entry_set_placeholder_text(GTK_ENTRY(ent_fim_aloc), "08:50");

    box_alt_aloc = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    drop_tipo_alt = gtk_drop_down_new_from_strings(opcoes_alt);
    gtk_box_append(GTK_BOX(box_alt_aloc), gtk_label_new("Alteracao:"));
    gtk_box_append(GTK_BOX(box_alt_aloc), drop_tipo_alt);

    gtk_box_append(GTK_BOX(linha1), gtk_label_new("Turma:"));
    gtk_box_append(GTK_BOX(linha1), ent_turma);
    gtk_box_append(GTK_BOX(linha1), gtk_label_new("Disciplina:"));
    gtk_box_append(GTK_BOX(linha1), ent_disc);
    gtk_box_append(GTK_BOX(linha1), gtk_label_new("Professor:"));
    gtk_box_append(GTK_BOX(linha1), ent_prof);

    gtk_box_append(GTK_BOX(linha2), gtk_label_new("Sala:"));
    gtk_box_append(GTK_BOX(linha2), drop_amb_aloc);
    gtk_box_append(GTK_BOX(linha2), gtk_label_new("Dia:"));
    gtk_box_append(GTK_BOX(linha2), drop_dia_aloc);
    gtk_box_append(GTK_BOX(linha2), gtk_label_new("Inicio:"));
    gtk_box_append(GTK_BOX(linha2), ent_ini_aloc);
    gtk_box_append(GTK_BOX(linha2), gtk_label_new("Fim:"));
    gtk_box_append(GTK_BOX(linha2), ent_fim_aloc);
    gtk_box_append(GTK_BOX(linha2), box_alt_aloc);
    gtk_box_append(GTK_BOX(linha2), b_salv);

    gtk_box_append(GTK_BOX(form_aloc), linha1);
    gtk_box_append(GTK_BOX(form_aloc), linha2);
    gtk_widget_set_visible(form_aloc, FALSE);

    gtk_box_append(GTK_BOX(box), botoes);
    gtk_box_append(GTK_BOX(box), msg_aloc);
    gtk_box_append(GTK_BOX(box), conf_aloc);
    gtk_box_append(GTK_BOX(box), scroll);
    gtk_box_append(GTK_BOX(box), form_aloc);
    return box;
}

GtkWidget* montar_tela_reservas(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *botoes = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *linha1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *linha2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *scroll = gtk_scrolled_window_new();
    GtkWidget *b_nova = gtk_button_new_with_label("Nova reserva");
    GtkWidget *b_conf = gtk_button_new_with_label("Confirmar");
    GtkWidget *b_canc = gtk_button_new_with_label("Cancelar");
    GtkWidget *b_volt = gtk_button_new_with_label("Voltar");
    GtkWidget *b_salv = gtk_button_new_with_label("Salvar solicitacao");
    const char *vazio[] = {NULL};

    gtk_widget_set_margin_top(box, 16); gtk_widget_set_margin_bottom(box, 16);
    gtk_widget_set_margin_start(box, 16); gtk_widget_set_margin_end(box, 16);

    g_signal_connect(b_nova, "clicked", G_CALLBACK(clicou_nova_res), NULL);
    g_signal_connect(b_conf, "clicked", G_CALLBACK(clicou_confirmar_res), NULL);
    g_signal_connect(b_canc, "clicked", G_CALLBACK(clicou_cancelar_res), NULL);
    g_signal_connect(b_volt, "clicked", G_CALLBACK(ir_para_tela), "menu");
    g_signal_connect(b_salv, "clicked", G_CALLBACK(clicou_salvar_res), NULL);

    gtk_box_append(GTK_BOX(botoes), b_nova);
    gtk_box_append(GTK_BOX(botoes), b_conf);
    gtk_box_append(GTK_BOX(botoes), b_canc);
    gtk_box_append(GTK_BOX(botoes), b_volt);

    msg_res = gtk_label_new("Gerenciamento de Reservas");
    conf_res = criar_barra_confirma(G_CALLBACK(confirmar_cancelamento_res), G_CALLBACK(desistir_cancelamento_res));

    lista_res = gtk_list_box_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), lista_res);

    form_res = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    ent_resp = gtk_entry_new(); gtk_entry_set_placeholder_text(GTK_ENTRY(ent_resp), "Responsavel");
    ent_fin = gtk_entry_new();  gtk_entry_set_placeholder_text(GTK_ENTRY(ent_fin), "Finalidade");
    drop_amb_res = gtk_drop_down_new_from_strings(vazio);
    drop_dia_res = gtk_drop_down_new_from_strings(nomes_dias);
    ent_ini_res = gtk_entry_new(); gtk_entry_set_placeholder_text(GTK_ENTRY(ent_ini_res), "13:50");
    ent_fim_res = gtk_entry_new(); gtk_entry_set_placeholder_text(GTK_ENTRY(ent_fim_res), "15:30");

    gtk_box_append(GTK_BOX(linha1), gtk_label_new("Responsavel:"));
    gtk_box_append(GTK_BOX(linha1), ent_resp);
    gtk_box_append(GTK_BOX(linha1), gtk_label_new("Finalidade:"));
    gtk_box_append(GTK_BOX(linha1), ent_fin);

    gtk_box_append(GTK_BOX(linha2), gtk_label_new("Sala:"));
    gtk_box_append(GTK_BOX(linha2), drop_amb_res);
    gtk_box_append(GTK_BOX(linha2), gtk_label_new("Dia:"));
    gtk_box_append(GTK_BOX(linha2), drop_dia_res);
    gtk_box_append(GTK_BOX(linha2), gtk_label_new("Inicio:"));
    gtk_box_append(GTK_BOX(linha2), ent_ini_res);
    gtk_box_append(GTK_BOX(linha2), gtk_label_new("Fim:"));
    gtk_box_append(GTK_BOX(linha2), ent_fim_res);
    gtk_box_append(GTK_BOX(linha2), b_salv);

    gtk_box_append(GTK_BOX(form_res), linha1);
    gtk_box_append(GTK_BOX(form_res), linha2);
    gtk_widget_set_visible(form_res, FALSE);

    gtk_box_append(GTK_BOX(box), botoes);
    gtk_box_append(GTK_BOX(box), msg_res);
    gtk_box_append(GTK_BOX(box), conf_res);
    gtk_box_append(GTK_BOX(box), scroll);
    gtk_box_append(GTK_BOX(box), form_res);
    return box;
}

GtkWidget* montar_tela_menu(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *b_amb = gtk_button_new_with_label("Ambientes");
    GtkWidget *b_alo = gtk_button_new_with_label("Alocacoes");
    GtkWidget *b_dis = gtk_button_new_with_label("Disponibilidade");
    GtkWidget *b_res = gtk_button_new_with_label("Reservas");

    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);

    gtk_widget_set_size_request(b_amb, 220, 40);
    gtk_widget_set_size_request(b_alo, 220, 40);
    gtk_widget_set_size_request(b_dis, 220, 40);
    gtk_widget_set_size_request(b_res, 220, 40);

    g_signal_connect(b_amb, "clicked", G_CALLBACK(ir_para_tela), "ambientes");
    g_signal_connect(b_alo, "clicked", G_CALLBACK(ir_para_tela), "alocacoes");
    g_signal_connect(b_dis, "clicked", G_CALLBACK(ir_para_tela), "disponibilidade");
    g_signal_connect(b_res, "clicked", G_CALLBACK(ir_para_tela), "reservas");

    gtk_box_append(GTK_BOX(box), gtk_label_new("Sistema de Gestao de Salas — POLI-UPE"));
    gtk_box_append(GTK_BOX(box), b_amb);
    gtk_box_append(GTK_BOX(box), b_alo);
    gtk_box_append(GTK_BOX(box), b_dis);
    gtk_box_append(GTK_BOX(box), b_res);
    return box;
}

void ativar_interface(GtkApplication *app, gpointer user_data) {
    char mensagem[TAM_MENSAGEM];
    GtkWidget *janela;

    inicializar_backend(mensagem);

    janela = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(janela), "Gestao de Salas — POLI-UPE");
    gtk_window_set_default_size(GTK_WINDOW(janela), 900, 580);

    stack_telas = gtk_stack_new();
    gtk_stack_add_named(GTK_STACK(stack_telas), montar_tela_menu(), "menu");
    gtk_stack_add_named(GTK_STACK(stack_telas), montar_tela_ambientes(), "ambientes");
    gtk_stack_add_named(GTK_STACK(stack_telas), montar_tela_alocacoes(), "alocacoes");
    gtk_stack_add_named(GTK_STACK(stack_telas), montar_tela_disponibilidade(), "disponibilidade");
    gtk_stack_add_named(GTK_STACK(stack_telas), montar_tela_reservas(), "reservas");

    gtk_stack_set_visible_child_name(GTK_STACK(stack_telas), "menu");
    gtk_window_set_child(GTK_WINDOW(janela), stack_telas);
    gtk_window_present(GTK_WINDOW(janela));
}