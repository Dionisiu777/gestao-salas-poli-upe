#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include "interface.h"
#include "backend.h"

/* =========================================================================
 * ESTRUTURAS DE WIDGETS DAS TELAS
 * ========================================================================= */

/* Parte 1: Guarda os componentes visuais da Tela de Ambientes */
struct TelaAmbientes {
    GtkWidget *lista_box;
    GtkWidget *box_formulario;
    GtkWidget *lbl_titulo_form;
    GtkWidget *entry_codigo;
    GtkWidget *dropdown_tipo;
    GtkWidget *lbl_mensagem;
    GtkWidget *box_confirmar_exclusao;
    int id_em_edicao; /* 0 = Novo Ambiente; > 0 = ID do ambiente sendo editado */
};

/* Parte 2: Guarda os componentes visuais da Tela de Disponibilidade */
struct TelaDisponibilidade {
    GtkWidget *dropdown_dia;
    GtkWidget *entry_inicio;
    GtkWidget *entry_fim;
    GtkWidget *lbl_mensagem;
    GtkWidget *lista_box;
};

static struct TelaAmbientes tela_amb;
static struct TelaDisponibilidade tela_disp;

/* Protótipos internos */
static void atualizar_lista_ambientes(void);
static void limpar_tela_disponibilidade(void);

/* =========================================================================
 * FUNÇÕES AUXILIARES DE HORÁRIO E FORMATAÇÃO
 * ========================================================================= */

/* Converte texto "HH:MM" (ex: "07:10") para minutos desde 00:00 (ex: 430).
 * Devolve -1 se o formato digitado for inválido. */
static int converter_texto_para_minutos(const char *texto) {
    int h = -1, m = -1;
    char extra;

    if (texto == NULL || strlen(texto) == 0) {
        return -1;
    }
    if (sscanf(texto, "%d:%d%c", &h, &m, &extra) != 2) {
        return -1;
    }
    if (h < 0 || h > 23 || m < 0 || m > 59) {
        return -1;
    }
    return h * 60 + m;
}

/* Converte minutos desde 00:00 (ex: 430) para texto "07:10" */
static void converter_minutos_para_texto(int minutos, char saida[], int tam) {
    int h = minutos / 60;
    int m = minutos % 60;
    snprintf(saida, tam, "%02d:%02d", h, m);
}

/* =========================================================================
 * NAVEGAÇÃO ENTRE AS TELAS (GtkStack)
 * ========================================================================= */
static void mudar_tela(GtkWidget *botao, gpointer user_data) {
    GtkStack *stack = GTK_STACK(user_data);
    const char *destino = (const char *)g_object_get_data(G_OBJECT(botao), "destino");

    if (destino != NULL) {
        if (strcmp(destino, "ambientes") == 0) {
            atualizar_lista_ambientes();
            gtk_label_set_text(GTK_LABEL(tela_amb.lbl_mensagem),
                               "Selecione um ambiente na lista ou clique em 'Novo'.");
            gtk_widget_set_visible(tela_amb.box_formulario, FALSE);
            gtk_widget_set_visible(tela_amb.box_confirmar_exclusao, FALSE);
        } else if (strcmp(destino, "disponibilidade") == 0) {
            limpar_tela_disponibilidade();
        }
        gtk_stack_set_visible_child_name(stack, destino);
    }
}

/* =========================================================================
 * PARTE 1: TELA DE AMBIENTES (Conectada ao backend.c da equipe)
 * ========================================================================= */

