#include "app_controller.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libnotify/notify.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

const char* controller_tr(AppContext *ctx, const char *msgid) {
    if (!ctx || !ctx->model) return msgid;
    return model_translate_text(ctx->model, msgid);
}

AppContext* controller_init(void) {
    AppContext *ctx = g_new0(AppContext, 1);
    ctx->model = model_init();
    strncpy(ctx->current_lang, ctx->model->active_po_lang, sizeof(ctx->current_lang));
    ctx->last_notified_count = -1;
    notify_init(APP_NAME);
    return ctx;
}

static void show_popup(GtkWindow *parent, GtkMessageType type, const char *title, const char *msg) {
    GtkWidget *dialog = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, type, GTK_BUTTONS_OK, "%s", title);
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", msg);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

bool controller_ensure_sudo_access(AppContext *ctx) {
    if (ctx->model->has_saved_password) {
        return true;
    }

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Autentikasi Sudo Diperlukan",
                                                    GTK_WINDOW(ctx->main_window),
                                                    GTK_DIALOG_MODAL,
                                                    "Batal", GTK_RESPONSE_CANCEL,
                                                    "Verifikasi", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 15);

    GtkWidget *lbl_info = gtk_label_new("🔒 Fitur ini memerlukan akses administrator (Sudo).\nMasukkan password Anda untuk melanjutkan:");
    gtk_box_pack_start(GTK_BOX(content_area), lbl_info, FALSE, FALSE, 5);

    GtkWidget *entry_pass = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry_pass), FALSE);
    gtk_box_pack_start(GTK_BOX(content_area), entry_pass, FALSE, FALSE, 10);

    gtk_widget_show_all(dialog);

    bool verified = false;
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char *pass = gtk_entry_get_text(GTK_ENTRY(entry_pass));
        if (model_verify_sudo_password(pass)) {
            model_save_sudo_password(ctx->model, pass);
            verified = true;
        } else {
            show_popup(GTK_WINDOW(ctx->main_window), GTK_MESSAGE_ERROR, "Verifikasi Gagal!", "Password Sudo yang Anda masukkan salah.");
        }
    }

    gtk_widget_destroy(dialog);
    return verified;
}

static void on_tray_activate(GtkStatusIcon *icon, gpointer user_data) {
    AppContext *ctx = (AppContext*)user_data;
    if (gtk_widget_get_visible(ctx->main_window)) {
        gtk_widget_hide(ctx->main_window);
    } else {
        gtk_widget_show_all(ctx->main_window);
        gtk_window_deiconify(GTK_WINDOW(ctx->main_window));
    }
}

static void on_tray_popup_menu(GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data) {
    AppContext *ctx = (AppContext*)user_data;
    if (ctx->tray_menu) {
        gtk_menu_popup(GTK_MENU(ctx->tray_menu), NULL, NULL, gtk_status_icon_position_menu, status_icon, button, activate_time);
    }
}

static void on_tray_menu_open(GtkMenuItem *m, gpointer data) {
    AppContext *ctx = (AppContext*)data;
    gtk_widget_show_all(ctx->main_window);
    gtk_window_deiconify(GTK_WINDOW(ctx->main_window));
}

static void on_tray_menu_check(GtkMenuItem *m, gpointer data) {
    AppContext *ctx = (AppContext*)data;
    controller_switch_page(ctx, "page_upgrade");
    gtk_widget_show_all(ctx->main_window);
}

static void on_tray_menu_quit(GtkMenuItem *m, gpointer data) {
    gtk_main_quit();
}

