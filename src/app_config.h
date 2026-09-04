#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#define APP_NAME            "System Updater Pro"
#define APP_ID              "sysupdater"
#define APP_VERSION         "1.0.0"
#define GITHUB_REPO "firdausiregar/sysupdater"
#define APP_CLI_BINARY      "sysupdate"
#define APP_GUI_BINARY      "sysupdater"

/* Standard FHS Compliant Paths */
#define APP_LOGO_PATH       "/usr/share/sysupdater/assets/logoku.png"
#define APP_LOCAL_LOGO_PATH "assets/logoku.png"
#define APP_PO_DIR          "/usr/share/sysupdater/po"
#define APP_LOCAL_PO_DIR    "po"
#define APP_HOLD_STATE_FILENAME "hold-session"

/* Dynamic user-level cache directory fallback */
#define APP_CACHE_SUBDIR    "sysupdater"
#endif
