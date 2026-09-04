#include "pages.h"

static void on_switch_toggled(GtkCellRendererToggle *cell, gchar *path_str, gpointer user_data) {
AppContext *ctx = (AppContext*)user_data;
GtkTreeIter iter;


if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(ctx->blacklist_store), &iter, path_str)) {
    gboolean active;
    gchar *pkg_name;
    gtk_tree_model_get(GTK_TREE_MODEL(ctx->blacklist_store), &iter, 0, &pkg_name, 1, &active, -1);

    if (pkg_name) {
        bool new_state = !active;
        gtk_list_store_set(ctx->blacklist_store, &iter, 1, new_state, -1);
        controller_toggle_package_hold(ctx, pkg_name, new_state);
        g_free(pkg_name);
    }
}



}

GtkWidget* create_page_blacklist(AppContext *ctx) {
GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);


GtkWidget *lbl_info = gtk_label_new(controller_tr(ctx, "Geser saklar ke posisi ON untuk mengunci/disable update aplikasi (Status Sesi Otomatis Tersimpan):"));
gtk_widget_set_halign(lbl_info, GTK_ALIGN_START);
gtk_box_pack_start(GTK_BOX(vbox), lbl_info, FALSE, FALSE, 0);

ctx->blacklist_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_BOOLEAN);
GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->blacklist_store));
gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(tree), GTK_TREE_VIEW_GRID_LINES_BOTH);

GtkCellRenderer *r_text = gtk_cell_renderer_text_new();
g_object_set(r_text, "font", "bold 9", "foreground", "#f8fafc", "ypad", 8, NULL);
GtkTreeViewColumn *c_name = gtk_tree_view_column_new_with_attributes(controller_tr(ctx, "Nama Paket Terpasang"), r_text, "text", 0, NULL);
gtk_tree_view_column_set_expand(c_name, TRUE);

GtkCellRenderer *r_switch = gtk_cell_renderer_toggle_new();
g_signal_connect(r_switch, "toggled", G_CALLBACK(on_switch_toggled), ctx);
GtkTreeViewColumn *c_switch = gtk_tree_view_column_new_with_attributes(controller_tr(ctx, "Disable Update (Hold)"), r_switch, "active", 1, NULL);

gtk_tree_view_append_column(GTK_TREE_VIEW(tree), c_name);
gtk_tree_view_append_column(GTK_TREE_VIEW(tree), c_switch);

GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
gtk_container_add(GTK_CONTAINER(scroll), tree);
gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

return vbox;



}
