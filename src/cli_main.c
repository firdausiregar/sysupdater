#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app_config.h"
#include "models/package_model.h"

void show_cli_menu(AppModel *model) {
printf("\033[1;36m====================================================\033[0m\n");
printf("\033[1;32m  %s - %s\033[0m\n", APP_NAME, model_translate_text(model, "CLI_TITLE"));
printf("\033[1;36m====================================================\033[0m\n");
printf(" 1. %s\n", model_translate_text(model, "CLI_OPT1"));
printf(" 2. %s\n", model_translate_text(model, "CLI_OPT2"));
printf(" 3. %s\n", model_translate_text(model, "CLI_OPT3"));
printf(" 4. %s\n", model_translate_text(model, "CLI_OPT4"));
printf(" 0. %s\n", model_translate_text(model, "CLI_OPT0"));
printf("----------------------------------------------------\n");
printf("%s", model_translate_text(model, "CLI_CHOICE"));
}

int main(int argc, char *argv[]) {
	if (chdir("/opt/riskafirdaus/sysupdater") != 0) {
        (void)chdir(".");
    }
AppModel *model = model_init();


if (argc > 1) {
    if (strcmp(argv[1], "--check") == 0) {
        printf("\033[1;33m[*] Memindai Pembaruan Paket Live...\033[0m\n");
        system("apt list --upgradable 2>/dev/null || dnf check-update 2>/dev/null || pacman -Qu 2>/dev/null");
        return 0;
    } else if (strcmp(argv[1], "--upgrade") == 0) {
        printf("\033[1;32m[*] Memulai Pembaruan Paket Sistem...\033[0m\n");
        system("sudo apt-get update && sudo apt-get upgrade -y || sudo dnf upgrade -y || sudo pacman -Syu --noconfirm");
        return 0;
    } else if (strcmp(argv[1], "--uninstall") == 0) {
        printf("\033[1;31m[*] Menghapus Aplikasi System Updater Pro...\033[0m\n");
        system("sudo apt remove -y sysupdater || sudo dnf remove -y sysupdater || sudo pacman -R --noconfirm sysupdater");
        printf("✔ Aplikasi berhasil dihapus dari sistem.\n");
        return 0;
    }
}

int choice = -1;
while (choice != 0) {
    show_cli_menu(model);
    if (scanf("%d", &choice) != 1) break;
    switch (choice) {
        case 1:
            printf("\n\033[1;33m[*] Memeriksa paket siap update...\033[0m\n");
            system("apt list --upgradable 2>/dev/null || dnf check-update 2>/dev/null");
            break;
        case 2:
            printf("\n\033[1;32m[*] Menjalankan upgrade sistem...\033[0m\n");
            system("sudo apt-get update && sudo apt-get upgrade -y || sudo dnf upgrade -y");
            break;
        case 3:
            printf("\n\033[1;34m[*] Ringkasan Spesifikasi Sistem:\033[0m\n");
            system("uname -a && free -h && df -h /");
            break;
        case 4:
            printf("\n\033[1;31m[*] Menghapus Aplikasi...\033[0m\n");
            system("sudo apt remove -y sysupdater 2>/dev/null || sudo dnf remove -y sysupdater || sudo pacman -R --noconfirm sysupdater");
            printf("✔ Aplikasi telah dihapus.\n");
            return 0;
        case 0:
            printf("Keluar dari CLI.\n");
            break;
        default:
            printf("Pilihan tidak valid.\n");
    }
    printf("\nTekan Enter untuk melanjutkan...");
    getchar(); getchar();
}

return 0;



}
