#ifndef PACKAGE_MODEL_H
#define PACKAGE_MODEL_H

#include <stdbool.h>
#include <gtk/gtk.h>
#include "../app_config.h"

typedef enum {
    PKG_MANAGER_APT,
    PKG_MANAGER_DNF,
    PKG_MANAGER_PACMAN,
    PKG_MANAGER_LFS,
    PKG_MANAGER_UNKNOWN
} PkgManagerType;

typedef struct {
    char name[128];
    char category[64];
    char description[256];
    char old_version[64];
    char new_version[64];
    bool selected;
    bool is_hold;
} PkgInfo;

typedef struct {
    char ppa_name[256];
    char status[64];
    char file_source[128];
} PPAInfo;

typedef struct {
    PkgManagerType type;
    char sudo_password[256];
    bool has_saved_password;
    bool use_polkit;
    char os_pretty_name[128];
    char os_id[64];
    char os_arch[32];
    char os_kernel[64];
    char os_logo_path[256];
    char app_logo_path[256];

    char cpu_model[128];
    char ram_info[64];
    char swap_info[64];
    char disk_info[64];

    double ram_percentage;
    double swap_percentage;
    int total_updates_count;

    GHashTable *po_translations;
    GHashTable *hold_sessions;
    char active_po_lang[16];
    char user_cache_dir[512];
} AppModel;

AppModel* model_init(void);
void model_detect_system_info(AppModel *model);
bool model_verify_sudo_password(const char *password);
void model_save_sudo_password(AppModel *model, const char *password);
bool model_load_sudo_session(AppModel *model);

void model_save_hold_session(AppModel *model);
void model_load_hold_session(AppModel *model);

GList* model_get_upgradable_packages(AppModel *model);
GList* model_get_installed_packages(AppModel *model);
GList* model_get_upgrade_history(AppModel *model);
GList* model_get_live_categorized_repo(AppModel *model);
GList* model_get_ppa_list(AppModel *model);

bool model_set_package_hold_status(AppModel *model, const char *pkg_name, bool hold);
bool model_create_system_snapshot(AppModel *model, const char *comment);
bool model_clean_system_junk(AppModel *model);
bool model_add_ppa_repository(AppModel *model, const char *ppa_name);
bool model_remove_ppa_repository(AppModel *model, const char *ppa_name);
bool model_setup_auto_schedule(AppModel *model, int interval_hours);
bool model_fix_broken_locks(AppModel *model);

char* model_read_file_content(const char *filename);
GList* model_get_available_languages(void);
void model_load_po_file(AppModel *model, const char *lang_code);
const char* model_translate_text(AppModel *model, const char *msgid);

#endif
