#include "pages.h"

GtkWidget* create_page_installed(AppContext *ctx) {
GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);


ctx->installed_store = gtk_list_store_new(3, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING);
GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->installed_store));
gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(tree), GTK_TREE_VIEW_GRID_LINES_BOTH);

GtkCellRenderer *r_text = gtk_cell_renderer_text_new();
g_object_set(r_text, "font", "bold 9", "foreground", "#38bdf8", "ypad", 8, NULL);
GtkTreeViewColumn *c_name = gtk_tree_view_column_new_with_attributes(controller_tr(ctx, "Nama Paket Terpasang"), r_text, "text", 1, NULL);
gtk_tree_view_column_set_expand(c_name, TRUE);

GtkCellRenderer *r_ver = gtk_cell_renderer_text_new();
g_object_set(r_ver, "foreground", "#94a3b8", "ypad", 8, NULL);
GtkTreeViewColumn *c_ver = gtk_tree_view_column_new_with_attributes(controller_tr(ctx, "Versi Terpasang"), r_ver, "text", 2, NULL);

gtk_tree_view_append_column(GTK_TREE_VIEW(tree), c_name);
gtk_tree_view_append_column(GTK_TREE_VIEW(tree), c_ver);

GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
gtk_container_add(GTK_CONTAINER(scroll), tree);
gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

return vbox;



}