static void atualizar_lista_ambientes(void) {
    GtkWidget *filho = gtk_widget_get_first_child(tela_amb.lista_box);
    while (filho != NULL) {
        GtkWidget *proximo = gtk_widget_get_next_sibling(filho);
        gtk_list_box_remove(GTK_LIST_BOX(tela_amb.lista_box), filho);
        filho = proximo;
    }

    struct Ambiente vetor[MAX_AMBIENTES];
    int total = listar_ambientes(vetor, MAX_AMBIENTES);

    for (int i = 0; i < total; i++) {
        const char *nome_tipo = (vetor[i].tipo == LABORATORIO) ? "Laboratório" : "Sala";
        char texto_linha[128];

        snprintf(texto_linha, sizeof(texto_linha),
                 "ID: %d   |   Código: %-10s   |   Tipo: %s",
                 vetor[i].id, vetor[i].codigo, nome_tipo);

        GtkWidget *linha = gtk_list_box_row_new();
        GtkWidget *label = gtk_label_new(texto_linha);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_widget_set_margin_top(label, 6);
        gtk_widget_set_margin_bottom(label, 6);
        gtk_widget_set_margin_start(label, 10);
        gtk_widget_set_margin_end(label, 10);

        g_object_set_data(G_OBJECT(linha), "id_ambiente", GINT_TO_POINTER(vetor[i].id));
        g_object_set_data(G_OBJECT(linha), "tipo_ambiente", GINT_TO_POINTER(vetor[i].tipo));
        g_object_set_data_full(G_OBJECT(linha), "codigo_ambiente",
                               g_strdup(vetor[i].codigo), g_free);

        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(linha), label);
        gtk_list_box_append(GTK_LIST_BOX(tela_amb.lista_box), linha);
    }
}

static void ao_clicar_novo_ambiente(GtkWidget *botao, gpointer user_data) {
    (void)botao;
    (void)user_data;

    tela_amb.id_em_edicao = 0;
    gtk_label_set_text(GTK_LABEL(tela_amb.lbl_titulo_form), "Formulário: Novo Ambiente");
    gtk_editable_set_text(GTK_EDITABLE(tela_amb.entry_codigo), "");
    gtk_drop_down_set_selected(GTK_DROP_DOWN(tela_amb.dropdown_tipo), SALA);

    gtk_widget_set_visible(tela_amb.box_confirmar_exclusao, FALSE);
    gtk_widget_set_visible(tela_amb.box_formulario, TRUE);
    gtk_label_set_text(GTK_LABEL(tela_amb.lbl_mensagem),
                       "Digite o código, escolha o tipo e clique em 'Salvar'.");
}

static void ao_clicar_editar_ambiente(GtkWidget *botao, gpointer user_data) {
    (void)botao;
    (void)user_data;

    GtkListBoxRow *linha = gtk_list_box_get_selected_row(GTK_LIST_BOX(tela_amb.lista_box));
    if (linha == NULL) {
        gtk_label_set_text(GTK_LABEL(tela_amb.lbl_mensagem),
                           "Aviso: Selecione um ambiente na lista antes de clicar em 'Editar'.");
        return;
    }

    int id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(linha), "id_ambiente"));
    int tipo = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(linha), "tipo_ambiente"));
    const char *codigo = (const char *)g_object_get_data(G_OBJECT(linha), "codigo_ambiente");

    tela_amb.id_em_edicao = id;
    gtk_label_set_text(GTK_LABEL(tela_amb.lbl_titulo_form), "Formulário: Editar Ambiente");
    gtk_editable_set_text(GTK_EDITABLE(tela_amb.entry_codigo), codigo != NULL ? codigo : "");
    gtk_drop_down_set_selected(GTK_DROP_DOWN(tela_amb.dropdown_tipo),
                               (tipo == LABORATORIO) ? LABORATORIO : SALA);

    gtk_widget_set_visible(tela_amb.box_confirmar_exclusao, FALSE);
    gtk_widget_set_visible(tela_amb.box_formulario, TRUE);
    gtk_label_set_text(GTK_LABEL(tela_amb.lbl_mensagem),
                       "Altere os dados desejados e clique em 'Salvar'.");
}

static void ao_clicar_salvar_ambiente(GtkWidget *botao, gpointer user_data) {
    (void)botao;
    (void)user_data;

    const char *texto_digitado = gtk_editable_get_text(GTK_EDITABLE(tela_amb.entry_codigo));
    guint indice_tipo = gtk_drop_down_get_selected(GTK_DROP_DOWN(tela_amb.dropdown_tipo));
    int tipo = (indice_tipo == 1) ? LABORATORIO : SALA;

    if (texto_digitado == NULL || strlen(texto_digitado) == 0) {
        gtk_label_set_text(GTK_LABEL(tela_amb.lbl_mensagem),
                           "Aviso: Informe o código do ambiente antes de salvar.");
        return;
    }

    char codigo_buf[TAM_CODIGO];
    strncpy(codigo_buf, texto_digitado, TAM_CODIGO - 1);
    codigo_buf[TAM_CODIGO - 1] = '\0';

    char mensagem[TAM_MENSAGEM];
    int retorno;

    if (tela_amb.id_em_edicao == 0) {
        retorno = cadastrar_ambiente(codigo_buf, tipo, mensagem);
    } else {
        retorno = editar_ambiente(tela_amb.id_em_edicao, codigo_buf, tipo, mensagem);
    }

    gtk_label_set_text(GTK_LABEL(tela_amb.lbl_mensagem), mensagem);

    if (retorno == 0) {
        atualizar_lista_ambientes();
        gtk_widget_set_visible(tela_amb.box_formulario, FALSE);
    }
}