void controller_setup_tray_icon(AppContext *ctx) {
    char icon_path[512] = {0};
    if (g_file_test("assets/logoku.png", G_FILE_TEST_EXISTS)) {
        strcpy(icon_path, "assets/logoku.png");
    } else if (g_file_test(ctx->model->app_logo_path, G_FILE_TEST_EXISTS)) {
        strcpy(icon_path, ctx->model->app_logo_path);
    }

    if (strlen(icon_path) > 0) {
        ctx->tray_icon = gtk_status_icon_new_from_file(icon_path);
    } else {
        ctx->tray_icon = gtk_status_icon_new_from_icon_name("system-software-update");
    }

    gtk_status_icon_set_tooltip_text(ctx->tray_icon, APP_NAME);
    g_signal_connect(ctx->tray_icon, "activate", G_CALLBACK(on_tray_activate), ctx);
    g_signal_connect(ctx->tray_icon, "popup-menu", G_CALLBACK(on_tray_popup_menu), ctx);

    ctx->tray_menu = gtk_menu_new();
    GtkWidget *m_open = gtk_menu_item_new_with_label(controller_tr(ctx, "Buka Aplikasi Utama"));
    GtkWidget *m_check = gtk_menu_item_new_with_label(controller_tr(ctx, "Cek Pembaruan Package"));
    GtkWidget *m_sep = gtk_separator_menu_item_new();
    GtkWidget *m_quit = gtk_menu_item_new_with_label(controller_tr(ctx, "Keluar"));

    g_signal_connect(m_open, "activate", G_CALLBACK(on_tray_menu_open), ctx);
    g_signal_connect(m_check, "activate", G_CALLBACK(on_tray_menu_check), ctx);
    g_signal_connect(m_quit, "activate", G_CALLBACK(on_tray_menu_quit), ctx);

    gtk_menu_shell_append(GTK_MENU_SHELL(ctx->tray_menu), m_open);
    gtk_menu_shell_append(GTK_MENU_SHELL(ctx->tray_menu), m_check);
    gtk_menu_shell_append(GTK_MENU_SHELL(ctx->tray_menu), m_sep);
    gtk_menu_shell_append(GTK_MENU_SHELL(ctx->tray_menu), m_quit);
    gtk_widget_show_all(ctx->tray_menu);

    gtk_status_icon_set_visible(ctx->tray_icon, TRUE);

    controller_auto_check_updates(ctx);
    g_timeout_add_seconds(240, controller_auto_check_updates, ctx);
}

gboolean controller_auto_check_updates(gpointer user_data) {
    AppContext *ctx = (AppContext*)user_data;
    GList *upgrades = model_get_upgradable_packages(ctx->model);
    int count = g_list_length(upgrades);

    if (count > 0) {
        char tooltip_msg[256];
        snprintf(tooltip_msg, sizeof(tooltip_msg), "%s: %d Paket Pembaruan Tersedia!", APP_NAME, count);
        gtk_status_icon_set_tooltip_text(ctx->tray_icon, tooltip_msg);

        if (count != ctx->last_notified_count) {
            ctx->last_notified_count = count;
            char body[256];
            snprintf(body, sizeof(body), "Ada %d pembaruan paket yang tersedia!", count);
            NotifyNotification *n = notify_notification_new(APP_NAME, body, "dialog-information");
            notify_notification_show(n, NULL);
            g_object_unref(G_OBJECT(n));
        }
    } else {
        gtk_status_icon_set_tooltip_text(ctx->tray_icon, APP_NAME);
    }

    g_list_free_full(upgrades, g_free);
    return TRUE;
}

void controller_switch_language(AppContext *ctx, const char *lang_code) {
    if (!ctx || !lang_code) return;
    strncpy(ctx->current_lang, lang_code, sizeof(ctx->current_lang));
    model_load_po_file(ctx->model, lang_code);

    if (ctx->lbl_sb_dashboard) gtk_label_set_text(GTK_LABEL(ctx->lbl_sb_dashboard), controller_tr(ctx, "Dashboard"));
    if (ctx->lbl_sb_upgrade) gtk_label_set_text(GTK_LABEL(ctx->lbl_sb_upgrade), controller_tr(ctx, "Cek & Update"));
    if (ctx->lbl_sb_store) gtk_label_set_text(GTK_LABEL(ctx->lbl_sb_store), controller_tr(ctx, "Katalog Aplikasi"));
    if (ctx->lbl_sb_blacklist) gtk_label_set_text(GTK_LABEL(ctx->lbl_sb_blacklist), controller_tr(ctx, "Disable Update"));
    if (ctx->lbl_sb_installed) gtk_label_set_text(GTK_LABEL(ctx->lbl_sb_installed), controller_tr(ctx, "Paket Terinstall"));
    if (ctx->lbl_sb_history) gtk_label_set_text(GTK_LABEL(ctx->lbl_sb_history), controller_tr(ctx, "Riwayat Update"));
    if (ctx->lbl_sb_cleaner) gtk_label_set_text(GTK_LABEL(ctx->lbl_sb_cleaner), controller_tr(ctx, "Cleaner System"));
    if (ctx->lbl_sb_ppa) gtk_label_set_text(GTK_LABEL(ctx->lbl_sb_ppa), controller_tr(ctx, "Manajemen PPA"));
    if (ctx->lbl_sb_schedule) gtk_label_set_text(GTK_LABEL(ctx->lbl_sb_schedule), controller_tr(ctx, "Jadwal Otomatis"));
    if (ctx->lbl_sb_about) gtk_label_set_text(GTK_LABEL(ctx->lbl_sb_about), controller_tr(ctx, "Tentang App"));
    if (ctx->lbl_sb_lang) gtk_label_set_text(GTK_LABEL(ctx->lbl_sb_lang), controller_tr(ctx, "Bahasa"));

    if (ctx->stack) {
        const char *visible_page = gtk_stack_get_visible_child_name(GTK_STACK(ctx->stack));
        if (visible_page) {
            controller_switch_page(ctx, visible_page);
        }
    }
}

