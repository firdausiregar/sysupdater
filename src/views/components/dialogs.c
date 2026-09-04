#include "dialogs.h"

void dialog_show_cool_about(AppContext *ctx) {
char title[128];
snprintf(title, sizeof(title), "%s %s", controller_tr(ctx, "Tentang App"), APP_NAME);
GtkWidget *dialog = gtk_dialog_new_with_buttons(title, GTK_WINDOW(ctx->main_window), GTK_DIALOG_MODAL, controller_tr(ctx, "Tutup"), GTK_RESPONSE_CLOSE, NULL);
gtk_window_set_default_size(GTK_WINDOW(dialog), 650, 480);


GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
gtk_container_set_border_width(GTK_CONTAINER(content_area), 15);

GtkWidget *notebook = gtk_notebook_new();

GtkWidget *vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
GtkWidget *txt_umum = gtk_text_view_new();
gtk_text_view_set_editable(GTK_TEXT_VIEW(txt_umum), FALSE);
char *umum_txt = model_read_file_content("umum.md");
GtkTextBuffer *buf_umum = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txt_umum));
gtk_text_buffer_set_text(buf_umum, umum_txt, -1);
g_free(umum_txt);

GtkWidget *scr_umum = gtk_scrolled_window_new(NULL, NULL);
gtk_container_add(GTK_CONTAINER(scr_umum), txt_umum);
gtk_box_pack_start(GTK_BOX(vbox1), scr_umum, TRUE, TRUE, 0);

GtkWidget *vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
gtk_container_set_border_width(GTK_CONTAINER(vbox2), 15);

GtkWidget *logo_img = NULL;
if (strlen(ctx->model->os_logo_path) > 0 && g_file_test(ctx->model->os_logo_path, G_FILE_TEST_EXISTS)) {
    GdkPixbuf *pix = gdk_pixbuf_new_from_file_at_scale(ctx->model->os_logo_path, 64, 64, TRUE, NULL);
    if (pix) {
        logo_img = gtk_image_new_from_pixbuf(pix);
        g_object_unref(pix);
    }
}
if (!logo_img) logo_img = gtk_image_new_from_icon_name("computer", GTK_ICON_SIZE_DIALOG);

gtk_box_pack_start(GTK_BOX(vbox2), logo_img, FALSE, FALSE, 5);

char sys_info[1024];
snprintf(sys_info, sizeof(sys_info),
    "<b>OS Distro:</b> %s\n"
    "<b>Kernel Linux:</b> %s\n"
    "<b>Arsitektur:</b> %s\n"
    "<b>Model CPU:</b> %s\n"
    "<b>Memori RAM:</b> %s\n"
    "<b>Memori Swap:</b> %s\n"
    "<b>Penyimpanan Root:</b> %s",
    ctx->model->os_pretty_name,
    ctx->model->os_kernel,
    ctx->model->os_arch,
    ctx->model->cpu_model,
    ctx->model->ram_info,
    ctx->model->swap_info,
    ctx->model->disk_info);

GtkWidget *lbl_specs = gtk_label_new(NULL);
gtk_label_set_markup(GTK_LABEL(lbl_specs), sys_info);
gtk_box_pack_start(GTK_BOX(vbox2), lbl_specs, TRUE, TRUE, 0);

GtkWidget *vbox_doc = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
GtkWidget *txt_doc = gtk_text_view_new();
gtk_text_view_set_editable(GTK_TEXT_VIEW(txt_doc), FALSE);
char *readme_txt = model_read_file_content("dokumentasi");
GtkTextBuffer *buf_doc = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txt_doc));
gtk_text_buffer_set_text(buf_doc, readme_txt, -1);
g_free(readme_txt);

GtkWidget *scr_doc = gtk_scrolled_window_new(NULL, NULL);
gtk_container_add(GTK_CONTAINER(scr_doc), txt_doc);
gtk_box_pack_start(GTK_BOX(vbox_doc), scr_doc, TRUE, TRUE, 0);

GtkWidget *vbox_lic = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
GtkWidget *txt_lic = gtk_text_view_new();
gtk_text_view_set_editable(GTK_TEXT_VIEW(txt_lic), FALSE);
char *lic_txt = model_read_file_content("LICENSE");
GtkTextBuffer *buf_lic = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txt_lic));
gtk_text_buffer_set_text(buf_lic, lic_txt, -1);
g_free(lic_txt);

GtkWidget *scr_lic = gtk_scrolled_window_new(NULL, NULL);
gtk_container_add(GTK_CONTAINER(scr_lic), txt_lic);
gtk_box_pack_start(GTK_BOX(vbox_lic), scr_lic, TRUE, TRUE, 0);

gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox1, gtk_label_new("Umum"));
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox2, gtk_label_new("Spesifikasi"));
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox_doc, gtk_label_new("Dokumentasi"));
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox_lic, gtk_label_new("Lisensi"));

gtk_box_pack_start(GTK_BOX(content_area), notebook, TRUE, TRUE, 0);
gtk_widget_show_all(dialog);

g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);



}
