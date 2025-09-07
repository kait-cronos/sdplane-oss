# Installation der Abhängigkeiten

**Language:** [English](../en/install-dependencies.md) | [日本語](../ja/install-dependencies.md) | [Français](../fr/install-dependencies.md) | [中文](../zh/install-dependencies.md) | **Deutsch** | [Italiano](../it/install-dependencies.md) | [한국어](../ko/install-dependencies.md) | [ไทย](../th/install-dependencies.md) | [Español](../es/install-dependencies.md)

## Abhängigkeiten

sdplane-oss benötigt die folgenden Komponenten:
- **lthread** (yasuhironet/lthread): Leichtgewichtiges kooperatives Threading
- **liburcu-qsbr**: Userspace RCU library  
- **libpcap**: Packet capture library
- **DPDK 23.11.1**: Data Plane Development Kit

## Installation der sdplane-Abhängigkeiten Debian-Pakete

```bash
sudo apt update && sudo apt install liburcu-dev libpcap-dev
```

## Installation der Build-Tools und DPDK-Voraussetzungen

```bash
sudo apt install build-essential cmake autotools-dev autoconf automake \
                 libtool pkg-config python3 python3-pip meson ninja-build \
                 python3-pyelftools libnuma-dev pkgconf
```

## Installation von lthread

```bash
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
cd ..
```

## Installation von DPDK 23.11.1

```bash
# DPDK 23.11.1 herunterladen
wget https://fast.dpdk.org/rel/dpdk-23.11.1.tar.xz
tar xf dpdk-23.11.1.tar.xz
cd dpdk-23.11.1

# DPDK kompilieren und installieren
meson setup -Dprefix=/usr/local build
cd build
ninja install
cd ../..

# Installation überprüfen
pkg-config --modversion libdpdk
# Sollte ausgeben: 23.11.1
```