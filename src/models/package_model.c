#include "package_model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>


static void init_user_cache_dir(AppModel *model) {
    const char *home = getenv("HOME");
    if (!home) home = "/tmp";
    snprintf(model->user_cache_dir, sizeof(model->user_cache_dir), "%s/.cache/%s", home, APP_CACHE_SUBDIR);
    
    char mkdir_cmd[512];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s' 2>/dev/null || true", model->user_cache_dir);
    system(mkdir_cmd);
}


static char *model_get_user_config_file(const char *filename) {
    const char *config_root = g_get_user_config_dir();
    char *config_dir = g_build_filename(config_root, APP_ID, NULL);

    if (g_mkdir_with_parents(config_dir, 0700) != 0) {
        g_warning("Unable to create SysUpdater config directory: %s", config_dir);
        g_free(config_dir);
        return NULL;
    }

    char *path = g_build_filename(config_dir, filename, NULL);
    g_free(config_dir);
    return path;
}

static const char* find_distro_logo_path(const char *os_id) {
    static const char *candidate_paths[] = {
        "/usr/share/pixmaps/ubuntu-logo.png",
        "/usr/share/pixmaps/debian-logo.png",
        "/usr/share/pixmaps/fedora-logo.png",
        "/usr/share/pixmaps/archlinux-logo.png",
        "/usr/share/icons/hicolor/scalable/apps/ubuntu-logo.svg",
        "/usr/share/icons/hicolor/48x48/apps/debian-logo.png",
        "/usr/share/pixmaps/gnome-foot.png",
        NULL
    };

    for (int i = 0; candidate_paths[i] != NULL; i++) {
        if (g_file_test(candidate_paths[i], G_FILE_TEST_EXISTS)) {
            if (strstr(candidate_paths[i], os_id)) {
                return candidate_paths[i];
            }
        }
    }
    for (int i = 0; candidate_paths[i] != NULL; i++) {
        if (g_file_test(candidate_paths[i], G_FILE_TEST_EXISTS)) {
            return candidate_paths[i];
        }
    }
    return "";
}

void model_detect_system_info(AppModel *model) {
    strcpy(model->os_pretty_name, "Linux System Universal");
    strcpy(model->os_id, "linux");

    if (g_file_test(APP_LOCAL_LOGO_PATH, G_FILE_TEST_EXISTS)) {
        strcpy(model->app_logo_path, APP_LOCAL_LOGO_PATH);
    } else if (g_file_test(APP_LOGO_PATH, G_FILE_TEST_EXISTS)) {
        strcpy(model->app_logo_path, APP_LOGO_PATH);
    } else {
        strcpy(model->app_logo_path, "");
    }

    struct utsname uts;
    if (uname(&uts) == 0) {
        strncpy(model->os_arch, uts.machine, sizeof(model->os_arch) - 1);
        strncpy(model->os_kernel, uts.release, sizeof(model->os_kernel) - 1);
    }

    FILE *fp = fopen("/etc/os-release", "r");
    if (!fp) fp = fopen("/usr/lib/os-release", "r");

    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
                sscanf(line + 12, "\"%127[^\"]\"", model->os_pretty_name);
            } else if (strncmp(line, "ID=", 3) == 0) {
                sscanf(line + 3, "\"%63[^\"]\"", model->os_id);
                if (model->os_id[0] == '\0') sscanf(line + 3, "%63s", model->os_id);
            }
        }
        fclose(fp);
    }

    const char *found_logo = find_distro_logo_path(model->os_id);
    strncpy(model->os_logo_path, found_logo, sizeof(model->os_logo_path) - 1);

    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        unsigned long total_ram = (si.totalram * si.mem_unit) / (1024 * 1024);
        unsigned long free_ram = (si.freeram * si.mem_unit) / (1024 * 1024);
        unsigned long used_ram = total_ram - free_ram;
        snprintf(model->ram_info, sizeof(model->ram_info), "%lu MB / %lu MB", used_ram, total_ram);
        model->ram_percentage = (total_ram > 0) ? ((double)used_ram / (double)total_ram) : 0.0;

        unsigned long total_swap = (si.totalswap * si.mem_unit) / (1024 * 1024);
        unsigned long free_swap = (si.freeswap * si.mem_unit) / (1024 * 1024);
        unsigned long used_swap = total_swap - free_swap;
        snprintf(model->swap_info, sizeof(model->swap_info), "%lu MB / %lu MB", used_swap, total_swap);
        model->swap_percentage = (total_swap > 0) ? ((double)used_swap / (double)total_swap) : 0.0;
    }

    FILE *fcpu = fopen("/proc/cpuinfo", "r");
    strcpy(model->cpu_model, "Generic Processor");
    if (fcpu) {
        char line[256];
        while (fgets(line, sizeof(line), fcpu)) {
            if (strncmp(line, "model name", 10) == 0) {
                char *colon = strchr(line, ':');
                if (colon) {
                    strncpy(model->cpu_model, colon + 2, sizeof(model->cpu_model) - 1);
                    model->cpu_model[strcspn(model->cpu_model, "\r\n")] = 0;
                    break;
                }
            }
        }
        fclose(fcpu);
    }

    FILE *fdisk = popen("df -h / 2>/dev/null | tail -n 1", "r");
    if (fdisk) {
        char line[256], size[32] = {0}, used[32] = {0}, avail[32] = {0};
        if (fgets(line, sizeof(line), fdisk)) {
            sscanf(line, "%*s %31s %31s %31s", size, used, avail);
            snprintf(model->disk_info, sizeof(model->disk_info), "%s / %s (Bebas: %s)", used, size, avail);
        } else {
            strcpy(model->disk_info, "Linux Root System");
        }
        pclose(fdisk);
    }
}

