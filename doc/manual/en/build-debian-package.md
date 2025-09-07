# Build Debian Package and Install

**Language:** **English** | [Japanese](../ja/build-debian-package.md) | [French](../fr/build-debian-package.md) | [Chinese](../zh/build-debian-package.md) | [German](../de/build-debian-package.md) | [Italian](../it/build-debian-package.md) | [Korean](../ko/build-debian-package.md) | [Thai](../th/build-debian-package.md) | [Spanish](../es/build-debian-package.md)

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