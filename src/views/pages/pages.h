#ifndef PAGES_H
#define PAGES_H

#include <gtk/gtk.h>
#include "../../controllers/app_controller.h"

GtkWidget* create_page_dashboard(AppContext *ctx);
GtkWidget* create_page_upgrade(AppContext *ctx);
GtkWidget* create_page_store(AppContext *ctx);
GtkWidget* create_page_blacklist(AppContext *ctx);
GtkWidget* create_page_installed(AppContext *ctx);
GtkWidget* create_page_history(AppContext *ctx);
GtkWidget* create_page_cleaner(AppContext *ctx);
GtkWidget* create_page_ppa(AppContext *ctx);
GtkWidget* create_page_schedule(AppContext *ctx);

#endif
