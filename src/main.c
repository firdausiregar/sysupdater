#include <gtk/gtk.h>
#include <string.h>
#include <unistd.h>
#include "update_checker.h"
#include "views/main_view.h"

int main(int argc, char *argv[]) {
    if (chdir("/opt/riskafirdaus/sysupdater") != 0) {
        (void)chdir(".");
    }

    gtk_init(&argc, &argv);
    
    
    gboolean start_minimized = FALSE;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--minimized") == 0 || strcmp(argv[i], "--tray") == 0) {
            start_minimized = TRUE;
            break;
        }
    }

    
    show_app_with_splash();

  
    gtk_main();
    return 0;
}
