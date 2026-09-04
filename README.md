# System Updater Pro - Open Source Linux Package Manager

Universal GTK3/C99 package management tool for Linux distributions (Debian, Ubuntu, Arch, Fedora).

## Features
- Complete PPA & Source Repository Management
- Package Hold / Blacklist Session Persistence
- Automatic System Cleanup & Timeshift Snapshot integration
- Full Gettext PO Localization (Indonesian & English)
- Interactive CLI + System Tray Background Monitor

## Building from Source

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./sysupdater
```


## Debian Packaging

```bash
dpkg-buildpackage -us -uc
```

