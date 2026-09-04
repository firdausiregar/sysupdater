#include "main_view.h"
#include "../controllers/app_controller.h"
#include "components/sidebar.h"
#include "pages/pages.h"

static void apply_pro_css(void) {
GtkCssProvider *provider = gtk_css_provider_new();
const char *css =
"window { background-color: #0f172a; color: #f8fafc; font-family: 'Cantarell', 'Segoe UI', sans-serif; }"
".header-banner { background-image: linear-gradient(135deg, #1e293b, #0f172a); border-radius: 12px; border: 1px solid #334155; padding: 16px; margin-bottom: 12px; }"
".header-title { font-size: 20px; font-weight: bold; color: #60a5fa; }"
".os-badge { background-color: #1e3a8a; color: #93c5fd; font-weight: bold; padding: 4px 10px; border-radius: 12px; font-size: 11px; border: 1px solid #3b82f6; }"
".card-frame { background-color: #1e293b; border: 1px solid #334155; border-radius: 12px; padding: 18px; }"
"treeview { background-color: #0f172a; color: #f8fafc; border: 1px solid #334155; border-radius: 8px; font-size: 13px; }"
"treeview:nth-child(even) { background-color: #1e293b; }"
"treeview:selected { background-color: #0284c7; color: #ffffff; }"
"treeview:hover { background-color: #1e3a8a; }"
"treeview header button { background-color: #1e293b; color: #38bdf8; font-weight: bold; border: none; padding: 8px; }"
"button.suggested-action { background-image: linear-gradient(to bottom, #10b981, #059669); color: #ffffff; font-weight: bold; border: 1px solid #34d399; border-radius: 8px; padding: 8px 16px; }"
"entry { background-color: #0f172a; color: #34d399; border-radius: 8px; padding: 8px; border: 1px solid #334155; }";


gtk_css_provider_load_from_data(provider, css, -1, NULL);
gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
    GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);



}

static gboolean on_splash_step(gpointer user_data) {
AppContext *ctx = (AppContext*)user_data;
static double frac = 0.0;
frac += 0.2;


if (frac <= 1.0) {
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ctx->splash_progress), frac);
    return TRUE;
} else {
    gtk_widget_destroy(ctx->splash_window);
    gtk_widget_show_all(ctx->main_window);
    return FALSE;
}



}

static gboolean on_main_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
gtk_widget_hide(widget);
return TRUE;
}

void show_app_with_splash(void) {
g_set_prgname(APP_ID);
g_set_application_name(APP_NAME);


apply_pro_css();
AppContext *ctx = controller_init();

ctx->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
gtk_window_set_title(GTK_WINDOW(ctx->main_window), APP_NAME);
gtk_window_set_default_size(GTK_WINDOW(ctx->main_window), 1020, 680);
gtk_window_set_position(GTK_WINDOW(ctx->main_window), GTK_WIN_POS_CENTER);
gtk_window_set_resizable(GTK_WINDOW(ctx->main_window), FALSE);

if (g_file_test(ctx->model->app_logo_path, G_FILE_TEST_EXISTS)) {
    gtk_window_set_icon_from_file(GTK_WINDOW(ctx->main_window), ctx->model->app_logo_path, NULL);
}

controller_setup_tray_icon(ctx);

GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

GtkWidget *sidebar = create_sidebar_component(ctx);
gtk_box_pack_start(GTK_BOX(hbox), sidebar, FALSE, FALSE, 0);

GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
gtk_box_pack_start(GTK_BOX(hbox), sep, FALSE, FALSE, 0);

ctx->stack = gtk_stack_new();
gtk_stack_set_transition_type(GTK_STACK(ctx->stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);

gtk_stack_add_named(GTK_STACK(ctx->stack), create_page_dashboard(ctx), "page_dashboard");
gtk_stack_add_named(GTK_STACK(ctx->stack), create_page_upgrade(ctx), "page_upgrade");
gtk_stack_add_named(GTK_STACK(ctx->stack), create_page_store(ctx), "page_store");
gtk_stack_add_named(GTK_STACK(ctx->stack), create_page_blacklist(ctx), "page_blacklist");
gtk_stack_add_named(GTK_STACK(ctx->stack), create_page_installed(ctx), "page_installed");
gtk_stack_add_named(GTK_STACK(ctx->stack), create_page_history(ctx), "page_history");
gtk_stack_add_named(GTK_STACK(ctx->stack), create_page_cleaner(ctx), "page_cleaner");
gtk_stack_add_named(GTK_STACK(ctx->stack), create_page_ppa(ctx), "page_ppa");
gtk_stack_add_named(GTK_STACK(ctx->stack), create_page_schedule(ctx), "page_schedule");

gtk_box_pack_start(GTK_BOX(hbox), ctx->stack, TRUE, TRUE, 0);
gtk_container_add(GTK_CONTAINER(ctx->main_window), hbox);
g_signal_connect(ctx->main_window, "delete-event", G_CALLBACK(on_main_window_delete_event), ctx);

ctx->splash_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
gtk_window_set_decorated(GTK_WINDOW(ctx->splash_window), FALSE);
gtk_window_set_default_size(GTK_WINDOW(ctx->splash_window), 420, 260);
gtk_window_set_position(GTK_WINDOW(ctx->splash_window), GTK_WIN_POS_CENTER);

GtkWidget *s_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
gtk_container_set_border_width(GTK_CONTAINER(s_box), 20);

GtkWidget *splash_logo = NULL;
if (g_file_test(ctx->model->app_logo_path, G_FILE_TEST_EXISTS)) {
    GdkPixbuf *pix = gdk_pixbuf_new_from_file_at_scale(ctx->model->app_logo_path, 80, 80, TRUE, NULL);
    if (pix) {
        splash_logo = gtk_image_new_from_pixbuf(pix);
        g_object_unref(pix);
    }
}
if (!splash_logo) splash_logo = gtk_image_new_from_icon_name("system-software-update", GTK_ICON_SIZE_DIALOG);

char title_markup[256];
snprintf(title_markup, sizeof(title_markup), "<span font='18' font_weight='bold' foreground='#38bdf8'>%s</span>", APP_NAME);
GtkWidget *lbl_s_title = gtk_label_new(NULL);
gtk_label_set_markup(GTK_LABEL(lbl_s_title), title_markup);

ctx->splash_label = gtk_label_new("Memuat sistem Linux universal...");
ctx->splash_progress = gtk_progress_bar_new();

gtk_box_pack_start(GTK_BOX(s_box), splash_logo, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(s_box), lbl_s_title, FALSE, FALSE, 5);
gtk_box_pack_start(GTK_BOX(s_box), ctx->splash_label, FALSE, FALSE, 5);
gtk_box_pack_start(GTK_BOX(s_box), ctx->splash_progress, FALSE, FALSE, 5);

gtk_container_add(GTK_CONTAINER(ctx->splash_window), s_box);
gtk_widget_show_all(ctx->splash_window);

g_timeout_add(300, on_splash_step, ctx);



}