static void ao_clicar_excluir_ambiente(GtkWidget *botao, gpointer user_data) {
    (void)botao;
    (void)user_data;

    GtkListBoxRow *linha = gtk_list_box_get_selected_row(GTK_LIST_BOX(tela_amb.lista_box));
    if (linha == NULL) {
        gtk_label_set_text(GTK_LABEL(tela_amb.lbl_mensagem),
                           "Aviso: Selecione um ambiente na lista para excluir.");
        return;
    }

    const char *codigo = (const char *)g_object_get_data(G_OBJECT(linha), "codigo_ambiente");
    char pergunta[128];
    snprintf(pergunta, sizeof(pergunta),
             "Confirma a exclusão do ambiente '%s'?", codigo != NULL ? codigo : "");

    gtk_label_set_text(GTK_LABEL(tela_amb.lbl_mensagem), pergunta);
    gtk_widget_set_visible(tela_amb.box_formulario, FALSE);
    gtk_widget_set_visible(tela_amb.box_confirmar_exclusao, TRUE);
}

static void ao_confirmar_exclusao_ambiente(GtkWidget *botao, gpointer user_data) {
    (void)botao;
    (void)user_data;

    GtkListBoxRow *linha = gtk_list_box_get_selected_row(GTK_LIST_BOX(tela_amb.lista_box));
    if (linha == NULL) {
        gtk_widget_set_visible(tela_amb.box_confirmar_exclusao, FALSE);
        return;
    }

    int id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(linha), "id_ambiente"));
    char mensagem[TAM_MENSAGEM];

    int retorno = excluir_ambiente(id, mensagem);
    gtk_label_set_text(GTK_LABEL(tela_amb.lbl_mensagem), mensagem);
    gtk_widget_set_visible(tela_amb.box_confirmar_exclusao, FALSE);

    if (retorno == 0) {
        atualizar_lista_ambientes();
    }
}

static void ao_cancelar_exclusao_ambiente(GtkWidget *botao, gpointer user_data) {
    (void)botao;
    (void)user_data;

    gtk_widget_set_visible(tela_amb.box_confirmar_exclusao, FALSE);
    gtk_label_set_text(GTK_LABEL(tela_amb.lbl_mensagem), "Exclusão cancelada.");
}