void controller_switch_page(AppContext *ctx, const char *page_name) {
    if (!ctx || !ctx->stack) return;
    gtk_stack_set_visible_child_name(GTK_STACK(ctx->stack), page_name);

    if (strcmp(page_name, "page_upgrade") == 0) controller_load_upgrades(ctx);
    else if (strcmp(page_name, "page_store") == 0) controller_load_store_categories_async(ctx); /* Ganti pakai _async */
    else if (strcmp(page_name, "page_blacklist") == 0) controller_load_blacklist_switches(ctx);
    else if (strcmp(page_name, "page_installed") == 0) controller_load_installed(ctx);
    else if (strcmp(page_name, "page_history") == 0) controller_load_history(ctx);
    else if (strcmp(page_name, "page_ppa") == 0) controller_load_ppa_list(ctx);
}

void controller_load_upgrades(AppContext *ctx) {
    if (!ctx->upgrade_store) return;
    gtk_list_store_clear(ctx->upgrade_store);
    GList *upgrades = model_get_upgradable_packages(ctx->model);

    ctx->current_pkg_count = 0;
    for (GList *l = upgrades; l != NULL; l = l->next) {
        PkgInfo *info = (PkgInfo*)l->data;
        GtkTreeIter iter;
        gtk_list_store_append(ctx->upgrade_store, &iter);
        gtk_list_store_set(ctx->upgrade_store, &iter, 0, info->selected, 1, info->name, 2, info->old_version, 3, info->new_version, -1);
        ctx->current_pkg_count++;
    }

    if (ctx->badge_lbl) {
        char badge_text[128];
        snprintf(badge_text, sizeof(badge_text), "Total Paket Siap Di-Update: %d", ctx->current_pkg_count);
        gtk_label_set_text(GTK_LABEL(ctx->badge_lbl), badge_text);
    }
    
    g_list_free_full(upgrades, g_free);
}

void controller_toggle_all_upgrades(AppContext *ctx, gboolean select_all) {
    GtkTreeIter iter;
    gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(ctx->upgrade_store), &iter);
    while (valid) {
        gtk_list_store_set(ctx->upgrade_store, &iter, 0, select_all, -1);
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(ctx->upgrade_store), &iter);
    }
}

void controller_toggle_all_store(AppContext *ctx, gboolean select_all) {
    GtkTreeIter parent_iter;
    gboolean valid_parent = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(ctx->store_tree_store), &parent_iter);

    while (valid_parent) {
        GtkTreeIter child_iter;
        gboolean valid_child = gtk_tree_model_iter_children(GTK_TREE_MODEL(ctx->store_tree_store), &child_iter, &parent_iter);

        while (valid_child) {
            gtk_tree_store_set(ctx->store_tree_store, &child_iter, 0, select_all, -1);
            valid_child = gtk_tree_model_iter_next(GTK_TREE_MODEL(ctx->store_tree_store), &child_iter);
        }
        valid_parent = gtk_tree_model_iter_next(GTK_TREE_MODEL(ctx->store_tree_store), &parent_iter);
    }
}

