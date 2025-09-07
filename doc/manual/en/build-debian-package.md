# Build Debian Package and Install

**Language:** **English** | [Japanese](../ja/build-debian-package.md) | [Français](../fr/build-debian-package.md) | [中文](../zh/build-debian-package.md) | [Deutsch](../de/build-debian-package.md) | [Italiano](../it/build-debian-package.md) | [한국어](../ko/build-debian-package.md) | [ไทย](../th/build-debian-package.md) | [Español](../es/build-debian-package.md)

## Install prerequisite package
```bash
sudo apt install build-essential cmake devscripts debhelper
```

## Build sdplane-oss Debian Package
```bash
# First make sure to start in a clean space.
(cd build && make distclean)
make distclean

# Build Debian package from source
bash build-debian.sh

# Install the generated package (will be produced in parent dir)
sudo apt install ../sdplane_*.deb
```