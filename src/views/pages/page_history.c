#include "pages.h"

static void on_export_clicked(GtkWidget *w, gpointer data) { controller_export_log((AppContext*)data); }

GtkWidget* create_page_history(AppContext *ctx) {
GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);


GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
GtkWidget *btn_export = gtk_button_new_with_label(controller_tr(ctx, "Ekspor Log ke Desktop"));
g_signal_connect(btn_export, "clicked", G_CALLBACK(on_export_clicked), ctx);
gtk_box_pack_end(GTK_BOX(top_bar), btn_export, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), top_bar, FALSE, FALSE, 0);

ctx->history_store = gtk_list_store_new(4, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->history_store));
gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(tree), GTK_TREE_VIEW_GRID_LINES_BOTH);

GtkCellRenderer *r_text = gtk_cell_renderer_text_new();
g_object_set(r_text, "font", "bold 9", "foreground", "#fbbf24", "ypad", 8, NULL);
GtkTreeViewColumn *c_name = gtk_tree_view_column_new_with_attributes(controller_tr(ctx, "Nama Paket"), r_text, "text", 1, NULL);
gtk_tree_view_column_set_expand(c_name, TRUE);

GtkCellRenderer *r_act = gtk_cell_renderer_text_new();
g_object_set(r_act, "ypad", 8, NULL);
GtkTreeViewColumn *c_act = gtk_tree_view_column_new_with_attributes(controller_tr(ctx, "Waktu & Aksi Update"), r_act, "text", 2, NULL);

gtk_tree_view_append_column(GTK_TREE_VIEW(tree), c_name);
gtk_tree_view_append_column(GTK_TREE_VIEW(tree), c_act);

GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
gtk_container_add(GTK_CONTAINER(scroll), tree);
gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

return vbox;



}
