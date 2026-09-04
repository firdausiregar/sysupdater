#include "pages.h"

typedef struct {
AppContext *ctx;
GtkWidget *spin;
} SchedData;

static void on_save_sched(GtkWidget *w, gpointer data) {
SchedData *sdata = (SchedData*)data;
int val = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(sdata->spin));
controller_save_schedule(sdata->ctx, val);
}

GtkWidget* create_page_schedule(AppContext *ctx) {
GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);


GtkWidget *lbl_title = gtk_label_new(NULL);
char buf_markup[512];
snprintf(buf_markup, sizeof(buf_markup), "<span font='14' font_weight='bold' foreground='#fbbf24'>⏰ %s</span>", controller_tr(ctx, "Jadwal Otomatis"));
gtk_label_set_markup(GTK_LABEL(lbl_title), buf_markup);
gtk_widget_set_halign(lbl_title, GTK_ALIGN_START);

GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
GtkWidget *lbl_interval = gtk_label_new(controller_tr(ctx, "Interval Pengecekan Latar Belakang (Jam):"));
GtkWidget *spin = gtk_spin_button_new_with_range(1, 72, 1);
gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), 6);

gtk_box_pack_start(GTK_BOX(hbox), lbl_interval, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);

SchedData *sdata = g_new0(SchedData, 1);
sdata->ctx = ctx;
sdata->spin = spin;

GtkWidget *btn_save = gtk_button_new_with_label(controller_tr(ctx, "Simpan Pengaturan Cron Timer"));
g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save_sched), sdata);

gtk_box_pack_start(GTK_BOX(vbox), lbl_title, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(vbox), btn_save, FALSE, FALSE, 0);

return vbox;



}