void model_save_hold_session(AppModel *model) {
    char *path = model_get_user_config_file(APP_HOLD_STATE_FILENAME);
    if (!path) return;

    FILE *fp = fopen(path, "w");
    if (fp) {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, model->hold_sessions);

        while (g_hash_table_iter_next(&iter, &key, &value)) {
            if (GPOINTER_TO_INT(value) == 1) {
                fprintf(fp, "%s\n", (char *)key);
            }
        }

        fclose(fp);
        chmod(path, 0600);
    }

    g_free(path);
}

void model_load_hold_session(AppModel *model) {
    char *path = model_get_user_config_file(APP_HOLD_STATE_FILENAME);
    if (!path) return;

    FILE *fp = fopen(path, "r");
    if (fp) {
        char pkg[128];

        while (fgets(pkg, sizeof(pkg), fp)) {
            pkg[strcspn(pkg, "\r\n")] = 0;

            if (strlen(pkg) > 0) {
                g_hash_table_insert(
                    model->hold_sessions,
                    g_strdup(pkg),
                    GINT_TO_POINTER(1)
                );
            }
        }

        fclose(fp);
    }

    g_free(path);
}

AppModel* model_init(void) {
    AppModel *model = g_new0(AppModel, 1);
    init_user_cache_dir(model);

    if (system("which dnf > /dev/null 2>&1") == 0) {
        model->type = PKG_MANAGER_DNF;
    } else if (system("which apt-get > /dev/null 2>&1") == 0) {
        model->type = PKG_MANAGER_APT;
    } else if (system("which pacman > /dev/null 2>&1") == 0) {
        model->type = PKG_MANAGER_PACMAN;
    } else {
        model->type = PKG_MANAGER_LFS;
    }
    
    model->po_translations = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    model->hold_sessions = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    model_detect_system_info(model);
    model_load_sudo_session(model);
    model_load_hold_session(model);

    char *env_lang = getenv("LANG");
    if (env_lang && strncmp(env_lang, "en", 2) == 0) {
        model_load_po_file(model, "en");
    } else {
        model_load_po_file(model, "id");
    }
    return model;
}

bool model_verify_sudo_password(const char *password) {
    if (!password || strlen(password) == 0) return false;
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "echo '%s' | sudo -S true >/dev/null 2>&1", password);
    return (system(cmd) == 0);
}

void model_save_sudo_password(AppModel *model, const char *password) {
    if (!model || !password) return;

    /*
     * Keep the password in process memory only.
     * Never persist an administrator password to disk.
     * sudo itself manages its authentication timestamp.
     */
    strncpy(model->sudo_password, password, sizeof(model->sudo_password) - 1);
    model->sudo_password[sizeof(model->sudo_password) - 1] = '\0';
    model->has_saved_password = true;
}

