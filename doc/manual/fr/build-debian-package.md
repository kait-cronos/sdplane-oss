# Compilation et Installation du Package Debian

**Language:** [English](../en/build-debian-package.md) | [Japanese](../ja/build-debian-package.md) | **Français**

## Installation des packages prérequis
```bash
sudo apt install build-essential cmake devscripts debhelper
```

## Compilation du Package Debian sdplane-oss
```bash
# D'abord s'assurer de commencer dans un espace propre
(cd build && make distclean)
make distclean

# Compiler le package Debian depuis les sources
bash build-debian.sh

# Installer le package généré (sera produit dans le répertoire parent)
sudo apt install ../sdplane_*.deb
```