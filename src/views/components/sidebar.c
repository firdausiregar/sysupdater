#include "sidebar.h"
#include "dialogs.h"
#include "../update_checker.h"

typedef struct {
AppContext *ctx;
char lang[16];
} LangData;

typedef struct {
    AppContext *ctx;
    const char *page;
} NavData;

static void on_sidebar_btn_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;

    NavData *nav = (NavData *)user_data;
    controller_switch_page(nav->ctx, nav->page);
}

static void on_about_btn_clicked(GtkButton *btn, gpointer user_data) {
dialog_show_cool_about((AppContext*)user_data);
}

static void on_dynamic_lang_activate(GtkMenuItem *m, gpointer data) {
LangData *lang_data = (LangData*)data;
controller_switch_language(lang_data->ctx, lang_data->lang);
}

GtkWidget* create_sidebar_component(AppContext *ctx) {
GtkWidget *sidebar_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
gtk_container_set_border_width(GTK_CONTAINER(sidebar_vbox), 12);
gtk_widget_set_size_request(sidebar_vbox, 200, -1);


GtkWidget *lbl_brand = gtk_label_new(NULL);
gtk_label_set_markup(GTK_LABEL(lbl_brand), "<span font='16' font_weight='bold' foreground='#38bdf8'>SYSUPDATER</span>");
gtk_widget_set_halign(lbl_brand, GTK_ALIGN_START);
gtk_box_pack_start(GTK_BOX(sidebar_vbox), lbl_brand, FALSE, FALSE, 8);

ctx->lbl_sb_dashboard = gtk_label_new(controller_tr(ctx, "Dashboard"));
ctx->lbl_sb_upgrade = gtk_label_new(controller_tr(ctx, "Cek & Update"));
ctx->lbl_sb_store = gtk_label_new(controller_tr(ctx, "Katalog Aplikasi"));
ctx->lbl_sb_blacklist = gtk_label_new(controller_tr(ctx, "Disable Update"));
ctx->lbl_sb_installed = gtk_label_new(controller_tr(ctx, "Paket Terinstall"));
ctx->lbl_sb_history = gtk_label_new(controller_tr(ctx, "Riwayat Update"));
ctx->lbl_sb_cleaner = gtk_label_new(controller_tr(ctx, "Cleaner System"));
ctx->lbl_sb_ppa = gtk_label_new(controller_tr(ctx, "Manajemen PPA"));
ctx->lbl_sb_schedule = gtk_label_new(controller_tr(ctx, "Jadwal Otomatis"));

struct { GtkWidget **lbl; const char *icon; const char *page; } nav_items[] = {
    {&ctx->lbl_sb_dashboard, "go-home", "page_dashboard"},
    {&ctx->lbl_sb_upgrade, "software-update-available", "page_upgrade"},
    {&ctx->lbl_sb_store, "applications-other", "page_store"},
    {&ctx->lbl_sb_blacklist, "dialog-warning", "page_blacklist"},
    {&ctx->lbl_sb_installed, "package-x-generic", "page_installed"},
    {&ctx->lbl_sb_history, "document-open-recent", "page_history"},
    {&ctx->lbl_sb_cleaner, "edit-clear", "page_cleaner"},
    {&ctx->lbl_sb_ppa, "folder-remote", "page_ppa"},
    {&ctx->lbl_sb_schedule, "preferences-system-time", "page_schedule"}
};

for (int i = 0; i < 9; i++) {
    GtkWidget *btn = gtk_button_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *icon = gtk_image_new_from_icon_name(nav_items[i].icon, GTK_ICON_SIZE_BUTTON);

    gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), *nav_items[i].lbl, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(btn), box);

    NavData *nav = g_new0(NavData, 1);
    nav->ctx = ctx;
    nav->page = nav_items[i].page;

    g_signal_connect(btn, "clicked", G_CALLBACK(on_sidebar_btn_clicked), nav);
    gtk_box_pack_start(GTK_BOX(sidebar_vbox), btn, FALSE, FALSE, 0);
}

GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
gtk_box_pack_start(GTK_BOX(sidebar_vbox), separator, FALSE, FALSE, 6);

GtkWidget *menu_lang = gtk_menu_new();
GList *available_langs = model_get_available_languages();

for (GList *l = available_langs; l != NULL; l = l->next) {
    char *lang_code = (char*)l->data;
    char item_label[64];
    
    if (strcmp(lang_code, "id") == 0) snprintf(item_label, sizeof(item_label), "Bahasa Indonesia");
    else if (strcmp(lang_code, "en") == 0) snprintf(item_label, sizeof(item_label), "English");
    else snprintf(item_label, sizeof(item_label), "Bahasa (%s)", lang_code);

    GtkWidget *m_item = gtk_menu_item_new_with_label(item_label);
    LangData *lang_data = g_new0(LangData, 1);
    lang_data->ctx = ctx;
    strncpy(lang_data->lang, lang_code, sizeof(lang_data->lang) - 1);

    g_signal_connect_data(m_item, "activate", G_CALLBACK(on_dynamic_lang_activate), lang_data, (GClosureNotify)g_free, 0);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_lang), m_item);
}
g_list_free_full(available_langs, g_free);
gtk_widget_show_all(menu_lang);

GtkWidget *btn_lang = gtk_menu_button_new();
gtk_menu_button_set_popup(GTK_MENU_BUTTON(btn_lang), menu_lang);

GtkWidget *box_lang = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
GtkWidget *icon_lang = gtk_image_new_from_icon_name("preferences-desktop-locale", GTK_ICON_SIZE_BUTTON);
ctx->lbl_sb_lang = gtk_label_new(controller_tr(ctx, "Bahasa"));
gtk_box_pack_start(GTK_BOX(box_lang), icon_lang, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(box_lang), ctx->lbl_sb_lang, FALSE, FALSE, 0);
gtk_container_add(GTK_CONTAINER(btn_lang), box_lang);

gtk_box_pack_end(GTK_BOX(sidebar_vbox), btn_lang, FALSE, FALSE, 0);

GtkWidget *btn_about = gtk_button_new();
GtkWidget *box_ab = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
GtkWidget *icon_ab = gtk_image_new_from_icon_name("help-about", GTK_ICON_SIZE_BUTTON);
ctx->lbl_sb_about = gtk_label_new(controller_tr(ctx, "Tentang App"));
gtk_box_pack_start(GTK_BOX(box_ab), icon_ab, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(box_ab), ctx->lbl_sb_about, FALSE, FALSE, 0);
gtk_container_add(GTK_CONTAINER(btn_about), box_ab);
g_signal_connect(btn_about, "clicked", G_CALLBACK(on_about_btn_clicked), ctx);

gtk_box_pack_end(GTK_BOX(sidebar_vbox), btn_about, FALSE, FALSE, 0);

return sidebar_vbox;



}