bool model_load_sudo_session(AppModel *model) {
    if (!model) return false;

    /*
     * SysUpdater deliberately does not restore a sudo password from disk.
     * A new process starts without a saved password.
     */
    model->sudo_password[0] = '\0';
    model->has_saved_password = false;
    return false;
}

static void get_installed_version_for_pkg(const char *pkg_name, char *out_ver, size_t max_len) {
    strncpy(out_ver, "-", max_len);
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "dpkg-query -W -f='${Version}' %s 2>/dev/null || rpm -q --qf '%%{VERSION}' %s 2>/dev/null", pkg_name, pkg_name);
    FILE *fp = popen(cmd, "r");
    if (fp) {
        if (fgets(out_ver, max_len, fp)) {
            out_ver[strcspn(out_ver, "\r\n")] = 0;
        }
        pclose(fp);
    }
}

GList* model_get_upgradable_packages(AppModel *model) {
    GList *list = NULL;
    FILE *fp = NULL;

    if (model->type == PKG_MANAGER_APT) {
        fp = popen("apt list --upgradable 2>/dev/null | grep -v 'Listing...'", "r");
    } else if (model->type == PKG_MANAGER_DNF) {
        fp = popen("dnf check-update 2>/dev/null | grep -E '^[a-zA-Z0-9_-]+'", "r");
    } else if (model->type == PKG_MANAGER_PACMAN) {
        fp = popen("pacman -Qu 2>/dev/null", "r");
    }

    int count = 0;
    if (fp) {
        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            char pkg[128] = {0}, new_v[64] = {0}, old_v[64] = {0};
            if (sscanf(line, "%127[^/]/%*s %63s", pkg, new_v) >= 2 || sscanf(line, "%127s %63s", pkg, new_v) >= 1) {
                PkgInfo *info = g_new0(PkgInfo, 1);
                strncpy(info->name, pkg, sizeof(info->name) - 1);
                strncpy(info->new_version, new_v[0] ? new_v : "Latest", sizeof(info->new_version) - 1);
                get_installed_version_for_pkg(info->name, old_v, sizeof(old_v));
                strncpy(info->old_version, old_v[0] ? old_v : "-", sizeof(info->old_version) - 1);
                info->selected = true;
                list = g_list_append(list, info);
                count++;
            }
        }
        pclose(fp);
    }
    model->total_updates_count = count;
    return list;
}

GList* model_get_installed_packages(AppModel *model) {
    GList *list = NULL;
    FILE *fp = NULL;

    if (model->type == PKG_MANAGER_APT) {
        fp = popen("dpkg-query -W -f='${Package}\t${Version}\n' 2>/dev/null | head -n 120", "r");
    } else if (model->type == PKG_MANAGER_DNF) {
        fp = popen("rpm -qa --qf '%{NAME}\t%{VERSION}\n' 2>/dev/null | head -n 120", "r");
    } else if (model->type == PKG_MANAGER_PACMAN) {
        fp = popen("pacman -Q 2>/dev/null | head -n 120", "r");
    }

    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            char pkg[128] = {0}, ver[64] = {0};
            if (sscanf(line, "%127s\t%63s", pkg, ver) >= 1 || sscanf(line, "%127s %63s", pkg, ver) >= 1) {
                PkgInfo *info = g_new0(PkgInfo, 1);
                strncpy(info->name, pkg, sizeof(info->name) - 1);
                strncpy(info->old_version, ver[0] ? ver : "Active", sizeof(info->old_version) - 1);
                
                if (g_hash_table_lookup(model->hold_sessions, info->name)) {
                    info->is_hold = true;
                }
                
                list = g_list_append(list, info);
            }
        }
        pclose(fp);
    }
    return list;
}

GList* model_get_upgrade_history(AppModel *model) {
    GList *list = NULL;
    FILE *fp = (model->type == PKG_MANAGER_APT) ? popen("grep -E 'install |upgrade ' /var/log/dpkg.log 2>/dev/null | tail -n 50", "r") : popen("dnf history 2>/dev/null | head -n 30", "r");

    if (fp) {
        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            char date[32] = {0}, time[32] = {0}, act[32] = {0}, pkg[128] = {0};
            if (sscanf(line, "%31s %31s %31s %127s", date, time, act, pkg) >= 3) {
                PkgInfo *info = g_new0(PkgInfo, 1);
                strncpy(info->name, pkg[0] ? pkg : act, sizeof(info->name) - 1);
                snprintf(info->old_version, sizeof(info->old_version), "%s %s", date, time);
                list = g_list_append(list, info);
            }
        }
        pclose(fp);
    }
    return list;
}

