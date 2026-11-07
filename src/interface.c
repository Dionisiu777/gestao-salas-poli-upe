
#include "interface.h"

void iniciar(GtkApplication *app, gpointer dados)
{
    GtkWidget *janela;

    janela = gtk_application_window_new(app);

    gtk_window_set_title(
        GTK_WINDOW(janela),
        "Gestao de Salas - POLI-UPE"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(janela), 800, 500
    );

    gtk_window_present(GTK_WINDOW(janela));
}
