#include "pages.h"
#include "../components/header.h"

static void on_quick_check_clicked(GtkWidget *w, gpointer data) {
AppContext *ctx = (AppContext*)data;
controller_switch_page(ctx, "page_upgrade");
}

static void on_quick_store_clicked(GtkWidget *w, gpointer data) {
AppContext *ctx = (AppContext*)data;
controller_switch_page(ctx, "page_store");
}

static void on_open_terminal_cli(GtkWidget *w, gpointer data) {
system("x-terminal-emulator -e sysupdate --menu 2>/dev/null || gnome-terminal -- sysupdate --menu 2>/dev/null || konsole -e sysupdate --menu 2>/dev/null &");
}

GtkWidget* create_page_dashboard(AppContext *ctx) {
GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);


gtk_box_pack_start(GTK_BOX(vbox), create_header_component(ctx), FALSE, FALSE, 0);

GtkWidget *grid = gtk_grid_new();
gtk_grid_set_row_spacing(GTK_GRID(grid), 12);
gtk_grid_set_column_spacing(GTK_GRID(grid), 12);

GtkWidget *card1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
GtkStyleContext *c1_ctx = gtk_widget_get_style_context(card1);
gtk_style_context_add_class(c1_ctx, "card-frame");

GtkWidget *lbl_c1_title = gtk_label_new(NULL);
char buf_markup[512];
snprintf(buf_markup, sizeof(buf_markup), "<span font='12' font_weight='bold' foreground='#38bdf8'>%s</span>", controller_tr(ctx, "Monitoring Performa System"));
gtk_label_set_markup(GTK_LABEL(lbl_c1_title), buf_markup);
gtk_widget_set_halign(lbl_c1_title, GTK_ALIGN_START);

char ram_txt[256];
snprintf(ram_txt, sizeof(ram_txt), "<b>%s</b> %s", controller_tr(ctx, "Penggunaan RAM:"), ctx->model->ram_info);
GtkWidget *lbl_ram = gtk_label_new(NULL);
gtk_label_set_markup(GTK_LABEL(lbl_ram), ram_txt);
gtk_widget_set_halign(lbl_ram, GTK_ALIGN_START);

GtkWidget *pbar_ram = gtk_progress_bar_new();
gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar_ram), ctx->model->ram_percentage);

char cpu_txt[256];
snprintf(cpu_txt, sizeof(cpu_txt), "<b>%s</b> %s\n<b>%s</b> %s", controller_tr(ctx, "CPU:"), ctx->model->cpu_model, controller_tr(ctx, "Disk Root:"), ctx->model->disk_info);
GtkWidget *lbl_cpu = gtk_label_new(NULL);
gtk_label_set_markup(GTK_LABEL(lbl_cpu), cpu_txt);
gtk_widget_set_halign(lbl_cpu, GTK_ALIGN_START);

gtk_box_pack_start(GTK_BOX(card1), lbl_c1_title, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(card1), lbl_ram, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(card1), pbar_ram, FALSE, FALSE, 4);
gtk_box_pack_start(GTK_BOX(card1), lbl_cpu, FALSE, FALSE, 0);

GtkWidget *card2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
GtkStyleContext *c2_ctx = gtk_widget_get_style_context(card2);
gtk_style_context_add_class(c2_ctx, "card-frame");

GtkWidget *lbl_c2_title = gtk_label_new(NULL);
snprintf(buf_markup, sizeof(buf_markup), "<span font='12' font_weight='bold' foreground='#f59e0b'>%s</span>", controller_tr(ctx, "Terminal CLI Command Line"));
gtk_label_set_markup(GTK_LABEL(lbl_c2_title), buf_markup);
gtk_widget_set_halign(lbl_c2_title, GTK_ALIGN_START);

GtkWidget *lbl_cli_desc = gtk_label_new(controller_tr(ctx, "Buka menu teks berwarna interaktif langsung dari terminal:"));
gtk_widget_set_halign(lbl_cli_desc, GTK_ALIGN_START);

GtkWidget *btn_launch_cli = gtk_button_new_with_label(controller_tr(ctx, "Jalankan Menu CLI Interaktif"));
g_signal_connect(btn_launch_cli, "clicked", G_CALLBACK(on_open_terminal_cli), ctx);

gtk_box_pack_start(GTK_BOX(card2), lbl_c2_title, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(card2), lbl_cli_desc, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(card2), btn_launch_cli, FALSE, FALSE, 4);

gtk_grid_attach(GTK_GRID(grid), card1, 0, 0, 1, 1);
gtk_grid_attach(GTK_GRID(grid), card2, 1, 0, 1, 1);

GtkWidget *card3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
GtkStyleContext *c3_ctx = gtk_widget_get_style_context(card3);
gtk_style_context_add_class(c3_ctx, "card-frame");

GtkWidget *lbl_c3_title = gtk_label_new(NULL);
snprintf(buf_markup, sizeof(buf_markup), "<span font='12' font_weight='bold' foreground='#34d399'>%s</span>", controller_tr(ctx, "Operasi Pintas System"));
gtk_label_set_markup(GTK_LABEL(lbl_c3_title), buf_markup);
gtk_widget_set_halign(lbl_c3_title, GTK_ALIGN_START);

GtkWidget *action_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
GtkWidget *btn_check = gtk_button_new_with_label(controller_tr(ctx, "Periksa Pembaruan Paket"));
g_signal_connect(btn_check, "clicked", G_CALLBACK(on_quick_check_clicked), ctx);

GtkWidget *btn_store = gtk_button_new_with_label(controller_tr(ctx, "Buka Katalog Aplikasi"));
g_signal_connect(btn_store, "clicked", G_CALLBACK(on_quick_store_clicked), ctx);

gtk_box_pack_start(GTK_BOX(action_hbox), btn_check, TRUE, TRUE, 0);
gtk_box_pack_start(GTK_BOX(action_hbox), btn_store, TRUE, TRUE, 0);

gtk_box_pack_start(GTK_BOX(card3), lbl_c3_title, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(card3), action_hbox, FALSE, FALSE, 4);

gtk_grid_attach(GTK_GRID(grid), card3, 0, 1, 2, 1);

gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 0);

return vbox;



}
