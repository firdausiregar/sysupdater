#include "pages.h"

static void on_clean_click(GtkWidget *w, gpointer data) {
controller_run_cleaner((AppContext*)data);
}

GtkWidget* create_page_cleaner(AppContext *ctx) {
GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);


GtkWidget *lbl_title = gtk_label_new(NULL);
char buf_markup[512];
snprintf(buf_markup, sizeof(buf_markup), "<span font='14' font_weight='bold' foreground='#34d399'>🧹 %s</span>", controller_tr(ctx, "Cleaner System"));
gtk_label_set_markup(GTK_LABEL(lbl_title), buf_markup);
gtk_widget_set_halign(lbl_title, GTK_ALIGN_START);

GtkWidget *lbl_desc = gtk_label_new(controller_tr(ctx, "Bersihkan berkas cache paket tua, paket orphan tak terpakai, dan log sampah sistem untuk menghemat ruang disk Anda secara instan."));
gtk_label_set_line_wrap(GTK_LABEL(lbl_desc), TRUE);
gtk_widget_set_halign(lbl_desc, GTK_ALIGN_START);

GtkWidget *btn_clean = gtk_button_new_with_label(controller_tr(ctx, "Mulai Bersihkan System Junk"));
GtkStyleContext *b_style = gtk_widget_get_style_context(btn_clean);
gtk_style_context_add_class(b_style, "suggested-action");
g_signal_connect(btn_clean, "clicked", G_CALLBACK(on_clean_click), ctx);

gtk_box_pack_start(GTK_BOX(vbox), lbl_title, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), lbl_desc, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), btn_clean, FALSE, FALSE, 10);

return vbox;



}