GList* model_get_live_categorized_repo(AppModel *model) {
    GList *list = NULL;
    FILE *fp = NULL;

    if (model->type == PKG_MANAGER_APT) {
        /* Membaca Paket, Section Resmi, dan Deskripsi dalam 1 perintah tunggal tanpa loop slow-query */
        fp = popen("apt-cache show $(apt-cache search . | head -n 250 | cut -d' ' -f1) 2>/dev/null | grep -E '^(Package|Section|Description-en|Description):'", "r");
    } else if (model->type == PKG_MANAGER_DNF) {
        fp = popen("dnf list available 2>/dev/null | head -n 250", "r");
    }

    if (fp) {
        char line[512];
        PkgInfo *current_pkg = NULL;

        while (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\r\n")] = 0;

            if (strncmp(line, "Package: ", 9) == 0) {
                if (current_pkg && strlen(current_pkg->name) > 0) {
                    list = g_list_append(list, current_pkg);
                }
                current_pkg = g_new0(PkgInfo, 1);
                strncpy(current_pkg->name, line + 9, sizeof(current_pkg->name) - 1);
                strcpy(current_pkg->category, "📦 Main / System"); // Default jika section kosong
                current_pkg->selected = false;
            } 
            else if (strncmp(line, "Section: ", 9) == 0 && current_pkg) {
                char section_raw[64] = {0};
                strncpy(section_raw, line + 9, sizeof(section_raw) - 1);

                /* Kapitalisasi huruf pertama nama section repositori resmi */
                if (strlen(section_raw) > 0) {
                    section_raw[0] = g_ascii_toupper(section_raw[0]);
                    snprintf(current_pkg->category, sizeof(current_pkg->category), "📁 %s", section_raw);
                }
            } 
            else if ((strncmp(line, "Description: ", 13) == 0 || strncmp(line, "Description-en: ", 16) == 0) && current_pkg) {
                char *desc_start = strchr(line, ':');
                if (desc_start && strlen(current_pkg->description) == 0) {
                    strncpy(current_pkg->description, desc_start + 2, sizeof(current_pkg->description) - 1);
                }
            }
        }

        if (current_pkg && strlen(current_pkg->name) > 0) {
            list = g_list_append(list, current_pkg);
        }

        pclose(fp);
    }
    return list;
}

GList* model_get_ppa_list(AppModel *model) {
    GList *list = NULL;
    FILE *fp = popen("grep -h '^deb ' /etc/apt/sources.list /etc/apt/sources.list.d/*.list 2>/dev/null", "r");
    if (fp) {
        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            char repo_url[256] = {0};
            if (sscanf(line, "deb %255s", repo_url) >= 1) {
                PPAInfo *info = g_new0(PPAInfo, 1);

                if (strstr(repo_url, "ppa.launchpad")) {
                    char ppa_name[256] = {0};
                    sscanf(strstr(repo_url, "ppa.launchpad.net/"), "ppa.launchpad.net/%255s", ppa_name);
                    snprintf(info->ppa_name, sizeof(info->ppa_name), "ppa:%s", ppa_name);
                    strcpy(info->file_source, "PPA Pihak Ketiga");
                } else {
                    strncpy(info->ppa_name, repo_url, sizeof(info->ppa_name) - 1);
                    strcpy(info->file_source, "Repositori Bawaan Sistem");
                }
                
                strcpy(info->status, "🟢 Aktif");
                list = g_list_append(list, info);
            }
        }
        pclose(fp);
    }
    return list;
}

