#include <gtk/gtk.h>
#include "interface.h"

/* Função chamada quando qualquer botão de navegação é clicado */
static void mudar_tela(GtkWidget *botao, gpointer user_data) {
    GtkStack *stack = GTK_STACK(user_data);
    const char *destino = (const char *)g_object_get_data(G_OBJECT(botao), "destino");

    if (destino != NULL) {
        gtk_stack_set_visible_child_name(stack, destino);
    }
}

/* Função auxiliar para criar o esqueleto inicial das 4 telas secundárias */
static GtkWidget* criar_tela_base(GtkStack *stack, const char *titulo_texto) {
    GtkWidget *caixa = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_top(caixa, 24);
    gtk_widget_set_margin_bottom(caixa, 24);
    gtk_widget_set_margin_start(caixa, 24);
    gtk_widget_set_margin_end(caixa, 24);

    GtkWidget *titulo = gtk_label_new(titulo_texto);
    GtkWidget *aviso = gtk_label_new("Tela em construção (Fase 1 - Navegação)");
    GtkWidget *btn_voltar = gtk_button_new_with_label("Voltar ao Menu Principal");

    g_object_set_data(G_OBJECT(btn_voltar), "destino", "menu");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(mudar_tela), stack);

    gtk_box_append(GTK_BOX(caixa), titulo);
    gtk_box_append(GTK_BOX(caixa), aviso);
    gtk_box_append(GTK_BOX(caixa), btn_voltar);

    return caixa;
}

/* Função que constrói a tela do Menu Principal */
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

    /* Define para qual página do GtkStack cada botão deve levar */
    g_object_set_data(G_OBJECT(btn_ambientes), "destino", "ambientes");
    g_object_set_data(G_OBJECT(btn_alocacoes), "destino", "alocacoes");
    g_object_set_data(G_OBJECT(btn_disponibilidade), "destino", "disponibilidade");
    g_object_set_data(G_OBJECT(btn_reservas), "destino", "reservas");

    /* Conecta o clique de cada botão à função mudar_tela */
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

/* Função principal que inicializa a janela e registra todas as telas */
void ativar_interface(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    GtkWidget *janela = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(janela), "Gestão de Salas — POLI-UPE");
    gtk_window_set_default_size(GTK_WINDOW(janela), 800, 600);

    GtkWidget *stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);

    /* Cria as 5 páginas da aplicação */
    GtkWidget *tela_menu = criar_tela_menu(GTK_STACK(stack));
    GtkWidget *tela_ambientes = criar_tela_base(GTK_STACK(stack), "Gerenciamento de Ambientes");
    GtkWidget *tela_alocacoes = criar_tela_base(GTK_STACK(stack), "Gerenciamento de Alocações");
    GtkWidget *tela_disponibilidade = criar_tela_base(GTK_STACK(stack), "Consulta de Disponibilidade");
    GtkWidget *tela_reservas = criar_tela_base(GTK_STACK(stack), "Gerenciamento de Reservas");

    /* Adiciona as páginas no GtkStack com seus nomes internos */
    gtk_stack_add_named(GTK_STACK(stack), tela_menu, "menu");
    gtk_stack_add_named(GTK_STACK(stack), tela_ambientes, "ambientes");
    gtk_stack_add_named(GTK_STACK(stack), tela_alocacoes, "alocacoes");
    gtk_stack_add_named(GTK_STACK(stack), tela_disponibilidade, "disponibilidade");
    gtk_stack_add_named(GTK_STACK(stack), tela_reservas, "reservas");

    /* Define que o aplicativo começa exibindo o menu principal */
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "menu");

    gtk_window_set_child(GTK_WINDOW(janela), stack);
    gtk_window_present(GTK_WINDOW(janela));
}