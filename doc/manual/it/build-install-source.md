# Compilazione e Installazione dal Codice Sorgente

**Language:** [English](../en/build-install-source.md) | [Japanese](../ja/build-install-source.md) | [Français](../fr/build-install-source.md) | [中文](../zh/build-install-source.md) | [Deutsch](../de/build-install-source.md) | **Italiano**

**In generale, si prega di seguire questa procedura.**

## Installazione dei Pacchetti Ubuntu Prerequisiti

### Per Compilazione dal Codice Sorgente
```bash
# Strumenti di build essenziali
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# Prerequisiti DPDK
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

### Pacchetti Opzionali
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

## Compilazione di sdplane-oss dal Codice Sorgente

```bash
# Clonare il repository
git clone https://github.com/kait-cronos/sdplane-oss
cd sdplane-oss

# Generare file di build
sh autogen.sh

# Configurare e compilare
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make

# Installare nel sistema
# Questo installerà in $prefix (predefinito: /usr/local/sbin)
sudo make install
```