void controller_load_store_categories(AppContext *ctx) {
    if (!ctx->store_tree_store) return;
    gtk_tree_store_clear(ctx->store_tree_store);

    GList *repo_pkgs = model_get_live_categorized_repo(ctx->model);
    GHashTable *cat_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    for (GList *l = repo_pkgs; l != NULL; l = l->next) {
        PkgInfo *info = (PkgInfo*)l->data;
        GtkTreeIter *cat_iter = g_hash_table_lookup(cat_table, info->category);

        if (!cat_iter) {
            cat_iter = g_new0(GtkTreeIter, 1);
            gtk_tree_store_append(ctx->store_tree_store, cat_iter, NULL);
            gtk_tree_store_set(ctx->store_tree_store, cat_iter, 0, FALSE, 1, info->category, 2, "Klik memperluas", 3, TRUE, -1);
            g_hash_table_insert(cat_table, g_strdup(info->category), cat_iter);
        }

        GtkTreeIter child_iter;
        gtk_tree_store_append(ctx->store_tree_store, &child_iter, cat_iter);
        gtk_tree_store_set(ctx->store_tree_store, &child_iter, 0, FALSE, 1, info->name, 2, info->description, 3, FALSE, -1);
    }

    g_hash_table_destroy(cat_table);
    g_list_free_full(repo_pkgs, g_free);
}

void controller_load_blacklist_switches(AppContext *ctx) {
    if (!ctx->blacklist_store) return;
    gtk_list_store_clear(ctx->blacklist_store);
    GList *installed = model_get_installed_packages(ctx->model);

    for (GList *l = installed; l != NULL; l = l->next) {
        PkgInfo *info = (PkgInfo*)l->data;
        GtkTreeIter iter;
        gtk_list_store_append(ctx->blacklist_store, &iter);
        gtk_list_store_set(ctx->blacklist_store, &iter, 0, info->name, 1, info->is_hold, -1);
    }
    
    g_list_free_full(installed, g_free);
}

void controller_load_installed(AppContext *ctx) {
    if (!ctx->installed_store) return;
    gtk_list_store_clear(ctx->installed_store);
    GList *installed = model_get_installed_packages(ctx->model);

    for (GList *l = installed; l != NULL; l = l->next) {
        PkgInfo *info = (PkgInfo*)l->data;
        GtkTreeIter iter;
        gtk_list_store_append(ctx->installed_store, &iter);
        gtk_list_store_set(ctx->installed_store, &iter, 0, FALSE, 1, info->name, 2, info->old_version, -1);
    }
    
    g_list_free_full(installed, g_free);
}

void controller_load_history(AppContext *ctx) {
    if (!ctx->history_store) return;
    gtk_list_store_clear(ctx->history_store);
    GList *history = model_get_upgrade_history(ctx->model);

    for (GList *l = history; l != NULL; l = l->next) {
        PkgInfo *info = (PkgInfo*)l->data;
        GtkTreeIter iter;
        gtk_list_store_append(ctx->history_store, &iter);
        gtk_list_store_set(ctx->history_store, &iter, 0, FALSE, 1, info->name, 2, info->old_version, 3, info->new_version, -1);
    }

    g_list_free_full(history, g_free);
}

void controller_load_ppa_list(AppContext *ctx) {
    if (!ctx->ppa_store) return;
    gtk_list_store_clear(ctx->ppa_store);
    GList *ppas = model_get_ppa_list(ctx->model);

    for (GList *l = ppas; l != NULL; l = l->next) {
        PPAInfo *info = (PPAInfo*)l->data;
        GtkTreeIter iter;
        gtk_list_store_append(ctx->ppa_store, &iter);
        gtk_list_store_set(ctx->ppa_store, &iter, 0, info->ppa_name, 1, info->file_source, 2, info->status, -1);
    }

    g_list_free_full(ppas, g_free);
}