bool model_set_package_hold_status(AppModel *model, const char *pkg_name, bool hold) {
    if (!pkg_name || strlen(pkg_name) == 0 || !model->has_saved_password) return false;
    char cmd[512];
    if (model->type == PKG_MANAGER_APT) {
        snprintf(cmd, sizeof(cmd), "echo '%s' | sudo -S apt-mark %s %s >/dev/null 2>&1", model->sudo_password, hold ? "hold" : "unhold", pkg_name);
    } else {
        snprintf(cmd, sizeof(cmd), "echo '%s' | sudo -S dnf config-manager --%s-exclude=%s >/dev/null 2>&1", model->sudo_password, hold ? "add" : "remove", pkg_name);
    }

    if (system(cmd) == 0) {
        if (hold) g_hash_table_insert(model->hold_sessions, g_strdup(pkg_name), GINT_TO_POINTER(1));
        else g_hash_table_remove(model->hold_sessions, pkg_name);
        model_save_hold_session(model);
        return true;
    }
    return false;
}

bool model_create_system_snapshot(AppModel *model, const char *comment) {
    if (!model->has_saved_password) return false;
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "echo '%s' | sudo -S timeshift --create --comments '%s' --tags D >/dev/null 2>&1 || true",
        model->sudo_password, comment ? comment : "SysUpdater Snapshot");
    return (system(cmd) == 0);
}

bool model_clean_system_junk(AppModel *model) {
    if (!model->has_saved_password) return false;
    char cmd[512];
    if (model->type == PKG_MANAGER_APT) {
        snprintf(cmd, sizeof(cmd), "echo '%s' | sudo -S apt-get autoremove -y && echo '%s' | sudo -S apt-get clean >/dev/null 2>&1",
            model->sudo_password, model->sudo_password);
    } else {
        snprintf(cmd, sizeof(cmd), "echo '%s' | sudo -S dnf autoremove -y && echo '%s' | sudo -S dnf clean all >/dev/null 2>&1",
            model->sudo_password, model->sudo_password);
    }
    return (system(cmd) == 0);
}

bool model_add_ppa_repository(AppModel *model, const char *ppa_name) {
    if (!ppa_name || strlen(ppa_name) == 0 || !model->has_saved_password) return false;
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "echo '%s' | sudo -S add-apt-repository -y %s >/dev/null 2>&1", model->sudo_password, ppa_name);
    return (system(cmd) == 0);
}

bool model_remove_ppa_repository(AppModel *model, const char *ppa_name) {
    if (!ppa_name || strlen(ppa_name) == 0 || !model->has_saved_password) return false;
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "echo '%s' | sudo -S add-apt-repository --remove -y %s >/dev/null 2>&1", model->sudo_password, ppa_name);
    return (system(cmd) == 0);
}

bool model_setup_auto_schedule(AppModel *model, int interval_hours) {
    char cron_cmd[512];
    snprintf(cron_cmd, sizeof(cron_cmd), "(crontab -l 2>/dev/null; echo \"0 */%d * * * sysupdate --check >/dev/null 2>&1\") | crontab -", interval_hours);
    return (system(cron_cmd) == 0);
}



char* model_read_file_content(const char *filename) {
    char path[512];
    char *content = NULL;

    if (!filename || strlen(filename) == 0) {
        return g_strdup("File tidak ditemukan.");
    }

    /* 1. Coba baca dari folder instalasi resmi /opt/riskafirdaus/sysupdater/doc/ */
    snprintf(path, sizeof(path), "/usr/share/doc/sysupdater/%s", filename);
    if (g_file_get_contents(path, &content, NULL, NULL)) {
        return content;
    }

    /* 2. Coba baca dari relative path lokal (untuk mode dev/pengembangan) */
    snprintf(path, sizeof(path), "doc/%s", filename);
    if (g_file_get_contents(path, &content, NULL, NULL)) {
        return content;
    }

    /* 3. Coba baca langsung dari root project lokal */
    if (g_file_get_contents(filename, &content, NULL, NULL)) {
        return content;
    }

    /* 4. Khusus 'umum.md': Fallback ke README.md atau dokumentasi jika file umum.md tidak ada */
    if (g_str_equal(filename, "umum.md")) {
        if (g_file_get_contents("/opt/riskafirdaus/sysupdater/doc/README.md", &content, NULL, NULL) ||
            g_file_get_contents("/opt/riskafirdaus/sysupdater/doc/dokumentasi", &content, NULL, NULL)) {
            return content;
        }
    }

    return g_strdup("Gagal membaca berkas dokumentasi.");
}

