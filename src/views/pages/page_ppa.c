#include "pages.h"

typedef struct {
AppContext *ctx;
GtkWidget *entry;
} PPAData;

static void on_add_ppa_clicked(GtkWidget *w, gpointer data) {
PPAData *pdata = (PPAData*)data;
const char *txt = gtk_entry_get_text(GTK_ENTRY(pdata->entry));
controller_add_ppa(pdata->ctx, txt);
}

static void on_remove_ppa_clicked(GtkWidget *w, gpointer data) {
AppContext *ctx = (AppContext*)data;
GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_object_get_data(G_OBJECT(w), "tree")));
GtkTreeModel *model;
GtkTreeIter iter;


if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
    gchar *ppa_name;
    gtk_tree_model_get(model, &iter, 0, &ppa_name, -1);
    if (ppa_name) {
        controller_remove_ppa(ctx, ppa_name);
        g_free(ppa_name);
    }
}



}

GtkWidget* create_page_ppa(AppContext *ctx) {
GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);


GtkWidget *lbl_title = gtk_label_new(NULL);
char buf_markup[512];
snprintf(buf_markup, sizeof(buf_markup), "<span font='14' font_weight='bold' foreground='#60a5fa'>📦 %s</span>", controller_tr(ctx, "Manajemen PPA"));
gtk_label_set_markup(GTK_LABEL(lbl_title), buf_markup);
gtk_widget_set_halign(lbl_title, GTK_ALIGN_START);

GtkWidget *hbox_add = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
GtkWidget *entry_ppa = gtk_entry_new();
gtk_entry_set_placeholder_text(GTK_ENTRY(entry_ppa), "Contoh: ppa:graphics-drivers/ppa");

PPAData *pdata = g_new0(PPAData, 1);
pdata->ctx = ctx;
pdata->entry = entry_ppa;

GtkWidget *btn_add = gtk_button_new_with_label(controller_tr(ctx, "Tambah PPA Pihak Ketiga"));
g_signal_connect(btn_add, "clicked", G_CALLBACK(on_add_ppa_clicked), pdata);

gtk_box_pack_start(GTK_BOX(hbox_add), entry_ppa, TRUE, TRUE, 0);
gtk_box_pack_start(GTK_BOX(hbox_add), btn_add, FALSE, FALSE, 0);

ctx->ppa_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->ppa_store));
gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(tree), GTK_TREE_VIEW_GRID_LINES_BOTH);

GtkCellRenderer *r_text = gtk_cell_renderer_text_new();
g_object_set(r_text, "font", "bold 9", "foreground", "#38bdf8", "ypad", 8, NULL);
GtkTreeViewColumn *c_name = gtk_tree_view_column_new_with_attributes(controller_tr(ctx, "Nama Repositori / PPA"), r_text, "text", 0, NULL);
gtk_tree_view_column_set_expand(c_name, TRUE);

GtkCellRenderer *r_src = gtk_cell_renderer_text_new();
g_object_set(r_src, "ypad", 8, "foreground", "#94a3b8", NULL);
GtkTreeViewColumn *c_src = gtk_tree_view_column_new_with_attributes(controller_tr(ctx, "Jenis Sources"), r_src, "text", 1, NULL);

GtkCellRenderer *r_stat = gtk_cell_renderer_text_new();
g_object_set(r_stat, "ypad", 8, NULL);
GtkTreeViewColumn *c_stat = gtk_tree_view_column_new_with_attributes(controller_tr(ctx, "Status"), r_stat, "text", 2, NULL);

gtk_tree_view_append_column(GTK_TREE_VIEW(tree), c_name);
gtk_tree_view_append_column(GTK_TREE_VIEW(tree), c_src);
gtk_tree_view_append_column(GTK_TREE_VIEW(tree), c_stat);

GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
gtk_container_add(GTK_CONTAINER(scroll), tree);

GtkWidget *btn_remove = gtk_button_new_with_label(controller_tr(ctx, "Hapus PPA Terpilih"));
g_object_set_data(G_OBJECT(btn_remove), "tree", tree);
g_signal_connect(btn_remove, "clicked", G_CALLBACK(on_remove_ppa_clicked), ctx);

gtk_box_pack_start(GTK_BOX(vbox), lbl_title, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox_add, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
gtk_box_pack_start(GTK_BOX(vbox), btn_remove, FALSE, FALSE, 0);

return vbox;



}