static gboolean on_read_progress(GIOChannel *channel, GIOCondition cond, gpointer user_data) {
    AppContext *ctx = (AppContext*)user_data;
    gchar *line = NULL;
    gsize length = 0;

    if (cond & (G_IO_IN | G_IO_PRI)) {
        if (g_io_channel_read_line(channel, &line, &length, NULL, NULL) == G_IO_STATUS_NORMAL) {
            if (line) {
                gtk_progress_bar_pulse(GTK_PROGRESS_BAR(ctx->progress_bar));
                g_free(line);
                return TRUE;
            }
        }
    }

    if (cond & (G_IO_HUP | G_IO_ERR)) {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ctx->progress_bar), 1.0);
        gtk_label_set_text(GTK_LABEL(ctx->status_pkg_label), "✔ Transaksi Paket Selesai!");
        return FALSE;
    }

    return TRUE;
}

void controller_install_selected(AppContext *ctx) {
    if (!controller_ensure_sudo_access(ctx)) return;

    model_create_system_snapshot(ctx->model, "Pre-Update Snapshot via SysUpdater");

    GString *pkg_str = g_string_new("");
    GtkTreeIter iter;
    gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(ctx->upgrade_store), &iter);

    while (valid) {
        gboolean is_selected = FALSE;
        gchar *pkg_name = NULL;
        gtk_tree_model_get(GTK_TREE_MODEL(ctx->upgrade_store), &iter, 0, &is_selected, 1, &pkg_name, -1);

        if (is_selected && pkg_name) {
            g_string_append_printf(pkg_str, "%s ", pkg_name);
        }

        if (pkg_name) g_free(pkg_name);
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(ctx->upgrade_store), &iter);
    }

    gtk_widget_show_all(ctx->progress_box);
    char cmd[2048];
    if (ctx->model->type == PKG_MANAGER_APT) snprintf(cmd, sizeof(cmd), "echo '%s' | sudo -S apt-get install -y %s 2>&1", ctx->model->sudo_password, pkg_str->str);
    else snprintf(cmd, sizeof(cmd), "echo '%s' | sudo -S dnf install -y %s 2>&1", ctx->model->sudo_password, pkg_str->str);
    g_string_free(pkg_str, TRUE);

    ctx->pipe_fp = popen(cmd, "r");
    if (ctx->pipe_fp) {
        int fd = fileno(ctx->pipe_fp);
        GIOChannel *channel = g_io_channel_unix_new(fd);
        g_io_add_watch(channel, G_IO_IN | G_IO_PRI | G_IO_HUP | G_IO_ERR, on_read_progress, ctx);
        g_io_channel_unref(channel);
    }
}

void controller_install_store_selected(AppContext *ctx) {
    if (!controller_ensure_sudo_access(ctx)) return;

    GString *pkg_str = g_string_new("");
    GtkTreeIter parent_iter;
    gboolean valid_parent = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(ctx->store_tree_store), &parent_iter);

    int selected_count = 0;
    while (valid_parent) {
        GtkTreeIter child_iter;
        gboolean valid_child = gtk_tree_model_iter_children(GTK_TREE_MODEL(ctx->store_tree_store), &child_iter, &parent_iter);

        while (valid_child) {
            gboolean selected = FALSE;
            gchar *pkg_name = NULL;
            gboolean is_category = FALSE;

            gtk_tree_model_get(GTK_TREE_MODEL(ctx->store_tree_store), &child_iter, 0, &selected, 1, &pkg_name, 3, &is_category, -1);

            if (selected && !is_category && pkg_name) {
                g_string_append_printf(pkg_str, "%s ", pkg_name);
                selected_count++;
            }

            if (pkg_name) g_free(pkg_name);
            valid_child = gtk_tree_model_iter_next(GTK_TREE_MODEL(ctx->store_tree_store), &child_iter);
        }

        valid_parent = gtk_tree_model_iter_next(GTK_TREE_MODEL(ctx->store_tree_store), &parent_iter);
    }

    if (selected_count == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(ctx->main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Pilih Aplikasi!");
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "Silakan centang setidaknya satu aplikasi dari katalog untuk di-install.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        g_string_free(pkg_str, TRUE);
        return;
    }

    if (ctx->progress_box) gtk_widget_show_all(ctx->progress_box);
    char cmd[2048];
    if (ctx->model->type == PKG_MANAGER_APT) {
        snprintf(cmd, sizeof(cmd), "echo '%s' | sudo -S apt-get install -y %s 2>&1", ctx->model->sudo_password, pkg_str->str);
    } else {
        snprintf(cmd, sizeof(cmd), "echo '%s' | sudo -S dnf install -y %s 2>&1", ctx->model->sudo_password, pkg_str->str);
    }
    g_string_free(pkg_str, TRUE);

    ctx->pipe_fp = popen(cmd, "r");
    if (ctx->pipe_fp) {
        int fd = fileno(ctx->pipe_fp);
        GIOChannel *channel = g_io_channel_unix_new(fd);
        g_io_add_watch(channel, G_IO_IN | G_IO_PRI | G_IO_HUP | G_IO_ERR, on_read_progress, ctx);
        g_io_channel_unref(channel);
    }
}

