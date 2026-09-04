#ifndef UPDATE_CHECKER_H
#define UPDATE_CHECKER_H

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app_config.h"


static char* fetch_latest_github_version() {
    char command[512];
    snprintf(command, sizeof(command), "curl -s --max-time 3 https://api.github.com/repos/%s/releases/latest | grep '\"tag_name\"' | head -n 1 | cut -d '\"' -f 4", GITHUB_REPO);

    FILE *fp = popen(command, "r");
    if (!fp) return NULL;

    static char latest_version[64];
    if (fgets(latest_version, sizeof(latest_version), fp) != NULL) {
        latest_version[strcspn(latest_version, "\r\n")] = 0;
        pclose(fp);
        if (strlen(latest_version) > 0) {
            return latest_version;
        }
    }
    pclose(fp);
    return NULL;
}

static int compare_versions(const char *v1, const char *v2) {
    if (v1[0] == 'v' || v1[0] == 'V') v1++;
    if (v2[0] == 'v' || v2[0] == 'V') v2++;
    return strcmp(v1, v2);
}

static void on_check_update_clicked(GtkButton *button, gpointer user_data) {
    GtkWindow *parent_window = GTK_WINDOW(user_data);
    GtkWidget *dialog;
    
    char *latest = fetch_latest_github_version();
    
    if (latest == NULL) {
        dialog = gtk_message_dialog_new(parent_window,
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Gagal terhubung ke server!\nPeriksa koneksi internet Anda.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    if (compare_versions(latest, APP_VERSION) > 0) {
        char message[512];
        snprintf(message, sizeof(message), 
                 "🎉 Versi baru tersedia: %s\n\nVersi Anda saat ini: %s\n\nApakah Anda ingin mengunduh pembaruan ini sekarang?", 
                 latest, APP_VERSION);
                 
        GtkWidget *update_dialog = gtk_message_dialog_new(parent_window,
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_QUESTION,
            GTK_BUTTONS_YES_NO,
            "%s", message);
            
        gtk_window_set_title(GTK_WINDOW(update_dialog), "Pembaruan Tersedia!");
        
        int response = gtk_dialog_run(GTK_DIALOG(update_dialog));
        if (response == GTK_RESPONSE_YES) {
            char url[512];
            snprintf(url, sizeof(url), "xdg-open https://github.com/%s/releases/latest", GITHUB_REPO);
            system(url);
        }
        gtk_widget_destroy(update_dialog);
    } else {
        dialog = gtk_message_dialog_new(parent_window,
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "Aplikasi Anda sudah menggunakan versi paling mutakhir!\n\nVersi Saat Ini: %s", APP_VERSION);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

static void show_about_dialog(GtkWindow *parent) {
    GtkWidget *about_dialog = gtk_about_dialog_new();
    
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog), APP_NAME);
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dialog), APP_VERSION);
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog), 
        "Aplikasi manajer pembaruan sistem dan katalog utilitas Linux berbasis GTK3.");
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_dialog), "https://github.com/riskafirdaus/sysupdater");
    gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about_dialog), (const char*[]){ "Riska Firdaus", NULL });
    
    gtk_window_set_transient_for(GTK_WINDOW(about_dialog), parent);

   
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(about_dialog));
    GtkWidget *update_btn = gtk_button_new_with_label("🔄 Cek Pembaruan Otomatis");
    gtk_widget_set_margin_top(update_btn, 12);
    gtk_widget_set_margin_bottom(update_btn, 12);
    gtk_widget_set_margin_start(update_btn, 40);
    gtk_widget_set_margin_end(update_btn, 40);
    
    g_signal_connect(update_btn, "clicked", G_CALLBACK(on_check_update_clicked), about_dialog);
    
    gtk_box_pack_start(GTK_BOX(content_area), update_btn, FALSE, FALSE, 0);
    gtk_widget_show_all(update_btn);
    // ------------------------------------------------

    gtk_dialog_run(GTK_DIALOG(about_dialog));
    gtk_widget_destroy(about_dialog);
}

#endif // 