GList* model_get_available_languages(void) {
    GList *langs = NULL;
    const char *target_dir = g_file_test(APP_LOCAL_PO_DIR, G_FILE_TEST_IS_DIR) ? APP_LOCAL_PO_DIR : (g_file_test("po", G_FILE_TEST_IS_DIR) ? "po" : APP_PO_DIR);

    DIR *d = opendir(target_dir);
    if (d) {
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL) {
            char *ext = strrchr(dir->d_name, '.');
            if (ext && strcmp(ext, ".po") == 0) {
                char lang_code[32] = {0};
                strncpy(lang_code, dir->d_name, ext - dir->d_name);
                langs = g_list_append(langs, g_strdup(lang_code));
            }
        }
        closedir(d);
    }

    if (!langs) {
        langs = g_list_append(langs, g_strdup("id"));
        langs = g_list_append(langs, g_strdup("en"));
    }
    return langs;
}

static void extract_po_string(const char *src, char *dest, size_t dest_size) {
    const char *start = strchr(src, '"');
    if (!start) {
        dest[0] = '\0';
        return;
    }
    start++;
    const char *end = strrchr(start, '"');
    if (!end || end <= start) {
        dest[0] = '\0';
        return;
    }
    size_t len = end - start;
    if (len >= dest_size) len = dest_size - 1;
    strncpy(dest, start, len);
    dest[len] = '\0';
}

void model_load_po_file(AppModel *model, const char *lang_code) {
    if (!model || !lang_code) return;
    g_hash_table_remove_all(model->po_translations);
    strncpy(model->active_po_lang, lang_code, sizeof(model->active_po_lang) - 1);

    char po_path[512] = {0};
    if (g_file_test(APP_LOCAL_PO_DIR, G_FILE_TEST_IS_DIR)) {
        snprintf(po_path, sizeof(po_path), "%s/%s.po", APP_LOCAL_PO_DIR, lang_code);
    } else if (g_file_test("po", G_FILE_TEST_IS_DIR)) {
        snprintf(po_path, sizeof(po_path), "po/%s.po", lang_code);
    } else {
        snprintf(po_path, sizeof(po_path), "%s/%s.po", APP_PO_DIR, lang_code);
    }

    FILE *fp = fopen(po_path, "r");
    if (!fp) return;

    char line[1024];
    char cur_msgid[512] = {0};
    char cur_msgstr[512] = {0};
    int parsing_mode = 0;

    while (fgets(line, sizeof(line), fp)) {
        g_strstrip(line);
        if (line[0] == '#' || line[0] == '\0') continue;

        if (strncmp(line, "msgid ", 6) == 0) {
            if (strlen(cur_msgid) > 0 && strlen(cur_msgstr) > 0) {
                g_hash_table_insert(model->po_translations, g_strdup(cur_msgid), g_strdup(cur_msgstr));
            }
            cur_msgid[0] = '\0';
            cur_msgstr[0] = '\0';
            extract_po_string(line + 6, cur_msgid, sizeof(cur_msgid));
            parsing_mode = 1;
        } else if (strncmp(line, "msgstr ", 7) == 0) {
            extract_po_string(line + 7, cur_msgstr, sizeof(cur_msgstr));
            parsing_mode = 2;
        } else if (line[0] == '"') {
            char temp_str[512] = {0};
            extract_po_string(line, temp_str, sizeof(temp_str));
            if (parsing_mode == 1) {
                strncat(cur_msgid, temp_str, sizeof(cur_msgid) - strlen(cur_msgid) - 1);
            } else if (parsing_mode == 2) {
                strncat(cur_msgstr, temp_str, sizeof(cur_msgstr) - strlen(cur_msgstr) - 1);
            }
        }
    }

    if (strlen(cur_msgid) > 0 && strlen(cur_msgstr) > 0) {
        g_hash_table_insert(model->po_translations, g_strdup(cur_msgid), g_strdup(cur_msgstr));
    }

    fclose(fp);
}

const char* model_translate_text(AppModel *model, const char *msgid) {
    if (!model || !msgid) return msgid;
    const char *val = g_hash_table_lookup(model->po_translations, msgid);
    return (val && strlen(val) > 0) ? val : msgid;
}
