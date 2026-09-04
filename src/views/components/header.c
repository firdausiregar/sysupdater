#include "header.h"

GtkWidget* create_header_component(AppContext *ctx) {
GtkWidget *banner_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
GtkStyleContext *b_ctx = gtk_widget_get_style_context(banner_box);
gtk_style_context_add_class(b_ctx, "header-banner");


GtkWidget *logo_img = NULL;
if (g_file_test(ctx->model->app_logo_path, G_FILE_TEST_EXISTS)) {
    GdkPixbuf *pix = gdk_pixbuf_new_from_file_at_scale(ctx->model->app_logo_path, 48, 48, TRUE, NULL);
    if (pix) {
        logo_img = gtk_image_new_from_pixbuf(pix);
        g_object_unref(pix);
    }
}
if (!logo_img) logo_img = gtk_image_new_from_icon_name("system-software-update", GTK_ICON_SIZE_LARGE_TOOLBAR);

GtkWidget *banner_text_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
ctx->lbl_hdr_title = gtk_label_new(APP_NAME);
gtk_widget_set_halign(ctx->lbl_hdr_title, GTK_ALIGN_START);
GtkStyleContext *bt_ctx = gtk_widget_get_style_context(ctx->lbl_hdr_title);
gtk_style_context_add_class(bt_ctx, "header-title");

GtkWidget *badge_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
GtkWidget *distro_icon = NULL;

if (strlen(ctx->model->os_logo_path) > 0 && g_file_test(ctx->model->os_logo_path, G_FILE_TEST_EXISTS)) {
    GdkPixbuf *pix_distro = gdk_pixbuf_new_from_file_at_scale(ctx->model->os_logo_path, 20, 20, TRUE, NULL);
    if (pix_distro) {
        distro_icon = gtk_image_new_from_pixbuf(pix_distro);
        g_object_unref(pix_distro);
    }
}
if (!distro_icon) distro_icon = gtk_image_new_from_icon_name("computer", GTK_ICON_SIZE_MENU);

char badge_os[256];
snprintf(badge_os, sizeof(badge_os), "Distro: %s (%s)", ctx->model->os_pretty_name, ctx->model->os_arch);
GtkWidget *os_badge_lbl = gtk_label_new(badge_os);

gtk_box_pack_start(GTK_BOX(badge_box), distro_icon, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(badge_box), os_badge_lbl, FALSE, FALSE, 0);

GtkStyleContext *ob_ctx = gtk_widget_get_style_context(badge_box);
gtk_style_context_add_class(ob_ctx, "os-badge");

gtk_box_pack_start(GTK_BOX(banner_text_vbox), ctx->lbl_hdr_title, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(banner_text_vbox), badge_box, FALSE, FALSE, 2);

gtk_box_pack_start(GTK_BOX(banner_box), logo_img, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(banner_box), banner_text_vbox, TRUE, TRUE, 0);

return banner_box;



}
