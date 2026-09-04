cmake_minimum_required(VERSION 3.10)

project(sysupdater LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)
pkg_check_modules(LIBNOTIFY REQUIRED IMPORTED_TARGET libnotify)

add_executable(sysupdater
    src/main.c
    src/models/package_model.c
    src/controllers/app_controller.c
    src/views/main_view.c
    src/views/components/sidebar.c
    src/views/components/header.c
    src/views/components/dialogs.c
    src/views/pages/page_dashboard.c
    src/views/pages/page_upgrade.c
    src/views/pages/page_store.c
    src/views/pages/page_blacklist.c
    src/views/pages/page_installed.c
    src/views/pages/page_history.c
    src/views/pages/page_cleaner.c
    src/views/pages/page_ppa.c
    src/views/pages/page_schedule.c
)

target_include_directories(sysupdater PRIVATE
    src
    src/models
    src/controllers
    src/views
    src/views/components
    src/views/pages
)

target_link_libraries(sysupdater PRIVATE
    PkgConfig::GTK3
    PkgConfig::LIBNOTIFY
)

add_executable(sysupdate
    src/cli_main.c
    src/models/package_model.c
)

target_include_directories(sysupdate PRIVATE
    src
    src/models
    src/controllers
    src/views
    src/views/components
    src/views/pages
)

target_link_libraries(sysupdate PRIVATE PkgConfig::GTK3)

install(
    TARGETS sysupdater sysupdate
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    DIRECTORY assets/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/sysupdater/assets
)

install(
    DIRECTORY po/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/sysupdater/po
    FILES_MATCHING PATTERN "*.po"
)

install(
    FILES
        sysupdater.desktop
        sysupdater-cli.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(
    FILES assets/logoku.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps
    RENAME sysupdater.png
)
