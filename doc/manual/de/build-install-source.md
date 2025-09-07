# Kompilierung und Installation aus dem Quellcode

**Language:** [English](../en/build-install-source.md) | [Japanese](../ja/build-install-source.md) | [Français](../fr/build-install-source.md) | [中文](../zh/build-install-source.md) | **Deutsch**

**Bitte folgen Sie im Allgemeinen diesem Verfahren.**

## Installation der erforderlichen Ubuntu-Pakete

### Für Kompilierung aus dem Quellcode
```bash
# Wichtige Build-Tools
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# DPDK-Voraussetzungen
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

### Optionale Pakete
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

## Kompilierung von sdplane-oss aus dem Quellcode

```bash
# Repository klonen
git clone https://github.com/kait-cronos/sdplane-oss
cd sdplane-oss

# Build-Dateien generieren
sh autogen.sh

# Konfigurieren und kompilieren
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make

# Im System installieren
# Dies wird in $prefix installiert (Standard: /usr/local/sbin)
sudo make install
```