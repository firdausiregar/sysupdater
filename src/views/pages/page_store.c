#include "pages.h"

static void on_store_toggled(GtkCellRendererToggle *cell, gchar *path_str, gpointer user_data) {
    AppContext *ctx = (AppContext*)user_data;
    GtkTreeModel *model = GTK_TREE_MODEL(ctx->store_tree_store);
    GtkTreePath *path = gtk_tree_path_new_from_string(path_str);
    GtkTreeIter iter;

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gboolean current_val;
        gboolean is_category;
        gtk_tree_model_get(model, &iter, 0, &current_val, 3, &is_category, -1);

        /* Hanya ubah status centang jika ini item aplikasi, bukan folder kategori */
        if (!is_category) {
            gtk_tree_store_set(ctx->store_tree_store, &iter, 0, !current_val, -1);
        }
    }
    gtk_tree_path_free(path);
}

static void on_install_store_clicked(GtkButton *btn, gpointer user_data) {
    AppContext *ctx = (AppContext*)user_data;
    controller_install_store_selected(ctx);
}

static void on_select_all_store_toggled(GtkToggleButton *btn, gpointer user_data) {
    AppContext *ctx = (AppContext*)user_data;
    gboolean active = gtk_toggle_button_get_active(btn);
    controller_toggle_all_store(ctx, active);
}

GtkWidget* create_page_store(AppContext *ctx) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    /* Header Bar Katalog */
    GtkWidget *hbox_top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *lbl_title = gtk_label_new("🛒 Katalog Aplikasi Linux Universal");
    PangoAttrList *attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    gtk_label_set_attributes(GTK_LABEL(lbl_title), attrs);
    pango_attr_list_unref(attrs);

    GtkWidget *chk_all = gtk_check_button_new_with_label("Pilih Semua Aplikasi");
    g_signal_connect(chk_all, "toggled", G_CALLBACK(on_select_all_store_toggled), ctx);

    GtkWidget *btn_install = gtk_button_new_with_label("📥 Install Aplikasi Terpilih");
    GtkStyleContext *b_ctx = gtk_widget_get_style_context(btn_install);
    gtk_style_context_add_class(b_ctx, "suggested-action");
    g_signal_connect(btn_install, "clicked", G_CALLBACK(on_install_store_clicked), ctx);

    gtk_box_pack_start(GTK_BOX(hbox_top), lbl_title, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox_top), btn_install, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox_top), chk_all, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_top, FALSE, FALSE, 0);

    /* TreeStore (0: Selected, 1: Name/Category, 2: Description, 3: IsCategory) */
    ctx->store_tree_store = gtk_tree_store_new(4, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);

    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->store_tree_store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), TRUE);

    /* Kolom 1: Checkbox */
    GtkCellRenderer *r_toggle = gtk_cell_renderer_toggle_new();
    g_signal_connect(r_toggle, "toggled", G_CALLBACK(on_store_toggled), ctx);
    GtkTreeViewColumn *col_chk = gtk_tree_view_column_new_with_attributes("Pilih", r_toggle, "active", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col_chk);

    /* Kolom 2: Nama Aplikasi / Kategori */
    GtkCellRenderer *r_text_name = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *col_name = gtk_tree_view_column_new_with_attributes("Aplikasi / Kategori", r_text_name, "text", 1, NULL);
    gtk_tree_view_column_set_expand(col_name, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col_name);

    /* Kolom 3: Deskripsi Paket */
    GtkCellRenderer *r_text_desc = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *col_desc = gtk_tree_view_column_new_with_attributes("Deskripsi", r_text_desc, "text", 2, NULL);
    gtk_tree_view_column_set_expand(col_desc, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col_desc);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), tree);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    return vbox;
}
