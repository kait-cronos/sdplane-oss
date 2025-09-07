# Installazione delle Dipendenze

**Language:** [English](../en/install-dependencies.md) | [Japanese](../ja/install-dependencies.md) | [Français](../fr/install-dependencies.md) | [中文](../zh/install-dependencies.md) | [Deutsch](../de/install-dependencies.md) | **Italiano** | [한국어](../ko/install-dependencies.md) | [ไทย](../th/install-dependencies.md) | [Español](../es/install-dependencies.md)

## Dipendenze

sdplane-oss richiede i seguenti componenti:
- **lthread** (yasuhironet/lthread): Threading cooperativo leggero
- **liburcu-qsbr**: Libreria RCU userspace  
- **libpcap**: Libreria cattura pacchetti
- **DPDK 23.11.1**: Data Plane Development Kit

## Installazione dei pacchetti debian dipendenze sdplane

```bash
sudo apt update && sudo apt install liburcu-dev libpcap-dev
```

## Installazione degli Strumenti di Build e Prerequisiti DPDK

```bash
sudo apt install build-essential cmake autotools-dev autoconf automake \
                 libtool pkg-config python3 python3-pip meson ninja-build \
                 python3-pyelftools libnuma-dev pkgconf
```

## Installazione di lthread

```bash
git clone https://github.com/yasuhironet/lthread
cd lthread
cmake .
make
sudo make install
cd ..
```

## Installazione di DPDK 23.11.1

```bash
# Scaricare DPDK 23.11.1
wget https://fast.dpdk.org/rel/dpdk-23.11.1.tar.xz
tar xf dpdk-23.11.1.tar.xz
cd dpdk-23.11.1

# Compilare e installare DPDK
meson setup -Dprefix=/usr/local build
cd build
ninja install
cd ../..

# Verificare l'installazione
pkg-config --modversion libdpdk
# Dovrebbe mostrare: 23.11.1
```