void controller_toggle_package_hold(AppContext *ctx, const char *pkg_name, bool hold_active) {
    if (!controller_ensure_sudo_access(ctx)) return;
    if (model_set_package_hold_status(ctx->model, pkg_name, hold_active)) {
        show_popup(GTK_WINDOW(ctx->main_window), GTK_MESSAGE_INFO, "Status Sesi Tersimpan", "Perubahan saklar berhasil dikunci secara permanen.");
    }
}

void controller_run_cleaner(AppContext *ctx) {
    if (!controller_ensure_sudo_access(ctx)) return;
    if (model_clean_system_junk(ctx->model)) {
        show_popup(GTK_WINDOW(ctx->main_window), GTK_MESSAGE_INFO, "Pembersihan Selesai!", "Cache sistem dan paket orphan berhasil dibersihkan.");
    }
}

void controller_add_ppa(AppContext *ctx, const char *ppa_input) {
    if (!controller_ensure_sudo_access(ctx)) return;
    if (model_add_ppa_repository(ctx->model, ppa_input)) {
        show_popup(GTK_WINDOW(ctx->main_window), GTK_MESSAGE_INFO, "Repositori Ditambahkan", "Repositori PPA berhasil dikonfigurasi.");
        controller_load_ppa_list(ctx);
    }
}

void controller_remove_ppa(AppContext *ctx, const char *ppa_input) {
    if (!controller_ensure_sudo_access(ctx)) return;
    if (model_remove_ppa_repository(ctx->model, ppa_input)) {
        show_popup(GTK_WINDOW(ctx->main_window), GTK_MESSAGE_INFO, "Repositori Dihapus", "Repositori PPA berhasil dihapus dari sistem.");
        controller_load_ppa_list(ctx);
    }
}

void controller_save_schedule(AppContext *ctx, int interval_hours) {
    if (model_setup_auto_schedule(ctx->model, interval_hours)) {
        show_popup(GTK_WINDOW(ctx->main_window), GTK_MESSAGE_INFO, "Jadwal Disimpan", "Cron Job pembaruan otomatis berhasil didaftarkan.");
    }
}

void controller_export_log(AppContext *ctx) {
    system("mkdir -p ~/Desktop && grep -E 'install |upgrade ' /var/log/dpkg.log > ~/Desktop/sysupdater_export_log.txt 2>/dev/null || true");
    show_popup(GTK_WINDOW(ctx->main_window), GTK_MESSAGE_INFO, "Export Berhasil!", "Log disimpan di ~/Desktop/sysupdater_export_log.txt");
}



typedef struct {
    AppContext *ctx;
    GList *result_list;
} AsyncSearchData;

static void search_thread_func(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable) {
    AsyncSearchData *data = (AsyncSearchData*)task_data;
    data->result_list = model_get_live_categorized_repo(data->ctx->model);
    g_task_return_pointer(task, data, NULL);
}

static void search_finish_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    AsyncSearchData *data = (AsyncSearchData*)g_task_propagate_pointer(G_TASK(res), NULL);
    if (data && data->ctx) {
        controller_load_store_categories(data->ctx);
        if (data->ctx->progress_bar) {
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(data->ctx->progress_bar), 1.0);
        }
    }
}

void controller_load_store_categories_async(AppContext *ctx) {
    GTask *task = g_task_new(NULL, NULL, search_finish_callback, NULL);
    AsyncSearchData *data = g_new0(AsyncSearchData, 1);
    data->ctx = ctx;
    
    g_task_set_task_data(task, data, (GDestroyNotify)g_free);
    g_task_run_in_thread(task, search_thread_func);
    g_object_unref(task);
}

#pragma GCC diagnostic pop
