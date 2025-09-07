# Construcción e Instalación del Paquete Debian

**Language:** [English](../en/build-debian-package.md) | [Japanese](../ja/build-debian-package.md) | [Français](../fr/build-debian-package.md) | [中文](../zh/build-debian-package.md) | [Deutsch](../de/build-debian-package.md) | [Italiano](../it/build-debian-package.md) | [한국어](../ko/build-debian-package.md) | [ไทย](../th/build-debian-package.md) | **Español**

## Instalación de paquetes prerrequisitos
```bash
sudo apt install build-essential cmake devscripts debhelper
```

## Construcción del Paquete Debian de sdplane-oss
```bash
# Primero asegurarse de comenzar en un espacio limpio
(cd build && make distclean)
make distclean

# Construir paquete Debian desde código fuente
bash build-debian.sh

# Instalar el paquete generado (se producirá en el directorio padre)
sudo apt install ../sdplane_*.deb
```