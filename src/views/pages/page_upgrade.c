#include "pages.h"

static void on_package_toggled(GtkCellRendererToggle *cell, gchar *path_str, gpointer user_data) {
AppContext *ctx = (AppContext*)user_data;
GtkTreeModel *model = GTK_TREE_MODEL(ctx->upgrade_filter);
GtkTreeIter iter_filter, iter_store;


if (gtk_tree_model_get_iter_from_string(model, &iter_filter, path_str)) {
    gtk_tree_model_filter_convert_iter_to_child_iter(ctx->upgrade_filter, &iter_store, &iter_filter);
    gboolean active;
    gtk_tree_model_get(GTK_TREE_MODEL(ctx->upgrade_store), &iter_store, 0, &active, -1);
    gtk_list_store_set(ctx->upgrade_store, &iter_store, 0, !active, -1);
}



}

static void on_install_clicked(GtkWidget *w, gpointer data) { controller_install_selected((AppContext*)data); }
static void on_refresh_clicked(GtkWidget *w, gpointer data) { controller_load_upgrades((AppContext*)data); }
static void on_select_all_clicked(GtkWidget *w, gpointer data) { controller_toggle_all_upgrades((AppContext*)data, TRUE); }
static void on_deselect_all_clicked(GtkWidget *w, gpointer data) { controller_toggle_all_upgrades((AppContext*)data, FALSE); }

GtkWidget* create_page_upgrade(AppContext *ctx) {
GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);


GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
GtkWidget *btn_refresh = gtk_button_new_with_label(controller_tr(ctx, "Refresh Repositori"));
g_signal_connect(btn_refresh, "clicked", G_CALLBACK(on_refresh_clicked), ctx);

GtkWidget *btn_sel_all = gtk_button_new_with_label(controller_tr(ctx, "Pilih Semua"));
g_signal_connect(btn_sel_all, "clicked", G_CALLBACK(on_select_all_clicked), ctx);

GtkWidget *btn_desel_all = gtk_button_new_with_label(controller_tr(ctx, "Batal Pilih All"));
g_signal_connect(btn_desel_all, "clicked", G_CALLBACK(on_deselect_all_clicked), ctx);

gtk_box_pack_start(GTK_BOX(top_bar), btn_refresh, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(top_bar), btn_sel_all, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(top_bar), btn_desel_all, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), top_bar, FALSE, FALSE, 0);

ctx->upgrade_store = gtk_list_store_new(4, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
ctx->upgrade_filter = GTK_TREE_MODEL_FILTER(gtk_tree_model_filter_new(GTK_TREE_MODEL(ctx->upgrade_store), NULL));

GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->upgrade_filter));
gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(tree), GTK_TREE_VIEW_GRID_LINES_BOTH);

GtkCellRenderer *r_toggle = gtk_cell_renderer_toggle_new();
gtk_cell_renderer_toggle_set_radio(GTK_CELL_RENDERER_TOGGLE(r_toggle), FALSE);
g_signal_connect(r_toggle, "toggled", G_CALLBACK(on_package_toggled), ctx);

GtkTreeViewColumn *c_sel = gtk_tree_view_column_new_with_attributes(controller_tr(ctx, "Pilih"), r_toggle, "active", 0, NULL);

GtkCellRenderer *r_text = gtk_cell_renderer_text_new();
g_object_set(r_text, "font", "bold 10", "foreground", "#38bdf8", "ypad", 8, NULL);
GtkTreeViewColumn *c_name = gtk_tree_view_column_new_with_attributes(controller_tr(ctx, "Nama Paket"), r_text, "text", 1, NULL);
gtk_tree_view_column_set_expand(c_name, TRUE);

GtkCellRenderer *r_vold = gtk_cell_renderer_text_new();
g_object_set(r_vold, "ypad", 8, "foreground", "#94a3b8", NULL);
GtkTreeViewColumn *c_old = gtk_tree_view_column_new_with_attributes(controller_tr(ctx, "Versi Lama"), r_vold, "text", 2, NULL);

GtkCellRenderer *r_vnew = gtk_cell_renderer_text_new();
g_object_set(r_vnew, "foreground", "#34d399", "font", "bold 9", "ypad", 8, NULL);
GtkTreeViewColumn *c_new = gtk_tree_view_column_new_with_attributes(controller_tr(ctx, "Versi Baru"), r_vnew, "text", 3, NULL);

gtk_tree_view_append_column(GTK_TREE_VIEW(tree), c_sel);
gtk_tree_view_append_column(GTK_TREE_VIEW(tree), c_name);
gtk_tree_view_append_column(GTK_TREE_VIEW(tree), c_old);
gtk_tree_view_append_column(GTK_TREE_VIEW(tree), c_new);

GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
gtk_container_add(GTK_CONTAINER(scroll), tree);
gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

ctx->progress_box = gtk_frame_new(controller_tr(ctx, "Progress Transaksi"));
GtkWidget *prog_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
ctx->status_pkg_label = gtk_label_new(controller_tr(ctx, "Siap."));
ctx->progress_bar = gtk_progress_bar_new();
gtk_box_pack_start(GTK_BOX(prog_vbox), ctx->status_pkg_label, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(prog_vbox), ctx->progress_bar, FALSE, FALSE, 0);
gtk_container_add(GTK_CONTAINER(ctx->progress_box), prog_vbox);
gtk_box_pack_start(GTK_BOX(vbox), ctx->progress_box, FALSE, FALSE, 0);

GtkWidget *bot_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
ctx->badge_lbl = gtk_label_new("Total Paket Siap Di-Update: 0");

GtkWidget *btn_install = gtk_button_new_with_label(controller_tr(ctx, "Install Paket (Auto Snapshot)"));
GtkStyleContext *b_style = gtk_widget_get_style_context(btn_install);
gtk_style_context_add_class(b_style, "suggested-action");
g_signal_connect(btn_install, "clicked", G_CALLBACK(on_install_clicked), ctx);

gtk_box_pack_start(GTK_BOX(bot_bar), ctx->badge_lbl, FALSE, FALSE, 0);
gtk_box_pack_end(GTK_BOX(bot_bar), btn_install, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), bot_bar, FALSE, FALSE, 0);

return vbox;



}