static GtkWidget* criar_tela_ambientes(GtkStack *stack) {
    GtkWidget *caixa_principal = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(caixa_principal, 20);
    gtk_widget_set_margin_bottom(caixa_principal, 20);
    gtk_widget_set_margin_start(caixa_principal, 20);
    gtk_widget_set_margin_end(caixa_principal, 20);

    GtkWidget *titulo = gtk_label_new("Gerenciamento de Ambientes");
    gtk_widget_set_halign(titulo, GTK_ALIGN_START);

    GtkWidget *barra_botoes = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *btn_novo = gtk_button_new_with_label("Novo");
    GtkWidget *btn_editar = gtk_button_new_with_label("Editar");
    GtkWidget *btn_excluir = gtk_button_new_with_label("Excluir");
    GtkWidget *btn_voltar = gtk_button_new_with_label("Voltar");

    g_object_set_data(G_OBJECT(btn_voltar), "destino", "menu");

    g_signal_connect(btn_novo, "clicked", G_CALLBACK(ao_clicar_novo_ambiente), NULL);
    g_signal_connect(btn_editar, "clicked", G_CALLBACK(ao_clicar_editar_ambiente), NULL);
    g_signal_connect(btn_excluir, "clicked", G_CALLBACK(ao_clicar_excluir_ambiente), NULL);
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(mudar_tela), stack);

    gtk_box_append(GTK_BOX(barra_botoes), btn_novo);
    gtk_box_append(GTK_BOX(barra_botoes), btn_editar);
    gtk_box_append(GTK_BOX(barra_botoes), btn_excluir);
    gtk_box_append(GTK_BOX(barra_botoes), btn_voltar);

    tela_amb.lbl_mensagem = gtk_label_new("Selecione um ambiente na lista ou clique em 'Novo'.");
    gtk_widget_set_halign(tela_amb.lbl_mensagem, GTK_ALIGN_START);

    tela_amb.box_confirmar_exclusao = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *lbl_conf = gtk_label_new("Confirmar exclusão deste ambiente?");
    GtkWidget *btn_sim = gtk_button_new_with_label("Sim, Excluir");
    GtkWidget *btn_nao = gtk_button_new_with_label("Cancelar");

    g_signal_connect(btn_sim, "clicked", G_CALLBACK(ao_confirmar_exclusao_ambiente), NULL);
    g_signal_connect(btn_nao, "clicked", G_CALLBACK(ao_cancelar_exclusao_ambiente), NULL);

    gtk_box_append(GTK_BOX(tela_amb.box_confirmar_exclusao), lbl_conf);
    gtk_box_append(GTK_BOX(tela_amb.box_confirmar_exclusao), btn_sim);
    gtk_box_append(GTK_BOX(tela_amb.box_confirmar_exclusao), btn_nao);
    gtk_widget_set_visible(tela_amb.box_confirmar_exclusao, FALSE);

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    tela_amb.lista_box = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(tela_amb.lista_box), GTK_SELECTION_SINGLE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), tela_amb.lista_box);

    tela_amb.box_formulario = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    tela_amb.lbl_titulo_form = gtk_label_new("Formulário de Ambiente");
    gtk_widget_set_halign(tela_amb.lbl_titulo_form, GTK_ALIGN_START);

    GtkWidget *linha_campos = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *lbl_cod = gtk_label_new("Código:");
    tela_amb.entry_codigo = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(tela_amb.entry_codigo), TAM_CODIGO - 1);
    gtk_entry_set_placeholder_text(GTK_ENTRY(tela_amb.entry_codigo), "Ex: A03, LIP07");

    GtkWidget *lbl_tipo = gtk_label_new("Tipo:");
    const char *opcoes_tipo[] = {"Sala", "Laboratório", NULL};
    tela_amb.dropdown_tipo = gtk_drop_down_new_from_strings(opcoes_tipo);

    GtkWidget *btn_salvar = gtk_button_new_with_label("Salvar");
    g_signal_connect(btn_salvar, "clicked", G_CALLBACK(ao_clicar_salvar_ambiente), NULL);

    gtk_box_append(GTK_BOX(linha_campos), lbl_cod);
    gtk_box_append(GTK_BOX(linha_campos), tela_amb.entry_codigo);
    gtk_box_append(GTK_BOX(linha_campos), lbl_tipo);
    gtk_box_append(GTK_BOX(linha_campos), tela_amb.dropdown_tipo);
    gtk_box_append(GTK_BOX(linha_campos), btn_salvar);

    gtk_box_append(GTK_BOX(tela_amb.box_formulario), tela_amb.lbl_titulo_form);
    gtk_box_append(GTK_BOX(tela_amb.box_formulario), linha_campos);
    gtk_widget_set_visible(tela_amb.box_formulario, FALSE);

    gtk_box_append(GTK_BOX(caixa_principal), titulo);
    gtk_box_append(GTK_BOX(caixa_principal), barra_botoes);
    gtk_box_append(GTK_BOX(caixa_principal), tela_amb.lbl_mensagem);
    gtk_box_append(GTK_BOX(caixa_principal), tela_amb.box_confirmar_exclusao);
    gtk_box_append(GTK_BOX(caixa_principal), scroll);
    gtk_box_append(GTK_BOX(caixa_principal), tela_amb.box_formulario);

    return caixa_principal;
}

/* =========================================================================
 * PARTE 2: TELA DE DISPONIBILIDADE (Conectada a buscar_ambientes_livres)
 * ========================================================================= */

