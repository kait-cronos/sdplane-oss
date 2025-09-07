# Erstellung und Installation des Debian-Pakets

**Language:** [English](../en/build-debian-package.md) | [Japanese](../ja/build-debian-package.md) | [Français](../fr/build-debian-package.md) | [中文](../zh/build-debian-package.md) | **Deutsch**

## Installation der erforderlichen Pakete
```bash
sudo apt install build-essential cmake devscripts debhelper
```

## Kompilierung des sdplane-oss Debian-Pakets
```bash
# Zunächst sicherstellen, dass wir in einem sauberen Arbeitsbereich beginnen
(cd build && make distclean)
make distclean

# Debian-Paket aus dem Quellcode kompilieren
bash build-debian.sh

# Das generierte Paket installieren (wird im übergeordneten Verzeichnis erstellt)
sudo apt install ../sdplane_*.deb
```