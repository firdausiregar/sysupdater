#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include "../models/package_model.h"

typedef struct {
    AppModel *model;
    GtkWidget *main_window;
    GtkWidget *stack;

    /* Splash Screen Widgets */
    GtkWidget *splash_window;
    GtkWidget *splash_label;
    GtkWidget *splash_progress;

    /* Header Component Widgets */
    GtkWidget *lbl_hdr_title;
    GtkWidget *lbl_hdr_sub;

    /* View Stores Reference */
    GtkListStore *upgrade_store;
    GtkTreeModelFilter *upgrade_filter;
    GtkTreeStore *store_tree_store;
    GtkTreeModelFilter *store_filter;
    GtkListStore *blacklist_store;
    GtkListStore *installed_store;
    GtkTreeModelFilter *installed_filter;
    GtkListStore *history_store;
    GtkListStore *ppa_store;

    /* Live Search Filters */
    GtkWidget *upgrade_search_entry;
    GtkWidget *installed_search_entry;
    GtkWidget *store_search_entry;

    /* Sidebar Labels */
    GtkWidget *lbl_sb_dashboard;
    GtkWidget *lbl_sb_upgrade;
    GtkWidget *lbl_sb_store;
    GtkWidget *lbl_sb_blacklist;
    GtkWidget *lbl_sb_installed;
    GtkWidget *lbl_sb_history;
    GtkWidget *lbl_sb_cleaner;
    GtkWidget *lbl_sb_ppa;
    GtkWidget *lbl_sb_schedule;
    GtkWidget *lbl_sb_about;
    GtkWidget *lbl_sb_lang;

    GtkWidget *badge_lbl;
    GtkWidget *progress_box;
    GtkWidget *progress_bar;
    GtkWidget *status_pkg_label;
    GtkWidget *btn_cancel_op;

    GtkStatusIcon *tray_icon;
    GtkWidget *tray_menu;
    int last_notified_count;
    int current_pkg_count;
    char current_lang[16];

    FILE *pipe_fp;
    GPid active_pid;
} AppContext;

AppContext* controller_init(void);
const char* controller_tr(AppContext *ctx, const char *msgid);
bool controller_ensure_sudo_access(AppContext *ctx);
void controller_setup_tray_icon(AppContext *ctx);
gboolean controller_auto_check_updates(gpointer user_data);
void controller_switch_language(AppContext *ctx, const char *lang_code);
void controller_switch_page(AppContext *ctx, const char *page_name);

void controller_load_upgrades(AppContext *ctx);
void controller_toggle_all_upgrades(AppContext *ctx, gboolean select_all);
void controller_install_selected(AppContext *ctx);

void controller_load_store_categories(AppContext *ctx);
void controller_toggle_all_store(AppContext *ctx, gboolean select_all);
void controller_install_store_selected(AppContext *ctx);

void controller_load_blacklist_switches(AppContext *ctx);
void controller_toggle_package_hold(AppContext *ctx, const char *pkg_name, bool hold_active);

void controller_load_installed(AppContext *ctx);
void controller_load_history(AppContext *ctx);

void controller_load_ppa_list(AppContext *ctx);
void controller_add_ppa(AppContext *ctx, const char *ppa_input);
void controller_remove_ppa(AppContext *ctx, const char *ppa_input);

void controller_run_cleaner(AppContext *ctx);
void controller_save_schedule(AppContext *ctx, int interval_hours);
void controller_export_log(AppContext *ctx);
void controller_cancel_active_operation(AppContext *ctx);
void controller_load_store_categories_async(AppContext *ctx);

#endif