static void limpar_lista_disponibilidade(void) {
    GtkWidget *filho = gtk_widget_get_first_child(tela_disp.lista_box);
    while (filho != NULL) {
        GtkWidget *proximo = gtk_widget_get_next_sibling(filho);
        gtk_list_box_remove(GTK_LIST_BOX(tela_disp.lista_box), filho);
        filho = proximo;
    }
}

static void limpar_tela_disponibilidade(void) {
    limpar_lista_disponibilidade();
    gtk_label_set_text(GTK_LABEL(tela_disp.lbl_mensagem),
                       "Escolha o dia e digite o intervalo (ex: 07:10 às 08:50) para pesquisar.");
}

/* Botão "Pesquisar": valida os filtros e chama buscar_ambientes_livres */
static void ao_clicar_pesquisar_disponibilidade(GtkWidget *botao, gpointer user_data) {
    (void)botao;
    (void)user_data;

    limpar_lista_disponibilidade();

    int dia = (int)gtk_drop_down_get_selected(GTK_DROP_DOWN(tela_disp.dropdown_dia));
    const char *txt_ini = gtk_editable_get_text(GTK_EDITABLE(tela_disp.entry_inicio));
    const char *txt_fim = gtk_editable_get_text(GTK_EDITABLE(tela_disp.entry_fim));

    if (txt_ini == NULL || strlen(txt_ini) == 0 || txt_fim == NULL || strlen(txt_fim) == 0) {
        gtk_label_set_text(GTK_LABEL(tela_disp.lbl_mensagem),
                           "Aviso: Preencha o horário inicial e o horário final (formato HH:MM).");
        return;
    }

    int min_inicio = converter_texto_para_minutos(txt_ini);
    int min_fim = converter_texto_para_minutos(txt_fim);

    if (min_inicio == -1 || min_fim == -1) {
        gtk_label_set_text(GTK_LABEL(tela_disp.lbl_mensagem),
                           "Aviso: Formato de horário inválido! Use HH:MM (ex: 07:10 e 08:50).");
        return;
    }

    struct Ambiente livres[MAX_AMBIENTES];
    char mensagem[TAM_MENSAGEM];

    int qtd = buscar_ambientes_livres(dia, min_inicio, min_fim, livres, MAX_AMBIENTES, mensagem);

    if (qtd < 0) {
        /* Erro de validação do backend (ex: fora da faixa 07:10 - 22:10 ou inicio >= fim) */
        gtk_label_set_text(GTK_LABEL(tela_disp.lbl_mensagem), mensagem);
        return;
    }

    if (qtd == 0) {
        gtk_label_set_text(GTK_LABEL(tela_disp.lbl_mensagem),
                           "Nenhum ambiente disponível nesse horário");
        return;
    }

    char resumo[256];
    char ini_fmt[10], fim_fmt[10];
    converter_minutos_para_texto(min_inicio, ini_fmt, sizeof(ini_fmt));
    converter_minutos_para_texto(min_fim, fim_fmt, sizeof(fim_fmt));

    snprintf(resumo, sizeof(resumo),
             "%s — %d ambiente(s) livre(s) das %s às %s:",
             mensagem, qtd, ini_fmt, fim_fmt);
    gtk_label_set_text(GTK_LABEL(tela_disp.lbl_mensagem), resumo);

    for (int i = 0; i < qtd; i++) {
        const char *nome_tipo = (livres[i].tipo == LABORATORIO) ? "Laboratório" : "Sala";
        char texto_linha[128];

        snprintf(texto_linha, sizeof(texto_linha),
                 "ID: %d   |   Código: %-10s   |   Tipo: %s   [LIVRE]",
                 livres[i].id, livres[i].codigo, nome_tipo);

        GtkWidget *linha = gtk_list_box_row_new();
        GtkWidget *label = gtk_label_new(texto_linha);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_widget_set_margin_top(label, 6);
        gtk_widget_set_margin_bottom(label, 6);
        gtk_widget_set_margin_start(label, 10);
        gtk_widget_set_margin_end(label, 10);

        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(linha), label);
        gtk_list_box_append(GTK_LIST_BOX(tela_disp.lista_box), linha);
    }
}

