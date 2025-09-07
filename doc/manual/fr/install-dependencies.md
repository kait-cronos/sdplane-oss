# Installation des Dépendances

**Language:** [English](../en/install-dependencies.md) | [Japanese](../ja/install-dependencies.md) | **Français** | [中文](../zh/install-dependencies.md) | [Deutsch](../de/install-dependencies.md) | [Italiano](../it/install-dependencies.md) | [한국어](../ko/install-dependencies.md) | [ไทย](../th/install-dependencies.md) | [Español](../es/install-dependencies.md)

## Dépendances

sdplane-oss nécessite les composants suivants :
- **lthread** (yasuhironet/lthread) : Threading coopératif léger
- **liburcu-qsbr** : Userspace RCU library  
- **libpcap** : Packet capture library
- **DPDK 23.11.1** : Data Plane Development Kit

## Installation des paquets debian de dépendances sdplane

```bash
sudo apt update && sudo apt install liburcu-dev libpcap-dev
```

## Installation des outils de build et paquets prérequis DPDK

```bash
sudo apt install build-essential cmake autotools-dev autoconf automake \
                 libtool pkg-config python3 python3-pip meson ninja-build \
                 python3-pyelftools libnuma-dev pkgconf
```

## Installation de lthread

```bash
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
cd ..
```

## Installation de DPDK 23.11.1

```bash
# Télécharger DPDK 23.11.1
wget https://fast.dpdk.org/rel/dpdk-23.11.1.tar.xz
tar xf dpdk-23.11.1.tar.xz
cd dpdk-23.11.1

# Compiler et installer DPDK
meson setup -Dprefix=/usr/local build
cd build
ninja install
cd ../..

# Vérifier l'installation
pkg-config --modversion libdpdk
# Doit afficher : 23.11.1
```