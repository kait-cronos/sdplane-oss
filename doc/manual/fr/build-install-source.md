# Compilation et Installation depuis les Sources

**Language:** [English](../en/build-install-source.md) | [日本語](../ja/build-install-source.md) | **Français** | [中文](../zh/build-install-source.md) | [Deutsch](../de/build-install-source.md) | [Italiano](../it/build-install-source.md) | [한국어](../ko/build-install-source.md) | [ไทย](../th/build-install-source.md) | [Español](../es/build-install-source.md)

**En général, veuillez suivre cette procédure.**

## Installation des Packages Ubuntu Prérequis

### Pour Compilation depuis Sources
```bash
# Outils de build essentiels
sudo apt install build-essential cmake autotools-dev autoconf automake libtool pkg-config

# Prérequis DPDK
sudo apt install python3 python3-pip meson ninja-build python3-pyelftools libnuma-dev pkgconf
```

### Packages Optionnels
```bash
sudo apt install etckeeper tig bridge-utils \
                 iptables-persistent fail2ban dmidecode screen ripgrep
```

## Compilation de sdplane-oss depuis Sources

```bash
# Cloner le repository
git clone https://github.com/kait-cronos/sdplane-oss
cd sdplane-oss

# Générer les fichiers de build
sh autogen.sh

# Configurer et compiler
mkdir build
cd build
CFLAGS="-g -O0" sh ../configure
make

# Installer sur le système
# Ceci installera dans $prefix (défaut : /usr/local/sbin)
sudo make install
```