/* Monta visualmente a Tela de Disponibilidade */
static GtkWidget* criar_tela_disponibilidade(GtkStack *stack) {
    GtkWidget *caixa_principal = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(caixa_principal, 20);
    gtk_widget_set_margin_bottom(caixa_principal, 20);
    gtk_widget_set_margin_start(caixa_principal, 20);
    gtk_widget_set_margin_end(caixa_principal, 20);

    GtkWidget *titulo = gtk_label_new("Consulta de Disponibilidade de Ambientes");
    gtk_widget_set_halign(titulo, GTK_ALIGN_START);

    /* Linha de filtros: Dia, Início, Fim, Pesquisar, Voltar */
    GtkWidget *barra_filtros = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    GtkWidget *lbl_dia = gtk_label_new("Dia:");
    const char *opcoes_dia[] = {
        "Segunda-feira", "Terça-feira", "Quarta-feira",
        "Quinta-feira", "Sexta-feira", "Sábado", NULL
    };
    tela_disp.dropdown_dia = gtk_drop_down_new_from_strings(opcoes_dia);

    GtkWidget *lbl_ini = gtk_label_new("Início (HH:MM):");
    tela_disp.entry_inicio = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(tela_disp.entry_inicio), "07:10");
    gtk_widget_set_size_request(tela_disp.entry_inicio, 90, -1);

    GtkWidget *lbl_fim = gtk_label_new("Fim (HH:MM):");
    tela_disp.entry_fim = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(tela_disp.entry_fim), "08:50");
    gtk_widget_set_size_request(tela_disp.entry_fim, 90, -1);

    GtkWidget *btn_pesquisar = gtk_button_new_with_label("Pesquisar");
    GtkWidget *btn_voltar = gtk_button_new_with_label("Voltar");
    g_object_set_data(G_OBJECT(btn_voltar), "destino", "menu");

    g_signal_connect(btn_pesquisar, "clicked", G_CALLBACK(ao_clicar_pesquisar_disponibilidade), NULL);
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(mudar_tela), stack);

    gtk_box_append(GTK_BOX(barra_filtros), lbl_dia);
    gtk_box_append(GTK_BOX(barra_filtros), tela_disp.dropdown_dia);
    gtk_box_append(GTK_BOX(barra_filtros), lbl_ini);
    gtk_box_append(GTK_BOX(barra_filtros), tela_disp.entry_inicio);
    gtk_box_append(GTK_BOX(barra_filtros), lbl_fim);
    gtk_box_append(GTK_BOX(barra_filtros), tela_disp.entry_fim);
    gtk_box_append(GTK_BOX(barra_filtros), btn_pesquisar);
    gtk_box_append(GTK_BOX(barra_filtros), btn_voltar);

    tela_disp.lbl_mensagem = gtk_label_new(
        "Escolha o dia e digite o intervalo (ex: 07:10 às 08:50) para pesquisar.");
    gtk_widget_set_halign(tela_disp.lbl_mensagem, GTK_ALIGN_START);

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    tela_disp.lista_box = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(tela_disp.lista_box), GTK_SELECTION_NONE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), tela_disp.lista_box);

    gtk_box_append(GTK_BOX(caixa_principal), titulo);
    gtk_box_append(GTK_BOX(caixa_principal), barra_filtros);
    gtk_box_append(GTK_BOX(caixa_principal), tela_disp.lbl_mensagem);
    gtk_box_append(GTK_BOX(caixa_principal), scroll);

    return caixa_principal;
}

/* =========================================================================
 * TELAS PROVISÓRIAS (Partes 3 e 4) E MENU PRINCIPAL
 * ========================================================================= */
static GtkWidget* criar_tela_base(GtkStack *stack, const char *titulo_texto) {
    GtkWidget *caixa = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_top(caixa, 24);
    gtk_widget_set_margin_bottom(caixa, 24);
    gtk_widget_set_margin_start(caixa, 24);
    gtk_widget_set_margin_end(caixa, 24);

    GtkWidget *titulo = gtk_label_new(titulo_texto);
    GtkWidget *aviso = gtk_label_new("Tela será implementada nas próximas partes.");
    GtkWidget *btn_voltar = gtk_button_new_with_label("Voltar ao Menu Principal");

    g_object_set_data(G_OBJECT(btn_voltar), "destino", "menu");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(mudar_tela), stack);

    gtk_box_append(GTK_BOX(caixa), titulo);
    gtk_box_append(GTK_BOX(caixa), aviso);
    gtk_box_append(GTK_BOX(caixa), btn_voltar);

    return caixa;
}

static GtkWidget* criar_tela_menu(GtkStack *stack) {
    GtkWidget *caixa = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_valign(caixa, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(caixa, GTK_ALIGN_CENTER);

    GtkWidget *titulo = gtk_label_new("Sistema de Gestão de Salas — POLI-UPE");
    GtkWidget *subtitulo = gtk_label_new("Selecione uma área para gerenciar:");

    GtkWidget *btn_ambientes = gtk_button_new_with_label("Ambientes");
    GtkWidget *btn_alocacoes = gtk_button_new_with_label("Alocações");
    GtkWidget *btn_disponibilidade = gtk_button_new_with_label("Disponibilidade");
    GtkWidget *btn_reservas = gtk_button_new_with_label("Reservas");

    gtk_widget_set_size_request(btn_ambientes, 240, 40);
    gtk_widget_set_size_request(btn_alocacoes, 240, 40);
    gtk_widget_set_size_request(btn_disponibilidade, 240, 40);
    gtk_widget_set_size_request(btn_reservas, 240, 40);

    g_object_set_data(G_OBJECT(btn_ambientes), "destino", "ambientes");
    g_object_set_data(G_OBJECT(btn_alocacoes), "destino", "alocacoes");
    g_object_set_data(G_OBJECT(btn_disponibilidade), "destino", "disponibilidade");
    g_object_set_data(G_OBJECT(btn_reservas), "destino", "reservas");

    g_signal_connect(btn_ambientes, "clicked", G_CALLBACK(mudar_tela), stack);
    g_signal_connect(btn_alocacoes, "clicked", G_CALLBACK(mudar_tela), stack);
    g_signal_connect(btn_disponibilidade, "clicked", G_CALLBACK(mudar_tela), stack);
    g_signal_connect(btn_reservas, "clicked", G_CALLBACK(mudar_tela), stack);

    gtk_box_append(GTK_BOX(caixa), titulo);
    gtk_box_append(GTK_BOX(caixa), subtitulo);
    gtk_box_append(GTK_BOX(caixa), btn_ambientes);
    gtk_box_append(GTK_BOX(caixa), btn_alocacoes);
    gtk_box_append(GTK_BOX(caixa), btn_disponibilidade);
    gtk_box_append(GTK_BOX(caixa), btn_reservas);

    return caixa;
}

void ativar_interface(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    char msg_inicial[TAM_MENSAGEM];
    inicializar_backend(msg_inicial);

    GtkWidget *janela = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(janela), "Gestão de Salas — POLI-UPE");
    gtk_window_set_default_size(GTK_WINDOW(janela), 850, 600);

    GtkWidget *stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);

    GtkWidget *tela_menu = criar_tela_menu(GTK_STACK(stack));
    GtkWidget *tela_ambientes = criar_tela_ambientes(GTK_STACK(stack));
    GtkWidget *tela_alocacoes = criar_tela_base(GTK_STACK(stack), "Gerenciamento de Alocações");
    GtkWidget *tela_disponibilidade = criar_tela_disponibilidade(GTK_STACK(stack));
    GtkWidget *tela_reservas = criar_tela_base(GTK_STACK(stack), "Gerenciamento de Reservas");

    gtk_stack_add_named(GTK_STACK(stack), tela_menu, "menu");
    gtk_stack_add_named(GTK_STACK(stack), tela_ambientes, "ambientes");
    gtk_stack_add_named(GTK_STACK(stack), tela_alocacoes, "alocacoes");
    gtk_stack_add_named(GTK_STACK(stack), tela_disponibilidade, "disponibilidade");
    gtk_stack_add_named(GTK_STACK(stack), tela_reservas, "reservas");

    atualizar_lista_ambientes();
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "menu");

    gtk_window_set_child(GTK_WINDOW(janela), stack);
    gtk_window_present(GTK_WINDOW(janela));
}